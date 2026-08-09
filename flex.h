/*
	Copyright (c) 1999-2011, Phillip Stanley-Marbell (author)
 
	All rights reserved.

	Redistribution and use in source and binary forms, with or without 
	modification, are permitted provided that the following conditions
	are met:

	*	Redistributions of source code must retain the above
		copyright notice, this list of conditions and the following
		disclaimer.

	*	Redistributions in binary form must reproduce the above
		copyright notice, this list of conditions and the following
		disclaimer in the documentation and/or other materials
		provided with the distribution.

	*	Neither the name of the author nor the names of its
		contributors may be used to endorse or promote products
		derived from this software without specific prior written 
		permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
	FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
	COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
	ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
	POSSIBILITY OF SUCH DAMAGE.
*/

enum
{
	kFlexErrorStringLength		= 128
};

#ifdef INFERNO
#	define	FlexLock	QLock
#else
#	define	FlexLock 	char
#endif

#ifdef INFERNO
#	define FLEX_FLOATFMT	"%f"
#	define FLEX_DBLFMT	"%E"
#	define FLEX_INTFMT	"%d"
#	define FLEX_UINTFMT	"%ud"
#	define FLEX_UVLONGFMT	"%llud"
#	define FLEX_ULONGFMT	"%lud"
#	define FLEX_LONGFMT	"%ld"
#	define FLEX_UHLONGFMT	"%lux"
#	define FLEX_UHCLONGFMT	"%luX"
#	define FLEX_UVHCLONGFMT	"%lluX"
#	define FLEX_UH8LONGFMT	"%08lux"
#	define FLEX_UH2XFMT	"%02X"
#	define FLEX_STRINGFMT	"%s"
#	define FLEX_INTEGERFMT	FLEX_INTFMT
#	define FLEX_REALFMT	FLEX_FLOATFMT
#else
#	define FLEX_FLOATFMT	"%f"
#	define FLEX_DBLFMT	"%E"
#	define FLEX_INTFMT	"%d"
#	define FLEX_UINTFMT	"%u"
#	define FLEX_UVLONGFMT	"%llu"
#	define FLEX_ULONGFMT	"%lu"
#	define FLEX_LONGFMT	"%ld"
#	define FLEX_UHLONGFMT	"%lx"
#	define FLEX_UHCLONGFMT	"%lX"
#	define FLEX_UVHCLONGFMT	"%llX"
#	define FLEX_UH8LONGFMT	"%08lx"
#	define FLEX_UH2XFMT	"%02X"
#	define FLEX_STRINGFMT	"%s"
#	define FLEX_INTEGERFMT	FLEX_INTFMT
#	define FLEX_REALFMT	FLEX_FLOATFMT
#endif


#if defined FLEX32
#	define FLEX_PTRFMTH	FLEX_UHCLONGFMT
#	define FLEX_PTRFMTD	FLEX_ULONGFMT
#elif defined FLEX64
#	define FLEX_PTRFMTH	FLEX_UVHCLONGFMT
#	define FLEX_PTRFMTD	FLEX_UVLONGFMT
#else
#	error "You must define one of FLEX64/FLEX32/FLEX16/FLEX8"
#endif


#define	USED(x)		(void)(x)

/*      WIN32 compiler doesn't always know 'inline', and already has min/max */
#ifdef  _WIN32_WINNT
#	define  flexinline
#else
#	define  flexinline            inline
#	define  max(x,y)        ((x) > (y) ? (x) : (y))
#	define  min(x,y)        ((x) < (y) ? (x) : (y))
#endif

typedef struct
{
	char 		*data;
//obsolete... (delete after sflr migration to use flexlib (might be used there)	long		value;
	int		quoted;
	
	int		linenum;
	int		colnum;
	
	void		*next;
	void		*prev;
} Datum;

typedef struct
{
	Datum		*masthead;
	Datum 		*head;
	Datum 		*tail;

	FlexLock	lock;
} Input;

typedef struct
{
	Datum 		*head;
	Datum 		*tail;

	FlexLock	lock;
} Labels;

/*									*/
/*		Flexp: flexible/buffering print routines		*/
/*									*/
enum
{
	kFlexCircularBufferSize	= 65536*1024,
};

//obsolete	typedef enum
//obsolete	{
//obsolete		Flexstdout	= 1<<0,		/*	Print to stdout		*/
//obsolete		Flexstderr	= 1<<1,		/*	Print to stderr		*/
//obsolete		Flexbuffer	= 1<<2,		/*	Buffer (don't print)	*/
//obsolete		Flexfd		= 1<<3,		/*	Print to descriptor	*/
//obsolete	} FlexPrintDest;

typedef struct
{
	int		silent;		/*   Flag to elide all action	*/

	char		*circbuf;	/*   String circular buffer	*/
	int		size;		/*   Allocated size of buffer	*/
	int		h2o;		/*   Fill level of circ buf	*/

	int		fd;		/*   For output to files, fd	*/
	int		off;		/*   Likewise, file offset	*/


//TODO: this has been factored out into the FlexErrState structure. Update flexMalloc.c and flextoken.c which is
//where it is used
//	/*	Called routines set or clear (i.e., [0]='\0') this:	*/
//	char		errstr[kFlexErrorStringLength];

	FlexLock	lock;
} FlexPrintBuf;

typedef struct
{
	FlexPrintBuf	*fbs;
	int		nfbs;

	FlexLock	lock;
} FlexPrint;


/*									*/
/*	This structure is passed in to all FlexLib functions, having	*/
/*	previously being initialized. The callee sets the errlen	*/
/*	upon an error, filling errstr with the selfsame number of	*/
/*	chars (including terminating NUL)				*/
/*									*/
typedef struct
{
	char		errstr[kFlexErrorStringLength];

	/*	Length of the current error	*/
	int		errlen;
} FlexErrState;


