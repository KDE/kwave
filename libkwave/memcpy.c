/*
 * Copyright (C) 2001-2014 the xine project
 *
 * This file is part of xine, a free video player.
 *
 * xine is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * xine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 * These are the MMX/MMX2/SSE optimized versions of memcpy
 *
 * This code was adapted from Linux Kernel sources by Nick Kurshev to
 * the mplayer program. (http://mplayer.sourceforge.net)
 *
 * Miguel Freitas split the #ifdefs into several specialized functions that
 * are benchmarked at runtime by xine. Some original comments from Nick
 * have been preserved documenting some MMX/SSE oddities.
 * Also added kernel memcpy function that seems faster than libc one.
 *
 * 2004-12-06
 *   Copied this source into the Kwave project and adapted it to compile
 *   cleanly within this new environment
 *   by Thomas Eschenbacher <Thomas.Eschenbacher@gmx.de>
 *   Marked most changes with "#ifdef XINE_COMPILE"
 *
 * 2009-09-12
 *   synced with latest cvs version from sourceforge.net,
 *   xine/xine-lib/src/xine-utils/memcpy.c, rev. 1.44, 2007-07-20 20:00:36
 *
 * 2014-05-26
 *   synced with latest hg version, xine-lib-1-2-02e5a69f56c9
 *
 * 2015-09-19
 *   keyword "extern" was missing for probe_fast_memcpy()
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if defined (ARCH_PPC) && !defined (HOST_OS_DARWIN)
#include "ppcasm_string.h"
#endif

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#else
#include <time.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdint.h> /* for uint_64 */

#ifndef XINE_COMPILE

#include "cputest.h"
#include <stdio.h>

#define _(m) m
#define xprintf printf

extern void probe_fast_memcpy(void) ;
extern void *(* xine_fast_memcpy)(void *to, const void *from, size_t len);

#define LOG_MODULE "memcpy"
#define LOG_VERBOSE
#define HAVE_AVX

#else /* XINE_COMPILE */

/*
#define LOG
*/

#include <xine/xine_internal.h>
#include "../xine-engine/xine_private.h"

#endif /* XINE_COMPILE */

void *(* xine_fast_memcpy)(void *to, const void *from, size_t len);

/* Original comments from mplayer (file: aclib.c)
 This part of code was taken by me from Linux-2.4.3 and slightly modified
for MMX, MMX2, SSE instruction set. I have done it since linux uses page aligned
blocks but mplayer uses weakly ordered data and original sources can not
speedup them. Only using PREFETCHNTA and MOVNTQ together have effect!

>From IA-32 Intel Architecture Software Developer's Manual Volume 1,

Order Number 245470:
"10.4.6. Cacheability Control, Prefetch, and Memory Ordering Instructions"

Data referenced by a program can be temporal (data will be used again) or
non-temporal (data will be referenced once and not reused in the immediate
future). To make efficient use of the processor's caches, it is generally
desirable to cache temporal data and not cache non-temporal data. Overloading
the processor's caches with non-temporal data is sometimes referred to as
"polluting the caches".
The non-temporal data is written to memory with Write-Combining semantics.

The PREFETCHh instructions permits a program to load data into the processor
at a suggested cache level, so that it is closer to the processors load and
store unit when it is needed. If the data is already present in a level of
the cache hierarchy that is closer to the processor, the PREFETCHh instruction
will not result in any data movement.
But we should you PREFETCHNTA: Non-temporal data fetch data into location
close to the processor, minimizing cache pollution.

The MOVNTQ (store quadword using non-temporal hint) instruction stores
packed integer data from an MMX register to memory, using a non-temporal hint.
The MOVNTPS (store packed single-precision floating-point values using
non-temporal hint) instruction stores packed floating-point data from an
XMM register to memory, using a non-temporal hint.

The SFENCE (Store Fence) instruction controls write ordering by creating a
fence for memory store operations. This instruction guarantees that the results
of every store instruction that precedes the store fence in program order is
globally visible before any store instruction that follows the fence. The
SFENCE instruction provides an efficient way of ensuring ordering between
procedures that produce weakly-ordered data and procedures that consume that
data.

If you have questions please contact with me: Nick Kurshev: nickols_k@mail.ru.
*/

