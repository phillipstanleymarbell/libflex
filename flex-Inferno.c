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

#include <time.h>
#include <stdlib.h>
#include "lib9.h"
#include "mathi.h"
#include "kernel.h"
#include "fcall.h"
#include "flextypes.h"
#include "flex.h"

#ifdef linux
#	include <sys/types.h>
#	include <sys/mman.h>
#	include <sched.h>
#endif

extern void		pexit(char*, int);
extern vlong		osusectime(void);
extern int		osmillisleep(ulong);
extern int		devsfspawnscheduler(Engine *);
extern int		devsfkillscheduler(int);

static flexinline void	checkh2o(int maxbufsz, int *h2o, char *circbuf, char *buf);

flexinline void
checkh2o(int maxbufsz, int *h2o, char *circbuf, char *buf)
{
	int	ndelete, newidx, n = strlen(buf) + 1;


	/*	Make sure NUL terminated string can fit in buffer	*/
	if (n > maxbufsz)
	{
		n = maxbufsz;
	}
	
	ndelete = *h2o - (maxbufsz - n);
	if (ndelete < 0)
	{
		ndelete = 0;
	}

	/*	Move old stuff in buffer backwards, discarding some	*/
	memmove(&circbuf[0], &circbuf[ndelete], *h2o - ndelete);

	newidx = max(0, *h2o - ndelete - 1);

	/*	Put new stuff in, overwriting terminating NUL in old	*/
	memmove(&circbuf[newidx], buf, n);

	*h2o = newidx + n;
	circbuf[*h2o] = '\0';
	//fprint(2, "h2o= [%d], circbuf = [%s]\n", *h2o, circbuf);

	return;
}

int
flexfsize(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd)
{
	struct stat	sb;

	if (fstat(fd, &sb) < 0)
	{
		return -1;
	}

	return sb.st_size;
}

int
flexfsize(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd)
{
/*
	The kseek approach is much simpler, but I don't see
	what's buggered with this:
	
	char	buf[STATFIXLEN*4];
	Dir	d;

	kfstat(fd, &buf[0], STATFIXLEN*4);
	convM2D(&buf[0], STATFIXLEN*4, &d, nil);
	return d.length;
*/
	int	off, length;

	off = kseek(fd, 0, 1);
	length = kseek(fd, 0, 2);
	kseek(fd, off, 0);

	return length;
}

void
flexlock(FlexLock *l)
{
	qlock(l);

	return;
}

void
flexunlock(FlexLock *l)
{
	qunlock(l);

	return;
}

char*
flexfgets(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *buf, int len, int fd)
{
	int 	n, i = 0;
	char	ch;
	
	if (len <= 0)
	{
		return NULL;
	}

	do
	{
		n = kread(fd, &ch, 1);
		if (n == 0)
		{
			if (i == 0)
				return NULL;
			else
				break;
		}

		buf[i++] = ch;
	} while ((i < len) && (ch != '\n'));
	buf[i] = '\0';

	
	return buf;
}

/*
	TODO: there are many problems with our open/create management under
	inferno. Under slax linux liveCD, we cannot write to sunflower.out
	(we keep getting fd=0).
*/

TODO: i'm not happy with the way the flags are handled. they should be 1 << iota, and you
should be able to specify X|Y|Z

int
flexcreate(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *path, int omode)
{
	int	mode = 0;
	int	rw = omode & (~kFlexFileModeTruncate);
	int	fd;

	if (rw == kFlexFileModeRead)
		mode = OREAD;
	
	if (rw == kFlexFileModeWrite)
		mode = OWRITE;

	if (rw == (kFlexFileModeRead|kFlexFileModeWrite))
		mode = ORDWR;

	if (omode & kFlexFileModeTruncate)
		mode |= OTRUNC;


	fd = kcreate(path, mode, 0600);

	return fd;
}


int
flexopen(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *path, int omode)
{
	int	mode = 0;
	int	rw = omode & (~kFlexFileModeTruncate);


	if (rw == kFlexFileModeRead)
		mode = OREAD;
	if (rw == kFlexFileModeWrite)
		mode = OWRITE;
	if (rw == (kFlexFileModeRead|kFlexFileModeWrite))
		mode = ORDWR;
	if (mode & kFlexFileModeTruncate)
		mode |= OTRUNC;
	

	return kopen(path, mode);
}

