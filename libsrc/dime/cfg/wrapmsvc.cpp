/*
 * A wrapper script for the Microsoft Visual C++ build tools (cl.exe,
 * lib.exe and link.exe) to make them grok UNIX-style arguments.
 *
 * This is written rather quick & dirty and should be overhauled with
 * clean-ups and robustifications. See particularly the "FIXME"
 * comments peppered throughout the sourcecode.
 *
 * Written by Morten Eriksen <mortene@sim.no>.
 *
 * This code is covered by the GNU GPL.
 *
 * Compiles with the Cygwin port of GCC's C++ compiler, you can simply
 * invoke it like this: ``g++ -s -o wrapmsvc.exe wrapmsvc.cpp''.
 */
/*
 * TODO:
 *
 * - Try to set up the cl.exe MSVC compiler even though it's not in
 *   the PATH. (Get hold of the registry setting for %ProgramDir%,
 *   then try to find it in the default MSVC installer location.)
 *   Should then also set up the INCLUDE and LIB variables to include
 *   the "system" directories.
 *
 * - Avoid generating dependency information for system-headers.
 *
 * - Kill the misc minor FIXME issues throughout the code.
 *
 * - Avoid using the standard C++ library if this means we get a much
 *   smaller executable.
 *
 * - Look into including wrapping code for the Borland compiler aswell
 *   (and other MSWindows compilers too?).
 *
 * - Clean up code by modularizing various mess into functions (and
 *   new sourcecode files).
 */


// FIXME: this use of libstdc++ probably adds a lot to the final
// exe. Could extract and use our SbString, SbList and SbDict classes
// from libCoin instead. 20010627 mortene.
#include <string>
#include <vector>
#include <set>

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <errno.h>
#include <sys/cygwin.h>
#include <sys/stat.h>

#define BUFSIZE 4096 

// Global flag, whether or not to output debug information for the
// wrapper itself.
static bool wrapdbg = false;


static void
errmsg_win32(BOOL fatal, const char * msg)
{
  va_list args;
  DWORD lasterr;
  LPTSTR buffer;
  BOOL result;

  lasterr = GetLastError();

  (void)fprintf(stderr, "Win32 API %s, %s",
                fatal ? "FATAL ERROR" : "error", msg);

  result =
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  lasterr,
                  0,
                  (LPTSTR)&buffer,
                  0,
                  NULL);

  if (result) {
    (void)fprintf(stderr, ":\n\t%s", buffer);
    LocalFree(buffer);
  }

  if (fatal) exit(1);
}

#define ERRMSG_FWD(errmsg_fatal) \
  va_list args; \
  char msg[BUFSIZE]; \
  va_start(args, text); \
  vsprintf(msg, text, args); \
  assert(strlen(msg) < BUFSIZE/2); /* just in case */ \
  errmsg_win32(errmsg_fatal, msg); \
  va_end(args)

/*!
  Outputs on stderr the given text message and the last error as
  reported by the Win32 API call GetLastError(). The input arguments
  should typically indicate the call which failed. Example usage:

  if (!OpenPrinter(name, &handle, defaults)) {
    errorw32("OpenPrinter('%s', %p, %p)", name, &handle, defaults);
  }
  else {
    ...

  This will give an error message like this:

  Win32 API error, OpenPrinter('Brother HJ-100', 0x55555555, 0x7f7f7f7f):
    The printer name is invalid.
*/
void errorw32(const char * text, ...) { ERRMSG_FWD(0); }
void fatalw32(const char * text, ...) { ERRMSG_FWD(1); }

#undef ERRMSG_FWD


/*
  Run an external process.

  @param cmd full command line of process to run.
  @param procstdout string which will get all output from child process
         on stdout attached to it.
  @param procstderr string which will get all output from child process
         on stderr attached to it.
  @return exit status of process.
 */