/*  mmx v.1 Note: Since we added alignment of destinition it speedups
    of memory copying on PentMMX, Celeron-1 and P2 upto 12% versus
    standard (non MMX-optimized) version.
    Note: on K6-2+ it speedups memory copying upto 25% and
          on K7 and P3 about 500% (5 times).
*/

/* Additional notes on gcc assembly and processors: [MF]
prefetch is specific for AMD processors, the intel ones should be
prefetch0, prefetch1, prefetch2 which are not recognized by my gcc.
prefetchnta is supported both on athlon and pentium 3.

therefore i will take off prefetchnta instructions from the mmx1 version
to avoid problems on pentium mmx and k6-2.

quote of the day:
"Using prefetches efficiently is more of an art than a science"
*/


#if defined(ARCH_X86) || defined(ARCH_X86_64)

#ifndef _MSC_VER
/* for small memory blocks (<256 bytes) this version is faster */
#define small_memcpy(to,from,n)\
{\
register uintptr_t dummy;\
__asm__ __volatile__(\
  "rep; movsb"\
  :"=&D"(to), "=&S"(from), "=&c"(dummy)\
  :"0" (to), "1" (from),"2" (n)\
  : "memory");\
}

/* linux kernel __memcpy (from: /include/asm/string.h) */
static __inline__ void * linux_kernel_memcpy_impl (
			       void * to,
			       const void * from,
			       size_t n)
{
int d0, d1, d2;

  if( n < 4 ) {
    small_memcpy(to,from,n);
  }
  else
    __asm__ __volatile__(
    "rep ; movsl\n\t"
    "testb $2,%b4\n\t"
    "je 1f\n\t"
    "movsw\n"
    "1:\ttestb $1,%b4\n\t"
    "je 2f\n\t"
    "movsb\n"
    "2:"
    : "=&c" (d0), "=&D" (d1), "=&S" (d2)
    :"0" (n/4), "q" (n),"1" ((uintptr_t) to),"2" ((uintptr_t) from)
    : "memory");

  return (to);
}

#define AVX_MMREG_SIZE 32
#define SSE_MMREG_SIZE 16
#define MMX_MMREG_SIZE 8

#define MMX1_MIN_LEN 0x800  /* 2K blocks */
#define MIN_LEN 0x40  /* 64-byte blocks */

/* SSE note: i tried to move 128 bytes a time instead of 64 but it
didn't make any measureable difference. i'm using 64 for the sake of
simplicity. [MF] */
static void * sse_memcpy(void * to, const void * from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;

  /* PREFETCH has effect even for MOVSB instruction ;) */
  __asm__ __volatile__ (
    "   prefetchnta (%0)\n"
    "   prefetchnta 32(%0)\n"
    "   prefetchnta 64(%0)\n"
    "   prefetchnta 96(%0)\n"
    "   prefetchnta 128(%0)\n"
    "   prefetchnta 160(%0)\n"
    "   prefetchnta 192(%0)\n"
    "   prefetchnta 224(%0)\n"
    "   prefetchnta 256(%0)\n"
    "   prefetchnta 288(%0)\n"
    : : "r" (from) );

  if(len >= MIN_LEN)
  {
    register uintptr_t delta;
    /* Align destinition to MMREG_SIZE -boundary */
    delta = ((uintptr_t)to)&(SSE_MMREG_SIZE-1);
    if(delta)
    {
      delta=SSE_MMREG_SIZE-delta;
      len -= delta;
      small_memcpy(to, from, delta);
    }
    i = len >> 6; /* len/64 */
    len&=63;
    if(((uintptr_t)from) & 15)
      /* if SRC is misaligned */
      for(; i>0; i--)
      {
        __asm__ __volatile__ (
        "prefetchnta 320(%0)\n"
       "prefetchnta 352(%0)\n"
        "movups (%0), %%xmm0\n"
        "movups 16(%0), %%xmm1\n"
        "movups 32(%0), %%xmm2\n"
        "movups 48(%0), %%xmm3\n"
        "movntps %%xmm0, (%1)\n"
        "movntps %%xmm1, 16(%1)\n"
        "movntps %%xmm2, 32(%1)\n"
        "movntps %%xmm3, 48(%1)\n"
        :: "r" (from), "r" (to) : "memory");
        from = ((const unsigned char *)from) + 64;
        to = ((unsigned char *)to) + 64;
      }
    else
      /*
         Only if SRC is aligned on 16-byte boundary.
         It allows to use movaps instead of movups, which required data
         to be aligned or a general-protection exception (#GP) is generated.
      */
      for(; i>0; i--)
      {
        __asm__ __volatile__ (
        "prefetchnta 320(%0)\n"
       "prefetchnta 352(%0)\n"
        "movaps (%0), %%xmm0\n"
        "movaps 16(%0), %%xmm1\n"
        "movaps 32(%0), %%xmm2\n"
        "movaps 48(%0), %%xmm3\n"
        "movntps %%xmm0, (%1)\n"
        "movntps %%xmm1, 16(%1)\n"
        "movntps %%xmm2, 32(%1)\n"
        "movntps %%xmm3, 48(%1)\n"
        :: "r" (from), "r" (to) : "memory");
        from = ((const unsigned char *)from) + 64;
        to = ((unsigned char *)to) + 64;
      }
    /* since movntq is weakly-ordered, a "sfence"
     * is needed to become ordered again. */
    __asm__ __volatile__ ("sfence":::"memory");
  }
  /*
   *	Now do the tail of the block
   */
  if(len) linux_kernel_memcpy_impl(to, from, len);
  return retval;
}

