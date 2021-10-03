/* vi: set sw=4 ts=4: */

#ifndef __MKC_TYPES_INCLUDED__
#define __MKC_TYPES_INCLUDED__

#if (defined __amd64__) ||	(defined __amd64) || 	\
	(defined __x86_64__) || (defined __x86_64)
typedef unsigned long long	u64;
typedef unsigned int		u32;
typedef unsigned short		u16;
typedef unsigned char		u8;
typedef long long			i64;
typedef signed int			i32;
typedef signed short		i16;
typedef signed char			i8;
typedef const char			cc;
#else
	#error BIT64 only
#endif

#ifndef NULL
#define NULL	((void *)0)
#endif

#endif

/* eof */
