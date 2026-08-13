/*	TODO: bring this in line with stdint uint8_t etc.	*/

#ifndef LIBFLEX_FLEX_TYPES_H
#define LIBFLEX_FLEX_TYPES_H
/*
 *	Include guard added 2026-08-13. These headers had none, which was
 *	harmless while every consumer included each of them once. Baumol's
 *	firmware links libsets against a bare-metal target where SCCE's
 *	`llvm-link --only-needed` forces libsets and its host hooks into ONE
 *	translation unit to break a dependency cycle, so several .c files that
 *	each include flex.h land in the same unit. Without a guard that is a
 *	redefinition of every enumerator and typedef in the file.
 */
//typedef	char			bool;
typedef	unsigned char		uchar;
typedef unsigned short		ushort;
typedef	unsigned int		uint;
typedef	unsigned long		ulong;
typedef	unsigned long long	uvlong;
typedef	int			integer;
typedef	float			real;

#if defined FLEX64
#	define	FlexAddr	unsigned long long
#elif defined FLEX32
#	define	FlexAddr	unsigned long
#elif defined FLEX16
#	define FlexAddr		unsigned int
#elif defined  FLEX8
#	define FlexAddr		unsigned char
#else
#	error "You must define one of FLEX64/FLEX32/FLEX16/FLEX8"
#endif

#endif /* LIBFLEX_FLEX_TYPES_H */