#ifdef HAVE_AVX
static void * avx_memcpy(void * to, const void * from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;

  /* PREFETCH has effect even for MOVSB instruction ;) */
  __asm__ __volatile__ (
    "   prefetchnta (%0)\n"
    "   prefetchnta 32(%0)\n"
    "   prefetchnta 64(%0)\n"
    "   prefetchnta 96(%0)\n"
    "   prefetchnta 128(%0)\n"
    "   prefetchnta 160(%0)\n"
    "   prefetchnta 192(%0)\n"
    "   prefetchnta 224(%0)\n"
    "   prefetchnta 256(%0)\n"
    "   prefetchnta 288(%0)\n"
    : : "r" (from) );

  if(len >= MIN_LEN)
  {
    register uintptr_t delta;
    /* Align destinition to MMREG_SIZE -boundary */
    delta = ((uintptr_t)to)&(AVX_MMREG_SIZE-1);
    if(delta)
    {
      delta=AVX_MMREG_SIZE-delta;
      len -= delta;
      small_memcpy(to, from, delta);
    }
    i = len >> 7; /* len/128 */
    len&=127;
    if(((uintptr_t)from) & 31)
      /* if SRC is misaligned */
      for(; i>0; i--)
      {
        __asm__ __volatile__ (
        "prefetchnta 320(%0)\n"
        "prefetchnta 352(%0)\n"
        "prefetchnta 384(%0)\n"
        "prefetchnta 416(%0)\n"
        "vmovups    (%0), %%ymm0\n"
        "vmovups  32(%0), %%ymm1\n"
        "vmovups  64(%0), %%ymm2\n"
        "vmovups  96(%0), %%ymm3\n"
        "vmovntps %%ymm0,   (%1)\n"
        "vmovntps %%ymm1, 32(%1)\n"
        "vmovntps %%ymm2, 64(%1)\n"
        "vmovntps %%ymm3, 96(%1)\n"
        :: "r" (from), "r" (to) : "memory");
        from = ((const unsigned char *)from) + 128;
        to = ((unsigned char *)to) + 128;
      }
    else
      /*
         Only if SRC is aligned on 16-byte boundary.
         It allows to use movaps instead of movups, which required data
         to be aligned or a general-protection exception (#GP) is generated.
      */
      for(; i>0; i--)
      {
        __asm__ __volatile__ (
        "prefetchnta 320(%0)\n"
        "prefetchnta 352(%0)\n"
        "prefetchnta 384(%0)\n"
        "prefetchnta 416(%0)\n"
        "vmovaps    (%0), %%ymm0\n"
        "vmovaps  32(%0), %%ymm1\n"
        "vmovaps  64(%0), %%ymm2\n"
        "vmovaps  96(%0), %%ymm3\n"
        "vmovntps %%ymm0,   (%1)\n"
        "vmovntps %%ymm1, 32(%1)\n"
        "vmovntps %%ymm2, 64(%1)\n"
        "vmovntps %%ymm3, 96(%1)\n"
        :: "r" (from), "r" (to) : "memory");
        from = ((const unsigned char *)from) + 128;
        to = ((unsigned char *)to) + 128;
      }
    /* since movntq is weakly-ordered, a "sfence"
     * is needed to become ordered again. */
    __asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("vzeroupper");
  }
  /*
   *	Now do the tail of the block
   */
  if(len) linux_kernel_memcpy_impl(to, from, len);
  return retval;
}
#endif /* HAVE_AVX */

