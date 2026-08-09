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

#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include "flextypes.h"
#include "flex.h"

static flexinline void		checkh2o(int maxbufsz, FlexPrintBuf *P, char *buf);

flexinline void
checkh2o(int maxbufsz, FlexPrintBuf *P, char *buf)
{
	int	ndelete, newidx, n = strlen(buf) + 1;


	/*	Make sure NUL terminated string can fit in buffer	*/
	if (n > maxbufsz)
	{
		n = maxbufsz;
	}
	
	ndelete = P->h2o - (maxbufsz - n);
	if (ndelete < 0)
	{
		ndelete = 0;
	}

	/*	Move old stuff in buffer backwards, discarding some	*/
	memmove(&P->circbuf[0], &P->circbuf[ndelete], P->h2o - ndelete);

	newidx = max(0, P->h2o - ndelete - 1);

	/*	Put new stuff in, overwriting terminating NUL in old	*/
	memmove(&P->circbuf[newidx], buf, n);

	P->h2o = newidx + n;
	P->circbuf[P->h2o] = '\0';


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

void
flexlock(FlexLock *l)
{
}

void
flexunlock(FlexLock *l)
{
}

char *
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
		n = read(fd, &ch, 1);
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

int
flexcreate(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *path, int mode)
{
	int	perm = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP;
	int	flags = 0;
	int	rw = mode & (~kFlexFileModeTruncate);
	int	fd;

	if (rw == kFlexFileModeRead)
		flags = O_RDONLY;
	
	if (rw == kFlexFileModeWrite)
		flags = O_WRONLY;

	if (rw == (kFlexFileModeRead|kFlexFileModeWrite))
		flags = O_RDWR;

	if (mode & kFlexFileModeTruncate)
		flags |= O_TRUNC;
	else
		flags |= O_APPEND;

	fd = open(path, flags|O_CREAT, perm);
	return fd;
}

int
flexopen(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *path, int mode)
{
	int	perm = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP;
	int	flags = 0;
	int	rw = mode & (~kFlexFileModeTruncate);


	if (rw == kFlexFileModeRead)
		flags = O_RDONLY;
	
	if (rw == kFlexFileModeWrite)
		flags = O_WRONLY;

	if (rw == (kFlexFileModeRead|kFlexFileModeWrite))
		flags = O_RDWR;

	if (mode & kFlexFileModeTruncate)
		flags |= O_TRUNC;
	else
		flags |= O_APPEND;

	return open(path, flags, perm);
}

int
flexread(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd, char* buf, int len)
{
	return read(fd, buf, len);
}

int
flexclose(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd)
{
	return close(fd);
}

int
flexwrite(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int fd, char* buf, int len)
{
	return write(fd, buf, len);
}

int
flexchdir(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, char *path)
{
	return chdir(path);
}

char *
flexgetpwd(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, )
{
	/*								*/
	/*	This is non-POSIX, but...				*/
	/*	On Darwin, let getcwd() auto-allocate space for buf	*/
	/*	The buffer will be freed by caller, e.g., sf.y.		*/
	/*								*/
	return getcwd(NULL, 0);
}

int
flexsnprint(char *dst, int size, char *fmt, ...)
{
	va_list		arg;
	int		n;

	va_start(arg, fmt);
	n = vsnprintf(dst, size, fmt, arg);
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
	char	*buf;
	va_list	arg;


	if (P != NULL && P->silent)
	{
		return;
	}

	/*								*/
	/*	If failing due to memory, further prints go to stderr	*/
	/*								*/
	buf = flexcalloc(M, NULL, 1, kFlexCircularBufferSize, "flex-FreeBSD.c:flexprint/buf");
	if (buf == NULL)
	{
		fprintf(stderr, "Could not allocate memory for (char *)buf in flex-darwin.c/flexprint.\n");

		return;
	}

	va_start(arg, fmt);
	fmtlen = vsnprintf(buf, kFlexCircularBufferSize, fmt, arg);
	va_end(arg);
 
	if (fmtlen < 0)
	{
		fprintf(stderr, "vsnprintf() in flexprint() failed.\n");
		flexfree(M, NULL, buf, "flex-FreeBSD.c:flexprint/buf");

		return;
	}

	/*	If no buffer structure is given, send to global output	*/
	if (P == NULL)
	{
		fprintf(stdout, "%s", buf);
	}
	else if (P != NULL && P->circbuf == NULL)
	{
		write(P->fd, buf, strlen(buf));
TODO: (also in other archs: if the write failed, append message to E->errstr)
	}
	else
	{
		checkh2o(kFlexCircularBufferSize, P, buf);
	}

	flexfree(M, NULL, buf, "flex-FreeBSD.c:flexprint/buf");


	return;
}

void
flexnsleep(ulong nsecs)
{
	/*								*/
	/*	Inferno doesn't provide us with enough granularity.	*/
	/*	This should still be fairly portable since nanosleep	*/
	/*	is POSIX.1b.						*/
	/*								*/
	struct timespec rqtp;
	
	rqtp.tv_sec = nsecs/1000000000;
	rqtp.tv_nsec = nsecs % 1000000000;

	nanosleep(&rqtp, NULL);
}

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
