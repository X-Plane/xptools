/*
 * Copyright (c) 2026, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "ProcessUtils.h"

#include <string>
#include <vector>
#include <string.h>

#if IBM
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <map>
#include <mutex>

// Per-FILE* shim state for the CreateProcess-based xpt_popen on Windows. The
// original implementation in XGrinderShell.cpp kept these in file-scope
// globals; we promote them to a map so multiple xpt_popen calls can be live
// concurrently.
namespace {

struct WinPopenState {
    HANDLE stdout_read  = nullptr;
    HANDLE stdout_write = nullptr;
    HANDLE stdin_read   = nullptr;
    HANDLE stdin_write  = nullptr;
    HANDLE stderr_read  = nullptr;
    HANDLE stderr_write = nullptr;
    HANDLE process      = nullptr;
};

std::mutex                          g_popen_mtx;
std::map<FILE*, WinPopenState>      g_popen_state;

}

FILE* xpt_popen(const char* command, const char* mode)
{
    if (!command || strcmp(mode, "r") != 0)
        return nullptr;

    WinPopenState s;

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    if (!CreatePipe(&s.stdout_read, &s.stdout_write, &sa, 0))
        return nullptr;
    SetHandleInformation(s.stdout_read, HANDLE_FLAG_INHERIT, 0);
    if (!CreatePipe(&s.stdin_read, &s.stdin_write, &sa, 0))
    {
        CloseHandle(s.stdout_read);
        CloseHandle(s.stdout_write);
        return nullptr;
    }
    SetHandleInformation(s.stdin_write, HANDLE_FLAG_INHERIT, 0);
    if (!CreatePipe(&s.stderr_read, &s.stderr_write, &sa, 0))
    {
        CloseHandle(s.stdout_read); CloseHandle(s.stdout_write);
        CloseHandle(s.stdin_read);  CloseHandle(s.stdin_write);
        return nullptr;
    }
    SetHandleInformation(s.stderr_read, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si = {};
    si.cb = sizeof(STARTUPINFOA);
    si.hStdOutput = s.stdout_write;
    si.hStdInput  = s.stdin_read;
    si.hStdError  = s.stderr_write;
    si.dwFlags    = STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi = {};

    std::string cmdline(command);
    if (!CreateProcessA(nullptr, cmdline.data(), nullptr, nullptr, TRUE,
                        DETACHED_PROCESS, nullptr, nullptr, &si, &pi))
    {
        CloseHandle(s.stdout_read); CloseHandle(s.stdout_write);
        CloseHandle(s.stdin_read);  CloseHandle(s.stdin_write);
        CloseHandle(s.stderr_read); CloseHandle(s.stderr_write);
        return nullptr;
    }
    CloseHandle(pi.hThread);
    s.process = pi.hProcess;

    // Close child-side ends now that they belong to the child.
    CloseHandle(s.stdout_write); s.stdout_write = nullptr;
    CloseHandle(s.stdin_read);   s.stdin_read   = nullptr;
    CloseHandle(s.stderr_write); s.stderr_write = nullptr;

    int fd = _open_osfhandle(reinterpret_cast<intptr_t>(s.stdout_read), _O_RDONLY);
    if (fd < 0)
    {
        CloseHandle(s.stdout_read);
        CloseHandle(s.stdin_write);
        CloseHandle(s.stderr_read);
        CloseHandle(s.process);
        return nullptr;
    }
    FILE* fp = _fdopen(fd, "r");
    if (!fp)
    {
        _close(fd);
        CloseHandle(s.stdin_write);
        CloseHandle(s.stderr_read);
        CloseHandle(s.process);
        return nullptr;
    }

    // After _fdopen takes the fd, the underlying HANDLE is owned by the
    // CRT and closed when fclose() runs.
    s.stdout_read = nullptr;

    {
        std::lock_guard<std::mutex> lk(g_popen_mtx);
        g_popen_state[fp] = s;
    }
    return fp;
}

int xpt_pclose(FILE* stream)
{
    if (!stream) return -1;

    WinPopenState s;
    {
        std::lock_guard<std::mutex> lk(g_popen_mtx);
        auto it = g_popen_state.find(stream);
        if (it == g_popen_state.end())
        {
            fclose(stream);
            return -1;
        }
        s = it->second;
        g_popen_state.erase(it);
    }

    fclose(stream); // closes stdout_read (owned by the CRT)
    if (s.stdin_write)  CloseHandle(s.stdin_write);
    if (s.stderr_read)  CloseHandle(s.stderr_read);

    DWORD exit_code = 0;
    if (s.process)
    {
        WaitForSingleObject(s.process, INFINITE);
        GetExitCodeProcess(s.process, &exit_code);
        CloseHandle(s.process);
    }
    return static_cast<int>(exit_code);
}

#else // POSIX

FILE* xpt_popen(const char* command, const char* mode)
{
    return ::popen(command, mode);
}

int xpt_pclose(FILE* stream)
{
    return ::pclose(stream);
}

#endif

// ---------------------------------------------------------------------------
// run_subprocess: portable convenience wrapper around xpt_popen/xpt_pclose.

namespace {

// Quote one argv element for the platform shell. We always quote (even when
// not strictly needed) so the result is uniform and easy to read in logs.
std::string quote_arg(const std::string& a)
{
#if IBM
    // Windows: wrap in double quotes; escape embedded backslashes followed by
    // quotes per CommandLineToArgv rules, and escape internal quotes with \".
    std::string out = "\"";
    for (size_t i = 0; i < a.size(); ++i)
    {
        size_t bs = 0;
        while (i < a.size() && a[i] == '\\') { ++bs; ++i; }
        if (i == a.size())
        {
            out.append(bs * 2, '\\');
            break;
        }
        if (a[i] == '"')
        {
            out.append(bs * 2 + 1, '\\');
            out.push_back('"');
        }
        else
        {
            out.append(bs, '\\');
            out.push_back(a[i]);
        }
    }
    out.push_back('"');
    return out;
#else
    // POSIX: single-quote, escape any embedded single quotes.
    std::string out = "'";
    for (char c : a)
    {
        if (c == '\'')
            out += "'\\''";
        else
            out.push_back(c);
    }
    out.push_back('\'');
    return out;
#endif
}

} // namespace

int run_subprocess(const std::string& binary,
                   const std::vector<std::string>& argv,
                   std::function<void(const std::string&)> log_sink)
{
    std::string cmd = quote_arg(binary);
    for (const auto& a : argv)
    {
        cmd.push_back(' ');
        cmd += quote_arg(a);
    }
    // Merge stderr into stdout so the single popen pipe sees both.
#if IBM
    // Windows xpt_popen already routes child stderr to a separate pipe that
    // we never drain; merge it into stdout instead by going through cmd.exe.
    std::string cmdline = "cmd /S /C \"" + cmd + " 2>&1\"";
#else
    std::string cmdline = cmd + " 2>&1";
#endif

    FILE* p = xpt_popen(cmdline.c_str(), "r");
    if (!p) return -1;

    std::string line;
    char buf[1024];
    while (fgets(buf, sizeof(buf), p))
    {
        line += buf;
        size_t nl;
        while ((nl = line.find('\n')) != std::string::npos)
        {
            std::string one = line.substr(0, nl);
            if (!one.empty() && one.back() == '\r') one.pop_back();
            if (log_sink) log_sink(one);
            line.erase(0, nl + 1);
        }
    }
    if (!line.empty() && log_sink)
    {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        log_sink(line);
    }
    return xpt_pclose(p);
}