static void * mmx_memcpy(void * to, const void * from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;

  if(len >= MMX1_MIN_LEN)
  {
    register uintptr_t delta;
    /* Align destinition to MMREG_SIZE -boundary */
    delta = ((uintptr_t)to)&(MMX_MMREG_SIZE-1);
    if(delta)
    {
      delta=MMX_MMREG_SIZE-delta;
      len -= delta;
      small_memcpy(to, from, delta);
    }
    i = len >> 6; /* len/64 */
    len&=63;
    for(; i>0; i--)
    {
      __asm__ __volatile__ (
      "movq (%0), %%mm0\n"
      "movq 8(%0), %%mm1\n"
      "movq 16(%0), %%mm2\n"
      "movq 24(%0), %%mm3\n"
      "movq 32(%0), %%mm4\n"
      "movq 40(%0), %%mm5\n"
      "movq 48(%0), %%mm6\n"
      "movq 56(%0), %%mm7\n"
      "movq %%mm0, (%1)\n"
      "movq %%mm1, 8(%1)\n"
      "movq %%mm2, 16(%1)\n"
      "movq %%mm3, 24(%1)\n"
      "movq %%mm4, 32(%1)\n"
      "movq %%mm5, 40(%1)\n"
      "movq %%mm6, 48(%1)\n"
      "movq %%mm7, 56(%1)\n"
      :: "r" (from), "r" (to) : "memory");
      from = ((const unsigned char *)from) + 64;
      to = ((unsigned char *)to) + 64;
    }
    __asm__ __volatile__ ("emms":::"memory");
  }
  /*
   *	Now do the tail of the block
   */
  if(len) linux_kernel_memcpy_impl(to, from, len);
  return retval;
}

static void * mmx2_memcpy(void * to, const void * from, size_t len)
{
  void *retval;
  size_t i;
  retval = to;

  /* PREFETCH has effect even for MOVSB instruction ;) */
  __asm__ __volatile__ (
    "   prefetchnta (%0)\n"
    "   prefetchnta 32(%0)\n"
    "   prefetchnta 64(%0)\n"
    "   prefetchnta 96(%0)\n"
    "   prefetchnta 128(%0)\n"
    "   prefetchnta 160(%0)\n"
    "   prefetchnta 192(%0)\n"
    "   prefetchnta 224(%0)\n"
    "   prefetchnta 256(%0)\n"
    "   prefetchnta 288(%0)\n"
    : : "r" (from) );

  if(len >= MIN_LEN)
  {
    register uintptr_t delta;
    /* Align destinition to MMREG_SIZE -boundary */
    delta = ((uintptr_t)to)&(MMX_MMREG_SIZE-1);
    if(delta)
    {
      delta=MMX_MMREG_SIZE-delta;
      len -= delta;
      small_memcpy(to, from, delta);
    }
    i = len >> 6; /* len/64 */
    len&=63;
    for(; i>0; i--)
    {
      __asm__ __volatile__ (
      "prefetchnta 320(%0)\n"
      "prefetchnta 352(%0)\n"
      "movq (%0), %%mm0\n"
      "movq 8(%0), %%mm1\n"
      "movq 16(%0), %%mm2\n"
      "movq 24(%0), %%mm3\n"
      "movq 32(%0), %%mm4\n"
      "movq 40(%0), %%mm5\n"
      "movq 48(%0), %%mm6\n"
      "movq 56(%0), %%mm7\n"
      "movntq %%mm0, (%1)\n"
      "movntq %%mm1, 8(%1)\n"
      "movntq %%mm2, 16(%1)\n"
      "movntq %%mm3, 24(%1)\n"
      "movntq %%mm4, 32(%1)\n"
      "movntq %%mm5, 40(%1)\n"
      "movntq %%mm6, 48(%1)\n"
      "movntq %%mm7, 56(%1)\n"
      :: "r" (from), "r" (to) : "memory");
      from = ((const unsigned char *)from) + 64;
      to = ((unsigned char *)to) + 64;
    }
     /* since movntq is weakly-ordered, a "sfence"
     * is needed to become ordered again. */
    __asm__ __volatile__ ("sfence":::"memory");
    __asm__ __volatile__ ("emms":::"memory");
  }
  /*
   *	Now do the tail of the block
   */
  if(len) linux_kernel_memcpy_impl(to, from, len);
  return retval;
}

