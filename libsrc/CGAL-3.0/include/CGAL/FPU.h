// Copyright (c) 1998,1999,2000,2001,2002  Utrecht University (The Netherlands),
// ETH Zurich (Switzerland), Freie Universitaet Berlin (Germany),
// INRIA Sophia-Antipolis (France), Martin-Luther-University Halle-Wittenberg
// (Germany), Max-Planck-Institute Saarbruecken (Germany), RISC Linz (Austria),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 of the License.
// See the file LICENSE.LGPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Source: /CVSROOT/CGAL/Packages/Interval_arithmetic/include/CGAL/FPU.h,v $
// $Revision: 1.19 $ $Date: 2003/10/21 12:17:21 $
// $Name: current_submission $
//
// Author(s)     : Sylvain Pion

#ifndef CGAL_FPU_H
#define CGAL_FPU_H

// This file specifies some platform dependant functions, regarding the FPU
// directed rounding modes.  There is only support for double precision.
//
// It also contains the definition of the Protect_FPU_rounding<> classes,
// a helper class which is a nice way to protect blocks of code needing a
// particular rounding mode.

#ifdef __MWERKS__
#  include <fenv.h>
#elif defined __alpha__  && defined __linux__ 
extern "C" {
#  include <fenv.h>
}
#elif defined __linux__ 
#  include <fpu_control.h>
#elif defined __SUNPRO_CC || (defined __KCC && defined __sun)
#  include <ieeefp.h>
#elif defined __osf || defined __osf__ 
#  ifdef __GNUG__
     // GCC seems to remove (fixincludes) read_rnd/write_rnd...
#    include "/usr/include/float.h"
#  else
#    include <float.h>
#  endif
#elif defined __BORLANDC__
#  include <float.h>
#elif defined __sgi
#  include <sys/fpu.h>
#elif defined _MSC_VER || defined __sparc__ || \
     (defined __i386__ && !defined __PGI)
   // Nothing to include.
#else
   // By default we use the ISO C99 version.
#  include <fenv.h>
#endif



// Some useful constants

#if defined CGAL_CFG_NO_LIMITS
#  define CGAL_IA_MIN_DOUBLE (5e-324)
#  define CGAL_IA_MAX_DOUBLE (1.7976931348623157081e+308)
#else
#  include <limits>
#  define CGAL_IA_MIN_DOUBLE std::numeric_limits<double>::denorm_min()
#  define CGAL_IA_MAX_DOUBLE std::numeric_limits<double>::max()
#endif

CGAL_BEGIN_NAMESPACE


// Inline function to stop compiler optimization.
inline double IA_force_to_double(double x)
{
#if defined __GNUG__ && \
    ! (__GNUG__ == 3 && __GNUC_MINOR__ == 0 && __GNUC_PATCHLEVEL__ == 0)
  // This appears to be faster but is GNU specific,
  // and GCC 3.0.0 has a bug with it.
  asm("" : "=m"(x) : "m"(x));
  // asm("" : "+m"(x) );
  return x;
#else
  volatile double e = x;
  return e;
#endif
}

// The x86 processor keeps too wide exponents (15bits) in registers, even in
// double precision mode !  It's a problem when the intervals overflow or
// underflow.  To work around that, we store the variable to memory when
// needed, using the macro below.
// Another possible workaround would be to use intervals of "long doubles"
// directly, but I think it would be much slower.
#if !defined (CGAL_IA_NO_X86_OVER_UNDER_FLOW_PROTECT) && \
    (defined __i386__ || defined _MSC_VER || defined __BORLANDC__)
#  define CGAL_IA_FORCE_TO_DOUBLE(x) CGAL::IA_force_to_double(x)
#else
#  define CGAL_IA_FORCE_TO_DOUBLE(x) (x)
#endif

// We sometimes need to stop constant propagation,
// because operations are done with a wrong rounding mode at compile time.
// G++ also uses __builtin_constant_p().
#ifndef CGAL_IA_DONT_STOP_CONSTANT_PROPAGATION
#  if defined __GNUG__ && __GNUG__ < 3
	// Note : GCC 3 doesn't guarantee __builtin_constant_p to return false
	// when he will not do cprop :(.
#    define CGAL_IA_STOP_CPROP(x) \
            (__builtin_constant_p (x) ? CGAL::IA_force_to_double(x) : (x) )
