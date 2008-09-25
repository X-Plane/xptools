/*
 * Copyright (c) 2004, Laminar Research.
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
#include "ErrMsg.h"
#include <errno.h>

#if APL
	#if !defined(__MWERKS__)
	#include <Carbon/Carbon.h>
	#endif
#endif

static ErrFunc_f	efunc = NULL;
static void *		eref = NULL;

struct	ErrInfo { int code; const char * descrip; };

static ErrInfo sErrTable[] = {
	0,				"ok",
#if APL
  fnfErr,					"file not found",
  paramErr,                 "param err",
  dirFulErr,                "directory is full",
  dskFulErr,                "disk is full",
  nsvErr,                   "no such volume",
  ioErr,                    "io error",
  bdNamErr,                 "bad name",
  fnOpnErr,                 "file not open",
  eofErr,                   "end of file",
  posErr,                   "position error",
  mFulErr,                  "memory full",
  tmfoErr,                  "too many files open",
  fnfErr,                   "file not found",
  wPrErr,                   "write protected",
  fLckdErr,                 "file locked",
  vLckdErr,                 "volume locked",
  fBsyErr,                  "file busy",
  dupFNErr,                 "duplicate file name",
  opWrErr,                  "file already open with write permissions",
  rfNumErr,                 "refnum error",
  gfpErr,                   "get file position error",
  volOffLinErr,             "volume offline",
  permErr,                  "permissions error",
  nsDrvErr,                 "no such drive",
  noMacDskErr,              "not a mac disk",
  badMDBErr,                "bad master directory block",
  wrPermErr,                "write permissions error",
  dirNFErr,                 "directory not fouud",
  memFullErr,				"out of memory",
  afpAccessDenied,			"insufficient privileges for operation",
#endif

EAGAIN,			"Resource temporarily unavailable",
EDEADLK,		"Resource deadlock avoided",
ENAMETOOLONG,	"File name too long",
ENOSYS,			"Function not implemented",
ENOTEMPTY,		"Directory not empty",
EILSEQ,     	"Wide character encoding error",
E2BIG,			"Argument list too long",
EACCES,			"Permission denied",
EBADF,			"Bad file descriptor",
EBUSY,			"Device busy",
ECHILD,			"No child processes",
EDOM,			"Numerical argument out of domain",
EEXIST,			"File exists",
EFAULT,			"Bad address",
EFBIG,			"File too large",
//EFPOS,			"File Position Error",
EINTR,			"Interrupted system call",
EINVAL,			"Invalid argument",
EIO,			"Input/output error",
EISDIR,			"Is a directory",
EMFILE,			"Too many open files",
EMLINK,			"Too many links",
ENFILE,			"Too many open files in system",
ENODEV,			"Operation not supported by device",
ENOENT,			"No such file or directory",
//ENOERR,			"No error detected",
ENOEXEC,		"Exec format error",
ENOLCK,			"No locks available",
ENOMEM,			"Cannot allocate memory",
ENOSPC,			"No space left on device",
ENOTDIR,		"Not a directory",
ENOTTY,			"Inappropriate ioctl for device",
ENXIO,			"Device not configured",
EPERM,			"Operation not permitted",
EPIPE,			"Broken pipe",
ERANGE,			"Result too large",
EROFS,			"Read-only file system",
//ESIGPARM,		"Signal error",
ESPIPE,			"Illegal seek",
ESRCH,			"No such process",
//EUNKNOWN,		"Unknown error",
//EXDEV,			"Cross-device link",

	0, 0,
};

ErrFunc_f	InstallErrFunc(ErrFunc_f inFunc, void * inRef)
{
	ErrFunc_f old = efunc;
	efunc = inFunc;
	eref = inRef;
	return old;
}

static	const char * GetDescription(int err);

int	__ReportError(const char * msg, int err, const char * file, const char * m, int l)
{
	if (err == 0) return err;
	const char * descrip = GetDescription(err);
	if (efunc)
		efunc(msg, err, descrip, file, m, l, eref);
	return err;
}

static	const char * GetDescription(int err)
{
	int n = 0;
	while (sErrTable[n].descrip)
	{
		if (sErrTable[n].code == err)
			return sErrTable[n].descrip;
		++n;
	}
	return "unknown";
}