static void *linux_kernel_memcpy(void *to, const void *from, size_t len) {
  return linux_kernel_memcpy_impl(to,from,len);
}
#endif /* _MSC_VER */
#endif /* ARCH_X86 */

static const struct {
  const char name[16];
  void *(*const  function)(void *to, const void *from, size_t len);

  uint32_t cpu_require;
} memcpy_method[] =
{
  { "", NULL, 0 },
  { "libc", memcpy, 0 },
#if (defined(ARCH_X86) || defined(ARCH_X86_64)) && !defined(_MSC_VER)
  { "linux kernel", linux_kernel_memcpy, 0 },
  { "MMX ", mmx_memcpy, MM_MMX },
  { "MMXEXT", mmx2_memcpy, MM_MMXEXT },
  { "SSE", sse_memcpy, MM_MMXEXT|MM_SSE },
# ifdef HAVE_AVX
  { "AVX", avx_memcpy, MM_ACCEL_X86_AVX },
# endif /* HAVE_AVX */
#endif /* ARCH_X86 */
#if defined (ARCH_PPC) && !defined (HOST_OS_DARWIN)
  { "ppcasm", ppcasm_memcpy, 0 },
  { "ppcasm_cached", ppcasm_cacheable_memcpy, MM_ACCEL_PPC_CACHE32 },
#endif /* ARCH_PPC && !HOST_OS_DARWIN */
  { "", NULL, 0 }
};

static uint64_t memcpy_timing[sizeof(memcpy_method)/sizeof(memcpy_method[0])] = { 0, };

#ifdef HAVE_POSIX_TIMERS
/* Prefer clock_gettime() where available. */

# ifndef CLOCK_THREAD_CPUTIME_ID
/*  not defined in NetBSD (bug #535) */
#   define CLOCK_THREAD_CPUTIME_ID CLOCK_MONOTONIC
# endif

static int64_t _x_gettime(void)
{
  struct timespec tm;
  return (clock_gettime (CLOCK_THREAD_CPUTIME_ID, &tm) == -1)
       ? times (NULL)
       : (int64_t)tm.tv_sec * 1e9 + tm.tv_nsec;
}
#  define rdtsc(x) _x_gettime()

#elif (defined(ARCH_X86) || defined(ARCH_X86_64)) && defined(HAVE_SYS_TIMES_H)
static int64_t rdtsc(int config_flags)
{
  int64_t x;

  /* that should prevent us from trying cpuid with old cpus */
  if( config_flags & MM_MMX ) {
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
  } else {
    return times(NULL);
  }
}
#else

static uint64_t rdtsc(int config_flags)
{
  (void)config_flags;
  /* FIXME: implement an equivalent for using optimized memcpy on other
            architectures */
#ifdef HAVE_SYS_TIMES_H
  struct tms tp;
  return times(&tp);
#else
	return clock();
#endif /* HAVE_SYS_TIMES_H */
}
#endif