#    define CGAL_IA_STOP_CPROP2(x,y) \
            (__builtin_constant_p (y) ? CGAL_IA_STOP_CPROP(x) : (x) )
#  else
#    define CGAL_IA_STOP_CPROP(x)    CGAL::IA_force_to_double(x)
#    define CGAL_IA_STOP_CPROP2(x,y) CGAL::IA_force_to_double(x)
#  endif
#else
#  define CGAL_IA_STOP_CPROP(x)    (x)
#  define CGAL_IA_STOP_CPROP2(x,y) (x)
#endif

// std::sqrt(double) on VC++ and CygWin is buggy when not optimizing.
#ifdef _MSC_VER
inline double IA_bug_sqrt(double d)
{
  _asm
  {
    fld d
    fsqrt
    fstp d
  }
  return d;
}
#  define CGAL_BUG_SQRT(d) CGAL::IA_bug_sqrt(d)
#elif defined __CYGWIN__
inline double IA_bug_sqrt(double d)
{
  double r;
  asm volatile ("fsqrt" : "=t"(r) : "0"(d));
  return r;
}
#  define CGAL_BUG_SQRT(d) CGAL::IA_bug_sqrt(d)
#else
#  define CGAL_BUG_SQRT(d) CGAL_CLIB_STD::sqrt(d)
#endif

// Here are the operator macros that make use of the above.
// With GCC, we can do slightly better : test with __builtin_constant_p()
// that both arguments are constant before stopping one of them.
// Use inline functions instead ?
#define CGAL_IA_ADD(a,b) CGAL_IA_FORCE_TO_DOUBLE((a)+CGAL_IA_STOP_CPROP2(b,a))
#define CGAL_IA_SUB(a,b) CGAL_IA_FORCE_TO_DOUBLE((a)-CGAL_IA_STOP_CPROP2(b,a))
#define CGAL_IA_MUL(a,b) CGAL_IA_FORCE_TO_DOUBLE((a)*CGAL_IA_STOP_CPROP2(b,a))
#define CGAL_IA_DIV(a,b) CGAL_IA_FORCE_TO_DOUBLE((a)/CGAL_IA_STOP_CPROP2(b,a))
#define CGAL_IA_SQUARE(a) CGAL_IA_MUL(a,a)
#define CGAL_IA_SQRT(a) \
        CGAL_IA_FORCE_TO_DOUBLE(CGAL_BUG_SQRT(CGAL_IA_STOP_CPROP(a)))

#ifdef __STD_IEC_559__
// This is a version for the ISO C99 standard, which aims at portability.
// It should work with GNU libc 2.1.  Not tested yet.
#define CGAL_IA_SETFPCW(CW) fesetround(CW)
#define CGAL_IA_GETFPCW(CW) CW = fegetround()
typedef fpu_control_t FPU_CW_t;
#define CGAL_FE_TONEAREST    FE_TONEAREST
#define CGAL_FE_TOWARDZERO   FE_TOWARDZERO
#define CGAL_FE_UPWARD       FE_UPWARD
#define CGAL_FE_DOWNWARD     FE_DOWNWARD

#elif defined __MWERKS__
#define CGAL_IA_SETFPCW(CW) fesetround(CW)
#define CGAL_IA_GETFPCW(CW) CW = fegetround()
typedef int FPU_CW_t;
#define CGAL_FE_TONEAREST    FE_TONEAREST
#define CGAL_FE_TOWARDZERO   FE_TOWARDZERO
#define CGAL_FE_UPWARD       FE_UPWARD
#define CGAL_FE_DOWNWARD     FE_DOWNWARD

#elif defined __i386__
// The GNU libc version (cf powerpc) is nicer, but doesn't work on libc 5 :(
// This one also works with CygWin.
#define CGAL_IA_SETFPCW(CW) asm volatile ("fldcw %0" : :"m" (CW))
#define CGAL_IA_GETFPCW(CW) asm volatile ("fnstcw %0" : "=m" (CW))
typedef unsigned short FPU_CW_t;
#define CGAL_FE_TONEAREST    (0x000 | 0x127f)
#define CGAL_FE_TOWARDZERO   (0xc00 | 0x127f)
#define CGAL_FE_UPWARD       (0x800 | 0x127f)
#define CGAL_FE_DOWNWARD     (0x400 | 0x127f)

