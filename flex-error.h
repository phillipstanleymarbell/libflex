/*	TODO: per language,	*/

#ifndef LIBFLEX_FLEX_ERROR_H
#define LIBFLEX_FLEX_ERROR_H
/*
 *	Include guard added 2026-08-13. These headers had none, which was
 *	harmless while every consumer included each of them once. Baumol's
 *	firmware links libsets against a bare-metal target where SCCE's
 *	`llvm-link --only-needed` forces libsets and its host hooks into ONE
 *	translation unit to break a dependency cycle, so several .c files that
 *	each include flex.h land in the same unit. Without a guard that is a
 *	redefinition of every enumerator and typedef in the file.
 */
static const char Ebadfree[]		=	"attempt to free an unallocated block";
static const char Emallocfor[]		=	"malloc/calloc failed for";
static const char Emalloc[]		=	"malloc/calloc failed.";
static const char Ememblocks[]		=	"ran out of memblocks";
static const char Ememblockaddrs[]	=	"ran out of memblock addrs";
static const char Emallocaddrs[]	=	"could not allocate memory for memblocks addrs";
static const char Ereallocfor[]		=	"realloc failed for";
static const char Eunallocrealloc[]	=	"attempt to realloc an unallocated chunk";
static const char Eerrorstr[]		=	"Error";
static const char Eopen[]		=	"Could not open file";

#endif /* LIBFLEX_FLEX_ERROR_H */