int
flexread(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd, char* buf, int len)
{
	return kread(fd, buf, len);
}

int
flexclose(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd)
{
	return kclose(fd);
}

int
flexwrite(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd, char* buf, int len)
{
	return kwrite(fd, buf, len);
}

int
flexchdir(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *path)
{
	return kchdir(path);
}

char*
flexgetpwd(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, )
{
	int fd = kopen(".", OREAD);
	return kfd2path(fd);
}

int
flexsnprint(char *dst, int size, char *fmt, ...)
{
	va_list		arg;
	int		n;

	va_start(arg, fmt);
	n = vsnprint(dst, size, fmt, arg);
	va_end(arg);

	return n;
}

FlexPrintBuf*
flexbufalloc(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int circbufsz, char *filename, FlexFileMode mode)
{
	FlexPrintBuf	*tmpflexbuf;


	tmpflexbuf = (FlexPrintBuf *)flexcalloc(E, M, P, 1, sizeof(FlexPrintBuf),
					"FlexPrintBuf* in flex-darwin.c/flexbufalloc");
	if (tmpflexbuf == NULL)
	{
		if (E != NULL)
		{
			char	tmperr[kFlexErrorStringLength];

			snprintf(tmperr, kFlexErrorStringLength, " -> %s", Emalloc);
			strncat(E->errstr, tmperr, sizeof(E->errstr) - strlen(E->errstr) - 1);
			E->errlen = strlen(E->errstr);
		}

		return NULL;
	}

	if (circbufsz > 0)
	{
		tmpflexbuf->circbuf = (char *)flexcalloc(E, M, P, 1, circbufsz,
						"FlexPrintBuf->char* in flex-darwin.c/flexbufalloc");
		if (tmpflexbuf->circbuf == NULL)
		{
			if (E != NULL)
			{
				char	tmperr[kFlexErrorStringLength];

				snprintf(tmperr, kFlexErrorStringLength, " -> %s", Emalloc);
				strncat(E->errstr, tmperr, sizeof(E->errstr) - strlen(E->errstr) - 1);
				E->errlen = strlen(E->errstr);
			}

			return NULL;
		}
	}

	tmpflexbuf->fd = -1;
	if (filename != NULL)
	{
		tmpflexbuf->fd = flexcreate(E, M, P, filename, mode);
		if (tmpflexbuf->fd <= 2)
		{
			if (E != NULL)
			{
				char	tmperr[kFlexErrorStringLength];

				snprintf(tmperr, kFlexErrorStringLength, " -> %s", Emalloc);
				strncat(E->errstr, tmperr, sizeof(E->errstr) - strlen(E->errstr) - 1);
				E->errlen = strlen(E->errstr);
			}

			return NULL;
		}
	}


	return tmpflexbuf;
}

void
flexbufdealloc(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexPrintBuf *moribund)
{
	if (moribund == NULL)
	{
		return;
	}

	if (moribund->circbuf != NULL)
	{
		flexfree(E, M, P, moribund->circbuf, "flex-darwin.c/flexbufdealloc");
	}

	if (moribund->fd != -1)
	{
		flexclose(E, M, P, moribund->fd);
	}


	return;
}

void
flexprint(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *fmt, ...)
{
	int	fmtlen;
	char	*buf, *end;
	va_list	arg;


	if (P != NULL && P->silent)
	{
		return;
	}

	/*								*/
	/*	If failing due to memory, further prints go to stderr	*/
	/*								*/
	buf = flexcalloc(M, NULL, 1, kFlexCircularBufferSize, "flex-Inferno.c:flexprint/buf");
	if (buf == NULL)
	{
		fprint(2, "Could not allocate memory for (char *)buf in flex-darwin.c/flexprint.\n");

		return;
	}

	va_start(arg, fmt);
	end = vseprint(buf, buf+kFlexCircularBufferSize, fmt, arg);
	va_end(arg);
 
	if (end == NULL || end < buf || end > buf+kFlexCircularBufferSize)
	{
		fprint(t, "vseprint() in flexprint() failed.\n");
		flexfree(M, NULL, buf, "flex-Inferno.c:flexprint/buf");

		return;
	}

	/*	If no buffer structure is given, send to global output	*/
	if (P == NULL)
	{
		fprint(1, "%s", buf);
	}
	else if (P != NULL && P->circbuf == NULL)
	{
		kwrite(P->fd, buf, end - buf);

TODO: (also in other archs: if the write failed, append message to E->errstr)
	}
	else
	{
		checkh2o(kFlexCircularBufferSize, P, buf);
	}

	flexfree(M, NULL, buf, "flex-Inferno.c:flexprint/buf");


	return;
}