static DWORD
run_process(const char * cmd, std::string & procstdout, std::string & procstderr)
{
  if (wrapdbg) {
    (void)fprintf(stdout, "Invoking external process: ``%s''\n", cmd);
  }

  SECURITY_ATTRIBUTES securityattribs = {
    sizeof(SECURITY_ATTRIBUTES),
    NULL, // default permissions
    TRUE // child processes inherits handles
  }; 

  // Create pipes for the child process.
  HANDLE childstdinread, childstdinwrite;
  HANDLE childstdoutread, childstdoutwrite;
  HANDLE childstderrread, childstderrwrite;
  if (!CreatePipe(&childstdinread, &childstdinwrite, &securityattribs, 0)) { fatalw32("CreatePipe()");  }
  if (!CreatePipe(&childstdoutread, &childstdoutwrite, &securityattribs, 0)) { fatalw32("CreatePipe()"); }
  if (!CreatePipe(&childstderrread, &childstderrwrite, &securityattribs, 0)) { fatalw32("CreatePipe()");  }
 
  // Now create the child process. 
  PROCESS_INFORMATION childproc; 
  ZeroMemory( &childproc, sizeof(PROCESS_INFORMATION) );

  STARTUPINFO startupdata; 
  ZeroMemory( &startupdata, sizeof(STARTUPINFO) );
  startupdata.cb = sizeof(STARTUPINFO); 
  startupdata.dwFlags = STARTF_USESTDHANDLES;
  startupdata.hStdInput = childstdinread;
  startupdata.hStdOutput = childstdoutwrite;
  startupdata.hStdError = childstderrwrite;
 
  // Create the child process. 
  HLOCAL cmddup = (char *)LocalAlloc(0, strlen(cmd) + 1);
  if (!cmddup) { fatalw32("LocalAlloc()"); }
  (void)strcpy((char *)cmddup, cmd);
  BOOL result = CreateProcess(NULL, 
                              (LPTSTR)cmddup,// command line 
                              NULL,          // process security attributes 
                              NULL,          // primary thread security attributes 
                              TRUE,          // handles are inherited 
                              0,             // creation flags 
                              NULL,          // use parent's environment 
                              NULL,          // use parent's current directory 
                              &startupdata,  // STARTUPINFO pointer 
                              &childproc);   // receives PROCESS_INFORMATION 
  LocalFree(cmddup);
  if (!result) { fatalw32("CreateProcess(\"%s\", ...", cmd); }

  if (!CloseHandle(childstdinwrite)) { fatalw32("CloseHandle([handle])"); }
  if (!CloseHandle(childstdoutwrite)) { fatalw32("CloseHandle([handle])"); }
  if (!CloseHandle(childstderrwrite)) { fatalw32("CloseHandle([handle])"); }

  // Read output from the child process.
  DWORD dwRead; 
  CHAR chBuf[BUFSIZE]; 
  while (ReadFile(childstdoutread, chBuf, BUFSIZE-1, &dwRead, NULL) && (dwRead != 0)) {
    chBuf[dwRead] = '\0';
    // FIXME: I have a nagging feeling that this is _slow_, especially
    // when trying to cope with the large amount of output when
    // running sourcecode files with many dependencies on header files
    // through ``cl.exe /E [...]''.  Fixing this to use a larger tmp
    // buffer before concatening to the std::string instance would
    // probably have a noticable effect when compiling stuff like the
    // draggers in Coin, for instance. 20010823 mortene.
    procstdout += chBuf;
  }
  while (ReadFile(childstderrread, chBuf, BUFSIZE-1, &dwRead, NULL) && (dwRead != 0)) {
    chBuf[dwRead] = '\0';
    procstderr += chBuf;
  }

  // Wait for child process to exit and get hold of it's exit code.
  DWORD exitcode;
  DWORD slept = 0;
  do {
    if (!GetExitCodeProcess(childproc.hProcess, &exitcode)) {
      fatalw32("GetExitCodeProcess()");
    }

    if (exitcode == STILL_ACTIVE) {
      const DWORD SLEEPINTERVAL = 50;
      Sleep(SLEEPINTERVAL); slept += SLEEPINTERVAL;
    }
  } while (exitcode == STILL_ACTIVE);

  return exitcode;
}
 
/*
  @param s string to chop off end-of-line character from.
  @return true if a character was chomped, otherwise false.
*/
static bool
chomp(std::string & s)
{
  int len = s.length();
  if (len == 0) { return false; }
  char lastchar = s.at(len - 1);
  int cut = 0;
  if (lastchar == '\n') {
    cut++;
    if ((len >= 2) && s.at(len - 2) == '\r') { cut++; }
  }
  else if (lastchar == '\r') { cut++; }

  if (cut > 0) { s.resize(len - cut); }
  return cut > 0;
}

static std::string
dirprefix(const std::string arg)
{
  int idx = arg.length() - 1;
  while (arg.at(idx)!='\\' && arg.at(idx)!='/' && idx > 0) { idx--; }
  return arg.substr(0, idx);
}

static std::string
winpath(const std::string & s)
{
  char buf[MAX_PATH];
  // FIXME: what does this function return? 20010627 mortene.
  (void)cygwin32_conv_to_win32_path(s.c_str(), buf);
  return buf;
}

