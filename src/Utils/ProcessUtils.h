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
#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <stdio.h>
#include <string>
#include <vector>
#include <functional>

// Drop-in popen/pclose. On POSIX this is a thin wrapper around libc's
// popen/pclose. On Windows it is a CreateProcess-backed shim that returns a
// FILE* reading the child's stdout (with stderr redirected to a pipe that is
// drained on close so the child does not block on stderr writes).
//
// `mode` must be "r". The returned FILE* is owned by the caller and must be
// closed with xpt_pclose(); xpt_pclose returns the child's exit code (POSIX:
// libc pclose return; Windows: GetExitCodeProcess).
//
// Multiple concurrent xpt_popen handles are supported on both platforms.
FILE* xpt_popen(const char* command, const char* mode);
int   xpt_pclose(FILE* stream);

// Convenience wrapper: spawn `binary` with `argv` (NOT including argv[0]),
// quoting each argument for the platform shell, capture combined stdout/stderr,
// and invoke `log_sink` once per output line. Returns the child's exit code,
// or -1 on spawn failure.
int run_subprocess(const std::string& binary,
                   const std::vector<std::string>& argv,
                   std::function<void(const std::string&)> log_sink);

#endif