void
flexnsleep(ulong nsecs)
{
#ifdef	_WIN32_WINNT
	osmillisleep(nsecs/1000000);
#else
	/*								*/
	/*	Inferno 3.0 doesn't provide us with enough granularity.	*/
	/*	This should still be fairly portable since nanosleep	*/
	/*	is POSIX.1b ... yeah, right.				*/
	/*								*/
	struct timespec rqtp;
	
	rqtp.tv_sec = nsecs/1000000000;
	rqtp.tv_nsec = nsecs % 1000000000;

	nanosleep(&rqtp, nil);
#endif
}

#ifdef EMU
ulong
flexusercputimeusecs(void)
{
	struct rusage 	r;

	getrusage(RUSAGE_SELF, &r);

	return (ulong)(r.ru_utime.tv_sec*1E6 + r.ru_utime.tv_usec);
}


ulong
flexcputimeusecs(void)
{
	struct rusage 	r;

	getrusage(RUSAGE_SELF, &r);

	return (ulong)(r.ru_utime.tv_sec*1E6 + r.ru_utime.tv_usec +
		       r.ru_stime.tv_sec*1E6 + r.ru_stime.tv_usec);
}

ulong
flexwallclockusecs(void)
{
	struct timeval 	t;
	gettimeofday(&t, NULL);

	return t.tv_usec;
}
#else
ulong
flexusercputimeusecs(void)
{
	return osusectime();
}

ulong
flexcputimeusecs(void)
{
	return osusectime();
}

ulong
flexwallclockusecs(void)
{
	return osusectime();
}
#endif

ulong
flextimeusecs(void)
{
	return osusectime();
}









TODO: delete this entirely. replace all uses of mlog with flexprint, to a printbuf previously opened on the S->logfd
void
mlog(FlexMstate *M, FlexPrintBuf *P, char *fmt, ...)
{
	char	*buf;
	va_list	arg;

	if (!SF_SIMLOG)
	{
		return;
	}

	/*	Should never happen, but just in case code changes elsewhere:	*/
	if (S->logfd < 0)
	{
		return;
	}

	buf = mcalloc(E, 1, MAX_MIO_BUFSZ, "(char *)buf in arch-Inferno.c");
	if (buf == NULL)
	{
		mexit(E, "Could not allocate memory for (char *)buf in arch-Inferno.c", -1);
	}

	va_start(arg, fmt);
	vseprint(buf, buf+MAX_MIO_BUFSZ, fmt, arg);
	va_end(arg);

	kwrite(S->logfd, buf, strlen(buf));
	mfree(E, buf, "(char *)buf in arch-Inferno.c");


	return;
}

TODO: replace all uses of flexerror / merror with flexprint to an appropriate FlexPrintBuf...
void
flexerror(FlexMstate *M, FlexPrintBuf *P, char *fmt, ...)
{
	char	*buf;
	va_list	arg;


	buf = mcalloc(E, 1, MAX_MIO_BUFSZ, "(char *)buf in arch-Inferno.c");
	if (buf == NULL)
	{
		mexit(E, "Could not allocate memory for (char *)buf in arch-Inferno.c", -1);
	}

	va_start(arg, fmt);
	vseprint(buf, buf+MAX_MIO_BUFSZ, fmt, arg);
	va_end(arg);
 
	mprint(E, NULL, siminfo, "%s: %s\n", Eerrorstr, buf);
	mfree(E, buf, "(char *)buf in arch-Inferno.c");

	return;
}