static std::string
posixpath(const std::string & s)
{
  char buf[MAX_PATH];
  // FIXME: what does this function return? 20010627 mortene.
  (void)cygwin32_conv_to_posix_path(s.c_str(), buf);
  return buf;
}

static inline bool
prefixmatch(const std::string s, const std::string prefix)
{
  return
    s.length() >= prefix.length() &&
    s.substr(0, prefix.length()).compare(prefix)==0;
}

static inline bool
suffixmatch(const std::string s, const std::string suffix)
{
  return
    s.length() >= suffix.length() &&
    s.substr(s.length() - suffix.length()).compare(suffix)==0;
}

static bool
match(const std::string ss, const char * cs, bool ignorecase = false)
{
  if (!ignorecase) { return ss.compare(cs)==0; }

  int cslen = strlen(cs);
  if (ss.length() != cslen) { return false; }
  for (int i=0; i < cslen; i++) {
    if (tolower(ss.at(i)) != tolower(cs[i])) { return false; }
  }
  return true;
}

static std::string
quotespaces(const std::string u)
{
  int i;
  std::string s(u);
  for (i = 0; i < s.length(); i++) {
    if (s[i] == ' ') {
      s.insert(i, "\\");
      i++;
    }
  }
  return s;
}

/**** Argument containers ********************************************/

struct DepTrackingArgs {
  DepTrackingArgs::DepTrackingArgs()
    : generate(false), phonytargets(false), targetobj(""), filename("")
  { }
    
  bool generate, phonytargets;
  std::string targetobj, filename;
};

/** Generic data common for tools. ********/
struct Tool {
  typedef std::vector<std::string*> stringpvector;

  Tool::Tool()
    : objname(""), msvcppargs(""), pdbname(""), debugmode(false)
  {
    // Store the original $LIB, as we might need to set it up
    // repeatedly from scratch later on.
    int neededsize = GetEnvironmentVariable("LIB", NULL, 0);
    if (neededsize > 1) {
      char * tmpbuf = new char[neededsize + 1];
      int resultsize = GetEnvironmentVariable("LIB", tmpbuf, neededsize);
      assert(resultsize == neededsize-1);
      Tool::original_LIB = new std::string(tmpbuf);
      delete tmpbuf;
    }
  }

  virtual const char * getToolName(void) = 0;
  virtual std::string constructArguments(void) = 0;

  // Traverse list of given -L paths and try to find absolute path of
  // .lib file (the MSVC++ linker doesn't have any "-L"-like option).
  void addLibFile(const std::string & name)
  {
    std::string * fullname = NULL;

    // Could be that the .lib file is given with full path already.
    struct stat buf;
    // debug
//      (void)fprintf(stdout, "*** TRYING: %s\n", name.c_str());
    if (stat(name.c_str(), &buf)==0) { fullname = new std::string(name); }

    stringpvector::reverse_iterator i;
    for (i = this->linkpath.rbegin();
         i != this->linkpath.rend() && !fullname;
         i++) {
      std::string compositename = **i;
      char lastchar = compositename.at(compositename.length() - 1);
      if (lastchar!='/' && lastchar!='\\') { compositename += '\\'; }
      compositename += name;

      // debug
//        (void)fprintf(stdout, "*** TRYING: %s\n", compositename.c_str());

      struct stat buf;
      if (stat(compositename.c_str(), &buf)==0) { fullname = new std::string(compositename); }
      // debug
//        else (void)fprintf(stdout, "===> FAILED\n");
    }
    
    // Assume library is in a default location (as given by envvar
    // LIB) if it couldn't be found.
    std::string * add = fullname ? fullname : new std::string(name);
    // debug
//      (void)fprintf(stdout, "** ADD-TO-LIST '%s'\n", add->c_str());
    this->libfiles.push_back(add);
  }

  void addLinkPath(const std::string & name)
  {
    this->linkpath.push_back(new std::string(name));
  }

  void addFileObj(const std::string & name)
  {
    this->objfiles.push_back(new std::string(name));
  }