/*									*/
/*		Flexm:	flexible/monitoring allocator 			*/
/*									*/
enum
{
	kFlexMaxIdentifierStringLength	= 64,
	kFlexMaxAllocationBlocks	= 64,
	kFlexMaxAllocations		= 131072,
};

typedef struct
{
	char		id[kFlexMaxIdentifierStringLength];
	uvlong		*addrs;
	int		valid;
	int		allocs;
	int		frees;
	int		reallocs;
	int		mallocd;

	FlexLock	lock;
} FlexMblock;

/*									*/
/*	This structure contains all the state needed by the FlexM	*/
/*	routines.							*/
/*									*/
typedef struct
{
	FlexMblock	memblocks[kFlexMaxAllocationBlocks];
	int		nmemblocks;

	/*	Enable alloc accounting:	*/
	int		debug;

	char		lock;
} FlexMstate;

typedef struct
{
	int		scanning;
	Input		istream;
	Labels		labellist;

	FlexLock	lock;
} FlexIstream;

//TODO: whereever these are used, e.g. in fn call args, check use type FlexFileMode, not int
typedef enum
{
	kFlexFileModeTruncate	= 1 << 0,
	kFlexFileModeRead	= 1 << 1,
	kFlexFileModeWrite	= 1 << 2,
} FlexFileMode;

/*
TODO: april 2011: we should be able to replace uses of all three of the following
with GenericListItem which we already use in some places

typedef struct RealListItem RealListItem;
struct RealListItem
{
	real			value;
	RealListItem		*next;
};
typedef struct
{
	RealListItem		*hd;
	RealListItem		*tl;
	int			len;
} RealList;


typedef struct IntListItem IntListItem;
struct IntListItem
{
	int			value;
	IntListItem		*next;
};
typedef struct
{
	IntListItem		*hd;
	IntListItem		*tl;
	int			len;
} IntList;


typedef struct StringListItem StringListItem;
struct StringListItem
{
	char			*value;
	StringListItem		*next;
};
typedef struct
{
	StringListItem		*hd;
	StringListItem		*tl;
	int			len;
} StringList;
*/

/*
TODO: april 2011: remove these pre-determined GenericTypes, and leave it
	up to the user of flexlib to define their own element types
typedef enum
{
	GenericInteger,
	GenericReal,
	GenericString,
} GenericType;
*/

//TODO: rename this to FlexListItem, FlexList, FlexTupleListItem, and FlexTupleList.
//		We currently use GenericList and TupleList in salsvm implementation and in Noisy
//
typedef struct GenericListItem GenericListItem;
struct GenericListItem
{
	//GenericType		type;
	int			type;
	void			*value;
	GenericListItem		*next;
};
typedef struct
{
	GenericListItem		*hd;
	GenericListItem		*tl;
	int			len;
} GenericList;

typedef struct TupleListItem TupleListItem;
struct TupleListItem
{
	int			cardinality;
	GenericList		*siblings;
	TupleListItem		*next;
};
typedef struct
{
	TupleListItem		*hd;
	TupleListItem		*tl;
	int			len;
} TupleList;


//TODO/BUG Until we actually rename structures, introduce aliases:
typedef GenericListItem		FlexListItem;
typedef GenericList		FlexList;
typedef TupleListItem		FlexTupleListItem;
typedef TupleList		FlexTupleList;



/*									*/
/*	NOTE: the Flexm routines use the Flexp routines for printing.	*/
/*	The Flexp (and thus the Flexm) routines will default to 	*/
/*	printing stdout if a NULL FlexPrintBuf is supplied.		*/
/*									*/
void*	flexMalloc		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int size, char *id);
void*	flexCalloc		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int nelem, int size, char *id);
void*	flexRealloc		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, void *oldptr, int size, char *id);
void	flexFree		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, void *ptr, char *id);
void	flexMemoryBlocksDisplay	(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P);
void	flexincref		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, void *ptr);
void	flexdecref		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, void *ptr);
void	flexStreamMunch		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexIstream *S,
					const char *sepchars, const char *stickies, const char *buf, int *curline, int *curcol);
void	flexStreamClear		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexIstream *I);
void	flexStreamScan		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexIstream *I);
void	flexStreamCheck		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexIstream *I, int maxtokens, int fmtchars);
uvlong	flexFileSize		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd);
char*	flexFgets		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *buf, int len, int fd);
int	flexCreate		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *path, int omode);
int	flexOpen		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *path, int omode);
int	flexRead		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd, char* buf, int len);
int	flexClose		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd);
FlexPrintBuf*	flexBufferAllocate	(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int circbufsz, char *filename, FlexFileMode mode);
void	flexBufferDeallocate		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexPrintBuf *moribund);
void	flexBufferReset		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P);
int	flexWrite		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd, char* buf, int len);
int	flexChangeDirectory		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *path);
char*	flexGetWorkingDirectory		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P);
void	flexPrint		(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, const char *fmt, ...);

/*									*/
/*	Simple routines that do not need error and print buffers.	*/
/*									*/
void	flexlock		(FlexLock *l);
void	flexunlock		(FlexLock *l);
int	flexSnprint		(char *dst, int size, const char *fmt, ...);
void	flexNanosleep		(ulong nsecs);
ulong	flexUserCpuTimeMicroseconds	(void);
ulong	flexCpuTimeMicroseconds	(void);
ulong	flexWallClockMicroseconds	(void);
ulong	flextimeusecs		(void);