#ifdef XINE_COMPILE
static void update_fast_memcpy(void *user_data, xine_cfg_entry_t *entry) {
  static int   config_flags = -1;
  xine_t      *xine = (xine_t *) user_data;
  int          method;

  config_flags = xine_mm_accel();

  method = entry->num_value;

  if (method != 0
      && (config_flags & memcpy_method[method].cpu_require) ==
      memcpy_method[method].cpu_require ) {
    lprintf("using %s memcpy()\n", memcpy_method[method].name );
    xine_fast_memcpy = memcpy_method[method].function;
    return;
  } else {
    xprintf(xine, XINE_VERBOSITY_DEBUG, "xine: will probe memcpy on startup\n" );
  }
}
#endif /* XINE_COMPILE */

#define BUFSIZE 1024*1024
#ifdef XINE_COMPILE
void xine_probe_fast_memcpy(xine_t *xine)
#else
void probe_fast_memcpy(void)
#endif /* XINE_COMPILE */
{
  uint64_t          t;
  char             *buf1, *buf2;
  int               i, j, best;
  int               config_flags = -1;

#ifdef XINE_COMPILE

  static const char *const memcpy_methods[] = {
    "probe", "libc",
#if (defined(ARCH_X86) || defined(ARCH_X86_64)) && !defined(_MSC_VER)
    "kernel", "mmx", "mmxext", "sse",
# ifdef HAVE_AVX
    "avx",
# endif /* HAVE_AVX */
#endif
#if defined (ARCH_PPC) && !defined (HOST_OS_DARWIN)
    "ppcasm_memcpy", "ppcasm_cacheable_memcpy",
#endif
    NULL
  };

  config_flags = xine_mm_accel();

  best = xine->config->register_enum (xine->config, "engine.performance.memcpy_method", 0,
				      (char **)memcpy_methods,
				      _("memcopy method used by xine"),
				      _("The copying of large memory blocks is one of the most "
					"expensive operations on todays computers. Therefore xine "
					"provides various tuned methods to do this copying. "
					"Usually, the best method is detected automatically."),
				      20, update_fast_memcpy, (void *) xine);

  /* check if function is configured and valid for this machine */
  if( best != 0 &&
      best < sizeof(memcpy_methods)/sizeof(memcpy_method[0]) &&
     (config_flags & memcpy_method[best].cpu_require) ==
      memcpy_method[best].cpu_require ) {
    lprintf("using %s memcpy()\n", memcpy_method[best].name );
    xine_fast_memcpy = memcpy_method[best].function;
    return;
  }
#else /* XINE_COMPILE */
  config_flags = xine_mm_accel();
#endif /* XINE_COMPILE */
  best = 0;

  xine_fast_memcpy = memcpy;

  if( (buf1 = malloc(BUFSIZE)) == NULL )
    return;

  if( (buf2 = malloc(BUFSIZE)) == NULL ) {
    free(buf1);
    return;
  }

  xprintf(_("Benchmarking memcpy methods (smaller is better):\n"));
  /* make sure buffers are present on physical memory */
  memset(buf1,0,BUFSIZE);
  memset(buf2,0,BUFSIZE);

  /* some initial activity to ensure that we're not running slowly :-) */
  for(j=0;j<50;j++) {
    memcpy_method[1].function(buf2,buf1,BUFSIZE);
    memcpy_method[1].function(buf1,buf2,BUFSIZE);
  }

  for(i=1; memcpy_method[i].name[0]; i++)
  {
    if( (config_flags & memcpy_method[i].cpu_require) !=
         memcpy_method[i].cpu_require )
      continue;

    t = rdtsc(config_flags);
    for(j=0;j<50;j++) {
      memcpy_method[i].function(buf2,buf1,BUFSIZE);
      memcpy_method[i].function(buf1,buf2,BUFSIZE);
    }

    t = rdtsc(config_flags) - t;
    memcpy_timing[i] = t;

    xprintf("\t%s : %lld\n", memcpy_method[i].name, (long long int)t);

    if( best == 0 || t < memcpy_timing[best] )
      best = i;
  }

#ifdef XINE_COMPILE
  xine->config->update_num (xine->config, "engine.performance.memcpy_method", best);
#else /* XINE_COMPILE */
  xprintf("using -> '%s'\n", memcpy_method[best].name);
  xine_fast_memcpy = memcpy_method[best].function;
#endif /* XINE_COMPILE */

  free(buf1);
  free(buf2);
}