  std::string commonArgs(void)
  {
    std::string arg;
    stringpvector::iterator i;

    for (i = this->objfiles.begin(); i != this->objfiles.end(); i++) {
      arg += " " + (**i);
    }

    for (i = this->libfiles.begin(); i != this->libfiles.end(); i++) {
      arg += " " + (**i);
    }

    // Must also update LIB environment variable for the linker, in
    // case some of the .lib-files explicitly given have implicit
    // dependencies to other .lib-files in the same directory.
    std::string new_LIB(Tool::original_LIB ? Tool::original_LIB->c_str() : "");
    for (i = this->linkpath.begin(); i != this->linkpath.end(); i++) {
      if (new_LIB.length() > 0) { new_LIB += ";"; }
      new_LIB += **i;
    }
    if (wrapdbg) { (void)fprintf(stdout, "LIB=='%s'\n", new_LIB.c_str()); }
    BOOL r = SetEnvironmentVariable("LIB", new_LIB.c_str());
    assert(r && "SetEnvironmentVariable() failed -- investigate");

    // All passthrough options -- that is, the MSVC++ style arguments
    // given that starts with a "/".
    for (i = this->passthrough.begin(); i != this->passthrough.end(); i++) {
      arg += " " + (**i);
    }

    return arg;
  }

  bool debugmode;
  std::string objname, msvcppargs, pdbname;
  stringpvector linkpath, libfiles, objfiles, passthrough;
  static std::string * original_LIB;
};

std::string * Tool::original_LIB = NULL;

/** Compiler-specifics. *******************/
struct CompilerArgs : public Tool {
  CompilerArgs::CompilerArgs()
    : aspreproc(false), cpuspec(""), optimizelevel(-1), compileobj(false)
  { }

  CompilerArgs::~CompilerArgs()
  {
    // FIXME: dealloc string* blocks in file vectors. 20010627 mortene.
  }

  const char * getToolName(void) { return "cl.exe"; }

  std::string constructArguments(void)
  {
    std::string arg;

    if (this->aspreproc) { arg += " /E"; }
    if (this->compileobj) { arg += " /c"; }
    if (this->optimizelevel >= 0) { arg += " /Ogityb2 /Gs"; }
    if (this->cpuspec.length() > 0) { arg += " /" + this->cpuspec; }
    if (this->debugmode) { arg += " /Zi"; }

    if (this->objname.length() > 0) {
      arg += (this->compileobj ? " /Fo" : " /Fe") + this->objname;
    }

    if (this->pdbname.length() > 0) { arg += " /Fd" + this->pdbname; }

    stringpvector::iterator i;
    for (i = this->preprocdefs.begin(); i != this->preprocdefs.end(); i++) {
      arg += " /D" + **i;
    }

    for (i = this->incpaths.begin(); i != this->incpaths.end(); i++) {
      arg += " /I" + **i;
    }

    for (i = this->c_files.begin(); i != this->c_files.end(); i++) {
      arg += " /Tc" + winpath(**i);
    }
    for (i = this->cpp_files.begin(); i != this->cpp_files.end(); i++) {
      arg += " /Tp" + winpath(**i);
    }
    for (i = this->h_files.begin(); i != this->h_files.end(); i++) {
      arg += " " + winpath(**i);
    }

    arg += this->commonArgs();
    
    return arg;
  }

  void addFileC(const std::string & name)
  {
    this->c_files.push_back(new std::string(name));
  }
  void addFileCPlusPlus(const std::string & name)
  {
    this->cpp_files.push_back(new std::string(name));
  }
  void addHeaderFile(const std::string & name)
  {
    this->h_files.push_back(new std::string(name));
  }
  void addPreProcDef(const std::string & name)
  {
    this->preprocdefs.push_back(new std::string(name));
  }
  void addIncPath(const std::string & name)
  {
    this->incpaths.push_back(new std::string(name));
  }

  stringpvector c_files, cpp_files, h_files, preprocdefs, incpaths;
  bool aspreproc, compileobj;
  int optimizelevel;
  std::string cpuspec;
  struct DepTrackingArgs deptracking;
};

/** Dynamic linker specifics. *************/
struct LinkerArgs : public Tool {
  LinkerArgs::LinkerArgs()
    : linkdll(false), alternative_c_lib(""), releasemode(false), uselibexe(false)
  { }

  LinkerArgs::~LinkerArgs()
  {
    // FIXME: dealloc string* instances in stringpvectors. 20010627 mortene.
  }

  const char * getToolName(void) { return uselibexe ? "lib.exe" : "link.exe"; }