#elif defined __powerpc__  
#define CGAL_IA_SETFPCW(CW) _FPU_SETCW(CW)
#define CGAL_IA_GETFPCW(CW) _FPU_GETCW(CW)
typedef fpu_control_t FPU_CW_t;
#define CGAL_FE_TONEAREST    (_FPU_RC_NEAREST | _FPU_DEFAULT)
#define CGAL_FE_TOWARDZERO   (_FPU_RC_ZERO    | _FPU_DEFAULT)
#define CGAL_FE_UPWARD       (_FPU_RC_UP      | _FPU_DEFAULT)
#define CGAL_FE_DOWNWARD     (_FPU_RC_DOWN    | _FPU_DEFAULT)

#elif defined __SUNPRO_CC || (defined __KCC && defined __sun)
#define CGAL_IA_SETFPCW(CW) fpsetround(fp_rnd(CW))
#define CGAL_IA_GETFPCW(CW) CW = fpgetround()
typedef unsigned int FPU_CW_t;
#define CGAL_FE_TONEAREST    FP_RN
#define CGAL_FE_TOWARDZERO   FP_RZ
#define CGAL_FE_UPWARD       FP_RP
#define CGAL_FE_DOWNWARD     FP_RM

#elif defined __sparc__
#define CGAL_IA_SETFPCW(CW) asm volatile ("ld %0,%%fsr" : :"m" (CW))
#define CGAL_IA_GETFPCW(CW) asm volatile ("st %%fsr,%0" : "=m" (CW))
typedef unsigned int FPU_CW_t;
#define CGAL_FE_TONEAREST    (0x0        | 0x20000000 | 0x1f)
#define CGAL_FE_TOWARDZERO   (0x40000000 | 0x20000000 | 0x1f)
#define CGAL_FE_UPWARD       (0x80000000 | 0x20000000 | 0x1f)
#define CGAL_FE_DOWNWARD     (0xc0000000 | 0x20000000 | 0x1f)

#elif defined __sgi
typedef unsigned int FPU_CW_t;

inline FPU_CW_t sgi_get_fpu_cw()
{
  fpc_csr csr;
  csr.fc_word = get_fpc_csr();
  return csr.fc_struct.rounding_mode;
}

inline void sgi_set_fpu_cw(FPU_CW_t cw)
{
  fpc_csr csr;
  csr.fc_word = get_fpc_csr();
  csr.fc_struct.rounding_mode = cw;
  csr.fc_struct.flush = 0; // By default, denormals are flushed to zero !
  set_fpc_csr(csr.fc_word);
}

#define CGAL_IA_SETFPCW(CW)  sgi_set_fpu_cw(CW)
#define CGAL_IA_GETFPCW(CW)  CW = sgi_get_fpu_cw()
#define CGAL_FE_TONEAREST    ROUND_TO_NEAREST
#define CGAL_FE_TOWARDZERO   ROUND_TO_ZERO
#define CGAL_FE_UPWARD       ROUND_TO_PLUS_INFINITY
#define CGAL_FE_DOWNWARD     ROUND_TO_MINUS_INFINITY

#elif defined __mips__ // && !defined __sgi
#define CGAL_IA_SETFPCW(CW) asm volatile ("ctc1 %0,$31" : :"r" (CW))
#define CGAL_IA_GETFPCW(CW) asm volatile ("cfc1 %0,$31" : "=r" (CW))
typedef unsigned int FPU_CW_t;
#define CGAL_FE_TONEAREST    (0x0)
#define CGAL_FE_TOWARDZERO   (0x1)
#define CGAL_FE_UPWARD       (0x2)
#define CGAL_FE_DOWNWARD     (0x3)

#elif defined __osf || defined __osf__  // Not yet supported.
#define CGAL_IA_SETFPCW(CW) write_rnd(CW)
#define CGAL_IA_GETFPCW(CW) CW = read_rnd()
typedef unsigned int FPU_CW_t;
#define CGAL_FE_TONEAREST    FP_RND_RN
#define CGAL_FE_TOWARDZERO   FP_RND_RZ
#define CGAL_FE_UPWARD       FP_RND_RP
#define CGAL_FE_DOWNWARD     FP_RND_RM

