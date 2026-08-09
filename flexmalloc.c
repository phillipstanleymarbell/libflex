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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "flextypes.h"
#include "flexerror.h"
#include "flex.h"

static flexinline void		allocaccounting(FlexErrState *, FlexMstate *, FlexPrintBuf *, FlexAddr addr, char *id);
static flexinline void		reallocaccounting(FlexErrState *, FlexMstate *, FlexPrintBuf *, FlexAddr addr, void *oldaddr, char *id);


flexinline void
allocaccounting(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexAddr addr, char *id)
{
	int	i;


	if (addr == 0)
	{
		flexprint(E, M, P, "%s \"%s\".\n", Emallocfor, id);
		flexmblocksdisplay(E, M, P);

		return;
	}

	if (M->nmemblocks == kFlexMaxAllocationBlocks)
	{
		flexprint(E, M, P, "%s.\n", Ememblocks);
		flexmblocksdisplay(E, M, P);

		return;
	}

	for (i = 0; i < M->nmemblocks; i++)
	{
		if (!strncmp(M->memblocks[i].id, id, kFlexMaxIdentifierStringLength))
		{
			int	nentries = M->memblocks[i].allocs - M->memblocks[i].frees;

			if (nentries >= kFlexMaxAllocations)
			{
				flexprint(E, M, P, "%s.\n", Ememblockaddrs);
				flexmblocksdisplay(E, M, P);

				return;
			}

			M->memblocks[i].addrs[nentries] = addr;
			M->memblocks[i].allocs++;
				
			return;
		}
	}

	strncpy(M->memblocks[i].id, id, kFlexMaxIdentifierStringLength);

	if (!M->memblocks[i].mallocd)
	{
		M->memblocks[i].addrs = (FlexAddr *)calloc(kFlexMaxAllocations, sizeof(FlexAddr));
		if (M->memblocks[i].addrs == NULL)
		{
			flexprint(E, M, P, "%s.\n", Emallocaddrs);

			return;
		}

		M->memblocks[i].mallocd = 1;
	}

	M->memblocks[i].addrs[0] = addr;
	M->memblocks[i].allocs = 1;
	M->memblocks[i].frees = 0;
	M->memblocks[i].reallocs = 0;
	M->memblocks[i].valid = 1;
	M->nmemblocks++;


	return;
}

flexinline void
reallocaccounting(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexAddr addr, void *oldaddr, char *id)
{
	int	i;


	if (addr == 0)
	{
		flexprint(E, M, P, "%s \"%s\".\n", Ereallocfor, id);
		flexmblocksdisplay(E, M, P);

		return;
	}

	for (i = 0; i < M->nmemblocks; i++)
	{
		int	nentries = M->memblocks[i].allocs - M->memblocks[i].frees;

		if (!strncmp(M->memblocks[i].id, id, kFlexMaxIdentifierStringLength))
		{
			M->memblocks[i].addrs[nentries] = addr;
			M->memblocks[i].reallocs++;
				
			return;
		}
	}

	if (!M->memblocks[i].valid)
	{
		flexprint(E, M, P, "%s, id = [%s].\n", Eunallocrealloc, id);
		flexmblocksdisplay(E, M, P);

		return;
	}

	return;
}

void*
flexmalloc(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int size, char *id)
{
	FlexAddr addr = (FlexAddr)malloc(size);
	if (M->debug)
	{
		allocaccounting(E, M, P, addr, id);
	}

	return (void *)addr;
}

void*
flexcalloc(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, int nelem, int size, char *id)
{
	FlexAddr addr = (FlexAddr)calloc(nelem, size);
	if (M->debug)
	{
		allocaccounting(E, M, P, addr, id);
	}

	return (void *)addr;
}

void*
flexrealloc(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, void *oldptr, int size, char *id)
{
	FlexAddr addr = (FlexAddr)realloc(oldptr, size);
	if (M->debug)
	{
		reallocaccounting(E, M, P, addr, oldptr, id);
	}

	return (void *)addr;
}

//TODO: add a flexincref() and flexdecref(), which we can use to mark libflex allocated objects. We can then do
//garbage collection at least be reference counting, or implement other algorithms.

void
flexfree(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, void *ptr, char *id)
{
	int	i, j;


	if (!M->debug)
	{
		free(ptr);

		return;
	}

	for (i = 0; i < M->nmemblocks; i++)
	{
		int	nentries = M->memblocks[i].allocs - M->memblocks[i].frees;

		for (j = 0; j < nentries; j++)
		{
			if (M->memblocks[i].addrs[j] == (FlexAddr)ptr)
			{
				if (nentries == 1)
				{
					memmove(&M->memblocks[i], &M->memblocks[M->nmemblocks - 1],
						sizeof(FlexMblock));
					M->memblocks[M->nmemblocks - 1].valid = 0;
					M->nmemblocks--;
				}
				else
				{
					M->memblocks[i].addrs[j] = M->memblocks[i].addrs[nentries - 1];
					M->memblocks[i].addrs[nentries - 1] = 0;
					M->memblocks[i].frees++;
				}
				free(ptr);

				return;
			}
		}
	}

	if (P != NULL)
	{
//BUG: this should be appending to end of errstr like we do in flexbufalloc, and incrementing errlen
		snprintf(E->errstr, kFlexErrorStringLength, "%s", Ebadfree);
	}
	else
	{
		flexprint(E, M, P, "flexmalloc.c/flexfree(): %s (%s)\n", Ebadfree, id);
	}

	
	return;
}

void
flexmblocksdisplay(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P)
{
	int	i;

	flexprint(E, M, P, "\nMonitored memory allocation statistics (a:allocs, f:frees, r:reallocs):\n\n");
	for (i = 0; i < kFlexMaxAllocationBlocks; i++)
	{
		if (!M->memblocks[i].valid)
		{
			break;
		}

		flexprint(E, M, P, "Block \"%-45s\": %3d a%-2s %3d f%-2s %2d r%-2s\n",
			M->memblocks[i].id, M->memblocks[i].allocs,
			(M->memblocks[i].allocs == 1 ? "," : "s,"),

			M->memblocks[i].frees,
			(M->memblocks[i].frees == 1 ? "" : "s"),

			M->memblocks[i].reallocs,
			(M->memblocks[i].reallocs == 1 ? "" : "s"));
	}
	flexprint(E, M, P, "\n");

	return;
}