  std::string constructArguments(void) {
    std::string arg;
    if (linkdll) { arg += " /dll"; }

    if (this->objname.length() > 0) { arg += " /OUT:" + this->objname; }
    if (this->pdbname.length() > 0) { arg += " /PDB:" + this->pdbname; }
    if (this->debugmode) { arg += " /DEBUG"; }
    if (this->releasemode) { arg += " /RELEASE"; }

    if (!uselibexe) {
      // FIXME: should really not use /INCREMENTAL when making an
      // archive .lib file. So this is just a workaround for a bug in
      // the build setup for misc Coin modules (simage, at
      // least). 20010810 mortene.
      arg += std::string(" /INCREMENTAL:") + (this->incremental ? "YES" : "NO");
    }

    if (this->alternative_c_lib.length() > 0) { arg += " " + this->alternative_c_lib; }

    stringpvector::iterator i;

    for (i = this->lstfiles.begin(); i != this->lstfiles.end(); i++) {
      const char * lstname = (**i).c_str();
      std::string dir = dirprefix(**i);
      if (dir.length() > 0) { dir += "/"; }
      FILE * lst = fopen(lstname, "r");
      if (!lst) { (void)fprintf(stderr, "Couldn't open file '%s'!!\n", lstname); exit(1); }
      char buffer[MAX_PATH];
      while (fgets(buffer, MAX_PATH, lst)) {
        std::string lstobjname = buffer;
        while (chomp(lstobjname)) { }
        arg += " " + dir + lstobjname;
      }
      (void)fclose(lst);
    }

    arg += this->commonArgs();

    return arg;
  }

  void addLstFile(const std::string & name)
  {
    this->lstfiles.push_back(new std::string(name));
  }

  bool linkdll, incremental, releasemode, uselibexe;
  stringpvector lstfiles;
  std::string alternative_c_lib;
};

/**** main() *********************************************************/