#elif defined __alpha__  // preliminary suppor.
#define CGAL_IA_SETFPCW(CW) (__ieee_set_fp_control(CW))
#define CGAL_IA_GETFPCW(CW) (CW = __ieee_get_fp_control())
typedef unsigned long FPU_CW_t;
#define CGAL_FE_TONEAREST   FE_TONEAREST
#define CGAL_FE_TOWARDZERO  FE_TOWARDZERO 
#define CGAL_FE_UPWARD      FE_UPWARD 
#define CGAL_FE_DOWNWARD    FE_DOWNWARD 

#elif defined _MSC_VER
// Found in http://msdn.microsoft.com/library/sdkdoc/directx/imover_7410.htm :
#define CGAL_IA_SETFPCW(CW) __asm fldcw (CW)
#define CGAL_IA_GETFPCW(CW) __asm fstcw (CW)
typedef unsigned short FPU_CW_t;
#define CGAL_FE_TONEAREST    (0x0   | 0x127f)
#define CGAL_FE_TOWARDZERO   (0xC00 | 0x127f)
#define CGAL_FE_UPWARD       (0x800 | 0x127f)
#define CGAL_FE_DOWNWARD     (0x400 | 0x127f)

#elif defined __BORLANDC__
#define CGAL_IA_SETFPCW(CW) _control87(CW,~0)
#define CGAL_IA_GETFPCW(CW) CW = _control87(0,0)
typedef unsigned short FPU_CW_t;
#define CGAL_FE_TONEAREST    (0x0   | 0x127f)
#define CGAL_FE_TOWARDZERO   (0xC00 | 0x127f)
#define CGAL_FE_UPWARD       (0x800 | 0x127f)
#define CGAL_FE_DOWNWARD     (0x400 | 0x127f)

#else
// This is a version following the ISO C99 standard, which aims at portability.
// The drawbacks are speed on one hand, and also, on x86, it doesn't fix the
// extended mantissa issue (this is not a problem for IA, but it is one for
// Fixed_precision_nt, and some future modular computations as well).
#define CGAL_IA_SETFPCW(CW)  fesetround(CW)
#define CGAL_IA_GETFPCW(CW)  CW = fegetround()
typedef int FPU_CW_t;
#define CGAL_FE_TONEAREST    FE_TONEAREST
#define CGAL_FE_TOWARDZERO   FE_TOWARDZERO
#define CGAL_FE_UPWARD       FE_UPWARD
#define CGAL_FE_DOWNWARD     FE_DOWNWARD
#endif

// User interface:

inline
FPU_CW_t
FPU_get_cw (void)
{
#if defined __GNUG__ && \
    (__GNUG__ == 3 && __GNUC_MINOR__ == 0 && __GNUC_PATCHLEVEL__ == 0)
    volatile
#endif
    FPU_CW_t cw;
    CGAL_IA_GETFPCW(cw);
    return cw;
}

inline
void
FPU_set_cw (FPU_CW_t cw)
{
    CGAL_IA_SETFPCW(cw);
}

inline
FPU_CW_t
FPU_get_and_set_cw (FPU_CW_t cw)
{
    FPU_CW_t old = FPU_get_cw();
    FPU_set_cw(cw);
    return old;
}

FPU_CW_t FPU_empiric_test(); // Only used for debug.

void force_ieee_double_precision(); // Used by the Fixed_precision_nt.

// A class whose constructor sets the FPU mode to +inf, saves a backup of it,
// and whose destructor resets it back to the saved state.

template <bool Protected> struct Protect_FPU_rounding;
 
template <>
struct Protect_FPU_rounding<true>
{
  Protect_FPU_rounding(FPU_CW_t r = CGAL_FE_UPWARD)
    : backup( FPU_get_and_set_cw(r) ) {}

  ~Protect_FPU_rounding()
  {
     FPU_set_cw(backup);
  }

private:
  FPU_CW_t backup;
};
 
template <>
struct Protect_FPU_rounding<false>
{
  Protect_FPU_rounding(FPU_CW_t CGAL_expensive_assertion_code(r)
		        = CGAL_FE_UPWARD)
  {
    CGAL_expensive_assertion(FPU_empiric_test() == r);
  }

  ~Protect_FPU_rounding() {}
  // just to shut up a warning, but it has a performance issue with GCC 2.95,
  // so I should disable it for the moment.  Use __attribute__((unused)) ?
};

CGAL_END_NAMESPACE

#endif // CGAL_FPU_H