int
main(int argc, char ** argv)
{
  if (argc == 1) {
    std::string procstdout, procstderr;
    DWORD retcode = run_process("cl.exe", procstdout, procstderr);
    (void)fprintf(stdout, "%s", procstdout.c_str());
    (void)fprintf(stderr, "%s", procstderr.c_str());
    return retcode;
  }

  struct CompilerArgs compiler;
  struct LinkerArgs linker;

  struct Tool * tool = &compiler;


  /*** start of command-line parsing ************************************/

  for (int i=1; i < argc; i++) {
    std::string arg = argv[i];
    bool optarg = arg.at(0)=='-' || arg.at(0)=='/';
    bool forcompiler = (tool == &compiler);

    if (match(arg, "--wrapdbg")) {
      wrapdbg = true;
    }
    else if (match(arg, "--wrapversion")) {
      (void)fprintf(stdout, "$Revision: 1.1 $\n");
      exit(0);
    }

    // C srcfile?
    else if (suffixmatch(arg, ".c")) {
      tool = &compiler;
      compiler.addFileC(arg);
    }
    // C++ srcfile?
    else if (suffixmatch(arg, ".cpp") || suffixmatch(arg, ".cxx") ||
             suffixmatch(arg, ".c++") || suffixmatch(arg, ".cc")) {
      tool = &compiler;
      compiler.addFileCPlusPlus(arg);
    }
    // header file? (can be specified explicitly when executed as preprocessor)
    else if (suffixmatch(arg, ".h")) {
      tool = &compiler;
      compiler.addHeaderFile(arg);
    }

#if 0
    // OBSOLETED: we're now using .lst-files listing the obj-files to
    // be linked in. 20010807 mortene.

    // libtool ".la" file?
    else if (suffixmatch(arg, ".la")) {
      tool = &linker;

      // FIXME: we just assume that all object files should be
      // extracted from the library, in case we're making a DLL
      // (passing the .lib-files to link.exe doesn't preserve the
      // exported symbols). 20001024 mortene.

      FILE * fp = fopen(winpath(arg).c_str(), "r");
      if (!fp) { (void)fprintf(stderr, "Couldn't open file '%s'!!\n", arg.c_str()); exit(1); }
      const int TMPBUFFERSIZE = 1024;
      char buffer[TMPBUFFERSIZE];
      const std::string laoption = "old_library=";
      std::string old_library = "";
      while (fgets(buffer, TMPBUFFERSIZE, fp)) {
        if (strncmp(laoption.c_str(), buffer, laoption.length()) == 0) {
          old_library = &buffer[laoption.length()];
          break;
        }
      }
      fclose(fp);

      if (old_library.length() == 0) { (void)fprintf(stderr, "Couldn't find \"old_library\" in %s file!!\n", arg.c_str()); exit(1); }
      int firstquote = old_library.find('\'');
      if (firstquote != std::string::npos) {
        int lastquote = old_library.find_last_of('\'');
        old_library = old_library.substr(firstquote + 1, lastquote - 1);
      }

      std::string libpath = dirprefix(winpath(arg));
      
      bool trailingslash = libpath.at(libpath.length() - 1) == '\\';
      // FIXME: ".libs" is just hardcoded. 20001024 mortene.
      std::string full_libname =
        libpath + (trailingslash ? "" : "\\") + ".libs\\" + old_library;

      // List objects in .lib archive.
      std::string cmd = "lib.exe /nologo /list " + full_libname;
      std::string procstdout, procstderr;
      DWORD exitcode = run_process(cmd.c_str(), procstdout, procstderr);
      if (exitcode != 0) {
        (void)fprintf(exitcode ? stderr : stdout, "%s", procstdout.c_str());
        (void)fprintf(stderr, "%s", procstderr.c_str());
        exit(1);
      }

      int nexteol;
      do { // FIXME: man, this is lousy code -- should use a proper tokenizer. 20010622 mortene.
        int nexteol_n = procstdout.find('\n');
        int nexteol_r = procstdout.find('\r');
        nexteol = nexteol_n <= nexteol_r ? nexteol_n : nexteol_r;
        if (nexteol != std::string::npos) {
          linker.addFileObj(libpath + (trailingslash ? "" : "\\") + procstdout.substr(0, nexteol));
          procstdout.erase(0, nexteol);
          while (procstdout.length() && (procstdout.at(0)=='\n' || procstdout.at(0)=='\r')) { procstdout = procstdout.substr(1); }
        }
      } while (procstdout.length() && (nexteol != std::string::npos));
    }
#endif // OBSOLETED  .la-file handling

    // Name the .lib output file (this must be executed before the
    // filename suffix detections).
    else if (optarg && prefixmatch(arg.substr(1), "OUT:")) {
      tool = &linker;
      linker.objname = arg.substr(5);
    }

    // Just pass along object files to the linker process.
    else if (suffixmatch(arg, ".obj") || suffixmatch(arg, ".o")) {
      compiler.addFileObj(winpath(arg));
      linker.addFileObj(winpath(arg));
    }
    else if (suffixmatch(arg, ".lib")) {
      compiler.addLibFile(winpath(arg));
      linker.addLibFile(winpath(arg));
    }

    // Handle our own invention of ".lst"-files, listing up the
    // obj-files which should be part of the final link.
    else if (suffixmatch(arg, ".lst")) {
      tool = &linker;
      linker.addLstFile(winpath(arg));
    }

    // These checks must be kept before the -L* library linkfile match
    // to avoid -LINK or -LIB being interpreted as "link with library
    // INK" (ditto for -LIB).
    else if (optarg && match(arg.substr(1), "lib", true)) {
      tool = &linker;
      linker.uselibexe = true;
    }
    else if (optarg && match(arg.substr(1), "link", true)) {
      tool = &linker;
    }

    // This need to be kept before the -D* match to avoid -DLL being
    // interpreted as "set preprocessor define LL".
    else if (optarg && match(arg.substr(1), "dll", true)) {
      tool = &linker;
      linker.linkdll = true;
    }

    // Additional library path.
    else if (optarg && arg.at(1)=='L') {
      // FIXME: should check that the string trailing the 'L' actually
      // points to a valid path? 20010709 mortene.
      compiler.addLinkPath(winpath(arg.substr(2)));
      linker.addLinkPath(winpath(arg.substr(2)));
    }

    // Additional library for the compiler.
    else if (optarg && arg.at(1)=='l') {
      compiler.addLibFile(winpath(arg.substr(2)) + ".lib");
      linker.addLibFile(winpath(arg.substr(2)) + ".lib");
    }

    // Run in preprocessor mode.
    else if (optarg && match(arg.substr(1), "E")) {
      tool = &compiler;
      compiler.aspreproc = true;
    }

    // Option for the linker process, simply to be forwarded.
    else if (optarg && match(arg.substr(1), "RELEASE")) {
      tool = &linker;
      linker.releasemode = true;
    }

    // /DEBUG option for the linker process -- this must obviously go
    // before the '/D<preproc>' check below.
    else if (optarg && match(arg.substr(1), "DEBUG")) {
      tool = &linker;
      linker.debugmode = true;
    }

    // A preprocessor define.
    else if (optarg && arg.at(1)=='D') {
      tool = &compiler;
      compiler.addPreProcDef(arg.substr(2));
    }

    // Include debuginfo in object files.
    else if (optarg && match(arg.substr(1), "g")) {
      compiler.debugmode = true;
      linker.debugmode = true;
    }

    // An option for the linker this -- whether or not to build
    // incremental libraries (typically during debugging, trading
    // space for fast linking). This need to go before the '-I<path>'
    // check right below, so we don't confuse this with an additional
    // include path setting.
    else if (optarg && prefixmatch(arg.substr(1), "INCREMENTAL:")) {
      tool = &linker;
      linker.incremental = suffixmatch(arg, "YES") ? true : false;
    }

    // Additional include path for the preprocessor.
    else if (optarg && arg.at(1)=='I') {
      tool = &compiler;
      compiler.addIncPath(winpath(arg.substr(2)));
      // FIXME: should check that the string trailing the 'I' actually
      // points to a valid path? 20010709 mortene.
    }

    // Compile for specific hardware.
    else if (optarg && forcompiler && arg.at(1)=='m') {
      std::string intelcpu = arg.substr(2);
      const char prefix[] = "cpu=";
      if (intelcpu.compare(prefix, strlen(prefix))==0) { intelcpu.erase(0, strlen(prefix)); }
      const char * cpuconv[] = {
        "i386", "G3",
        "i486", "G4",
        "pentium", "G5",
        "pentiumpro", "G6",
        NULL
      };
      int i=0;
      while (cpuconv[i]) {
        if (intelcpu.compare(cpuconv[i])==0) {
          compiler.cpuspec = cpuconv[i+1];
          break;
        }
        i += 2;
      }
      // Just ignores if unknown CPU type. Not sure if that's a good
      // strategy.
    }

    // Optimization (ie matching regexp "[-/]O[0-9]?").
    else if (optarg && forcompiler && arg.at(1)=='O' &&
             ((arg.length()==2) || (arg.length()==3 && isdigit(arg.at(2))))) {
      compiler.optimizelevel = arg.at(2) - 0x30;
    }

    // NOTE that we explicitly check for "-" as prefix for the GCC3
    // options below, as they clash with the cl.exe options "/M?" for
    // specifying alternative C-libraries (MT-safe, debug-version,
    // etc).

    // GCC3-style "-MD" option for sending dependency tracking Make
    // rules to a file. The output file will have the same name as
    // srcfile, only with .d suffix instead of language-specific
    // suffix -- unless the "-MF" option is used.
    else if (optarg && match(arg.substr(0), "-MD")) {
      tool = &compiler;
      compiler.deptracking.generate = true;
    }

    // GCC3-style "-MT targetname" option for naming the target in a
    // dependency tracking Make rule.
    //
    // (ie: -MT 'name'  ==>   "name: <srcfiledeps>")
    else if (optarg && match(arg.substr(0), "-MT")) {
      tool = &compiler;
      compiler.deptracking.targetobj = argv[++i];
    }

    // GCC3-style "-MF depfilename" option for naming the file to
    // write dependency tracking Make rules.
    else if (optarg && match(arg.substr(0), "-MF")) {
      tool = &compiler;
      compiler.deptracking.filename =  argv[++i];
    }

    // GCC3-style "-MP" option for including phony targets when
    // writing dependency tracking Make rules.
    else if (optarg && match(arg.substr(0), "-MP")) {
      tool = &compiler;
      compiler.deptracking.phonytargets = true;
    }

    // Alternative C-libraries.
    else if (optarg &&
             (match(arg.substr(0), "/MD") ||   // MSVCRT.LIB
              match(arg.substr(0), "/ML") ||   // LIBC.LIB
              match(arg.substr(0), "/MT") ||   // LIBCMT.LIB
              match(arg.substr(0), "/MDd") ||  // MSVCRTD.LIB
              match(arg.substr(0), "/MLd") ||  // LIBCD.LIB
              match(arg.substr(0), "/MTd"))) { // LIBCMTD.LIB
      linker.alternative_c_lib = arg;
    }

    // Compile to object file
    else if (optarg && match(arg.substr(1), "c")) {
      tool = &compiler;
      compiler.compileobj = true;
    }

    // Name the objectfile / executable.
    else if (optarg && match(arg.substr(1), "o")) {
      std::string objname = argv[++i];
      compiler.objname = objname;
      linker.objname = objname;
    }

    // Name the .pdb debug file.
    else if (optarg && prefixmatch(arg.substr(1), "Fd")) {
      tool = &compiler;
      tool->pdbname = arg.substr(3);
    }
    else if (optarg && prefixmatch(arg.substr(1), "PDB:")) {
      tool = &linker;
      tool->pdbname = arg.substr(5);
    }

    // Remaining MSVC-tool options are regarded as pass-through
    // information.
    else if (arg.at(0) == '/') { // "MSVC++ format"
      tool->passthrough.push_back(new std::string(arg));
    }

    // Remaining options kills us.
    else {
      // Using a warning here and then continuing won't work, as then
      // for instance the configure script will believe the compiler
      // can handle all options which are tested.
      (void)fprintf(stderr, "%s ERROR: unknown option \"%s\"\n",
                    argv[0], arg.c_str());
      exit(1);
    }
  }

  /*** end of command-line parsing **************************************/


  std::string cmd = compiler.getToolName();

  if (compiler.deptracking.generate) {
    cmd = tool->getToolName() + (" /nologo /E " + tool->constructArguments());

    std::string procstdout, procstderr;
    DWORD exitcode = run_process(cmd.c_str(), procstdout, procstderr);
    if (exitcode != 0) {
      // stdout should just get the preprocessor output, so we ignore
      // it and dump stderr only.
      (void)fprintf(stderr, "%s", procstderr.c_str());
      exit(exitcode);
    }

    typedef std::set<std::string> stringset_t;
    stringset_t includedeps;

    const char * p = procstdout.c_str();
    const char * end = procstdout.c_str() + procstdout.length();
    const char * HASHLINE = "#line";
    const int HASHLINELEN = strlen(HASHLINE);
    while (p < (end - HASHLINELEN)) {
      // scan past whitespace
      while ((p < end) && ((*p == ' ') || (*p == '\t'))) { p++; }

      if ((*p == '#') && (strncmp(p, HASHLINE, HASHLINELEN)==0)) {
        p += HASHLINELEN;
        while ((p < end) && (*p != '"')) { p++; }
        p++;
        const char * e = p + 1;
        while ((e < end) && (*e != '"')) { e++; }
        assert(e > p);
        char buf[MAX_PATH];
        int n = e - p;
        assert(n < MAX_PATH-1);
        (void)strncpy(buf, p, n);
        buf[n] = '\0';

        includedeps.insert(std::string(buf));
      }

      // scan past eol
      while ((p < end) && (*p != '\n')) { p++; }
      while ((p < end) && ((*p == '\n') || (*p == '\r'))) { p++; }
    }

    const char * depfilename = compiler.deptracking.filename.c_str();
    // FIXME: handle compiler.deptracking.filename == "". 20010731 mortene.
    FILE * depfile = fopen(depfilename, "w");
    if (!depfile) {
      (void)fprintf(stderr, "Couldn't write to ``%s'': %s\n",
                    depfilename, strerror(errno));
      exit(1);
    }

    // FIXME: handle compiler.deptracking.targetobj == "". 20010731 mortene.
    // FIXME: detect failure to write. 20010731 mortene.
    (void)fprintf(depfile, "%s:", compiler.deptracking.targetobj.c_str());

    stringset_t::iterator i;
    for (i = includedeps.begin(); i != includedeps.end(); i++) {
      // FIXME: detect failure to write. 20010731 mortene.
      (void)fprintf(depfile, " \\\n\t%s", quotespaces(posixpath(*i)).c_str());
    }
    // FIXME: detect failure to write. 20010731 mortene.
    (void)fprintf(depfile, "\n\n");

    if (compiler.deptracking.phonytargets) {
      for (i = includedeps.begin(); i != includedeps.end(); i++) {
        // FIXME: detect failure to write. 20010731 mortene.
        (void)fprintf(depfile, "%s:\n", quotespaces(posixpath(*i)).c_str());
      }
    }

    if (fclose(depfile) != 0) {
      (void)fprintf(stderr, "Couldn't close ``%s'': %s\n",
                    depfilename, strerror(errno));
      exit(1);
    }
  }

  cmd = tool->getToolName() + (" /nologo " + tool->constructArguments());

  std::string procstdout, procstderr;
  DWORD exitcode = run_process(cmd.c_str(), procstdout, procstderr);

  // The geniuses at Microsoft push error messages from ``cl.exe'' out
  // on stdout -- _unless_ ``cl.exe'' is run in preprocessor modus
  // (with the /E option), then stdout will be occupied be the preproc
  // output, so errors must to go to stderr after all.
  bool printstdoutonstderr = exitcode != 0;
  if ((tool == &compiler) && compiler.aspreproc) { printstdoutonstderr = false; }
  (void)fprintf(printstdoutonstderr ? stderr : stdout, "%s", procstdout.c_str());
  (void)fprintf(stderr, "%s", procstderr.c_str());

  return exitcode;
}
