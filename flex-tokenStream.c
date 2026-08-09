/*
 *	Copyright (c) 1999-2011, Phillip Stanley-Marbell (author)
 *
 *	All rights reserved.
 *
 *	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions
 *	are met:
 *
 *	*	Redistributions of source code must retain the above
 *		copyright notice, this list of conditions and the following
 *		disclaimer.
 *
 *	*	Redistributions in binary form must reproduce the above
 *		copyright notice, this list of conditions and the following
 *		disclaimer in the documentation and/or other materials
 *		provided with the distribution.
 *
 *	*	Neither the name of the author nor the names of its
 *		contributors may be used to endorse or promote products
 *		derived from this software without specific prior written
 *		permission.
 *
 *	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *	FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *	COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *	ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *	POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>
#include <stdio.h>
#include "flex-types.h"
#include "flex.h"

/*	TODO: need to rewrite the token stream handling; this is a disgrace.	*/


static int	issepchar(const char *  sepchars, char c);
static int	issticky(const char *  stickies, char c);


int
issepchar(const char *  sepchars, char c)
{
	return (strchr(sepchars, c) == NULL ? 0 : 1);
}

int
issticky(const char *  stickies, char c)
{
	return (strchr(stickies, c) == NULL ? 0 : 1);
}

void
flexStreamMunch(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexIstream *S, const char *sepchars, const char *stickies, const char *buf, int *curline, int *curcol)
{
	int 	eaten = 0;
	Input	*I = &S->istream;

	if (strlen(buf) > 0)
	{
              	while (eaten < strlen(buf))
               	{
			char	*tptr;
			Datum	*t1;


			t1 = (Datum *) flexMalloc(E, M, P, sizeof(Datum), "flex-tokenStream.c:flexstreammunch/t1");
			if (t1 == NULL)
			{
				/*	BUG: this should be appending to end of errstr like we do in flexBufferAllocate, and incrementing errlen	*/
				snprintf(E->errstr, kFlexErrorStringLength, "Could not allocate memory for Datum *t1");

				return;
			}

			t1->next = NULL;
			t1->prev = NULL;
			//obsolete. Delete if not used in sflr after merge:	t1->value = 0;
			t1->quoted = 0;

			t1->data = (char*) flexMalloc(E, M, P, (strlen(buf)+1)*sizeof(char),
						"flex-tokenStream.c:flexstreammunch/t1->data");
			if (t1->data == NULL)
			{
				flexFree(E, M, P, t1, "flex-tokenStream.c:flexstreammunch/t1");

				/*	BUG: this should be appending to end of errstr like we do in flexBufferAllocate, and incrementing errlen	*/
				snprintf(E->errstr, kFlexErrorStringLength, "Could not allocate memory for char *t1->data");

				return;
			}
			tptr = t1->data;

			/* 	Throw away all chars till we see a non-sepchar 	*/
			while (issepchar(sepchars, buf[eaten]) && (eaten < strlen(buf)))
			{
				eaten++;
			}

			/*
			 *	I refer to tokens such as '(' etc. as "sticky":
			 *	they may be "stuck" onto another token, and are
			 *	NOT separators : we have to allocate a list entry
			 *	for them. 	So, get one sticky char:
			 */
			if (issticky(stickies, buf[eaten]))
			{
				*tptr++ = buf[eaten++];
			}
			else while(!issepchar(sepchars, buf[eaten]) &&
					!issticky(stickies, buf[eaten]) &&
					(eaten < strlen(buf))
				)
			{
				/*
				 *	Get all non sepchars into t1->data
				 *	If we see a quoted string, "*" (_not_
				 *	'*'), gobble it including any sepchars,
				 *	till end of input (we get line @a time)
				 *	or matching quote.
				 */
				if (buf[eaten] == '"')
				{
					/*
					 *	Mark as "quoted", thus even though
					 *	item in data may be "2", its may be
					 *	as a string, not an int by users
					 */
					t1->quoted = 1;

					/*		Discard opening quote:		*/
					eaten++;

					/*	Must make not to go past newline:	*/
					while ((buf[eaten] != '"') &&
						(buf[eaten] != '\n') &&
						(eaten < strlen(buf)))
					{
						*tptr++ = buf[eaten++];
					}

					/*	Discard trailing quote if its there:	*/
					if (eaten < strlen(buf) && (buf[eaten] == '"'))
					{
						eaten++;
					}
				}
				else
				{
					*tptr++ = buf[eaten++];
				}
			}

			*tptr = '\0';

			/*	If we actually ate any food, put it in our stomach	*/
			if (tptr != t1->data)
			{
				t1->linenum = *curline;
				t1->colnum = *curcol;

				if (!strncmp(t1->data, "\n", 1))
				{
					*curline = *curline + 1;
					*curcol = 1;
				}
				else
				{
					*curcol = *curcol + 1;
				}

				if ((I->head == NULL)  || (I->tail == NULL))
				{
					I->tail = I->head = I->masthead = t1;

					/*
					 *	NOTE tail and head now point to the lone datum
					 *	and they _both_ have null pre- and next-.
					 */
					I->masthead->prev = NULL;
					I->masthead->next = NULL;

					I->head->prev = NULL;
					I->head->next = NULL;

					I->tail->prev = NULL;
					I->tail->next = NULL;
				}
				else
				{
					/*
					 *	Add new datum to _tail_ of list. MUST keep it FIFO
					 *	for the asm to be parsed correctly.
					 */
					t1->next = I->tail;
					I->tail->prev = t1;
					I->tail = t1;
				}
			}
			else
			{
				flexFree(E, M, P, t1->data, "flex-tokenStream.c:flexstreammunch/t1->data");
				flexFree(E, M, P, t1, "flex-tokenStream.c:flexstreammunch/t1");
			}

			/*
			 *	Items on the FlexIstream must be deallocated by our caller
			 */
		}
	}


	return;
}

void
flexStreamClear(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexIstream *I)
{
	Datum	*p, *q;

	p = I->istream.head;
	while (p != NULL)
	{
		q = p;
		flexFree(E, M, P, p->data, "flex-tokenStream.c:flexstreamclear/p->data");
		p = p->prev;
		flexFree(E, M, P, q, "flex-tokenStream.c:flexstreamclear/q");
	}
	I->istream.tail = I->istream.head = I->istream.masthead = NULL;

	return;
}

void
flexStreamScan(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexIstream *I)
{
	Datum	*tmpistream = I->istream.tail;


	while (tmpistream != NULL)
	{
		/* 	If it is new a label, add it to labellist 		*/
		if (tmpistream->data[strlen(tmpistream->data)-1] == ':')
		{
			Datum*	tmplabel = (Datum *) flexMalloc(E, M, P, sizeof(Datum),
							"flex-tokenStream.c:flexstreamscan/tmplabel");
			if (tmplabel == NULL)
			{
				/*	BUG: this should be appending to end of errstr like we do in flexBufferAllocate, and incrementing errlen	*/
				snprintf(E->errstr, kFlexErrorStringLength,
					"Could not allocate memory for flex-tokenStream.c:flexstreamscan/tmplabel");

				return;
			}

			tmplabel->next = NULL;
			tmplabel->prev = NULL;
			tmplabel->data = (char*) flexMalloc(E, M, P, strlen(tmpistream->data)*sizeof(char),
							"flex-tokenStream.c:flexstreamscan/tmplabel->data");
			if (tmplabel->data == NULL)
			{
				flexFree(E, M, P, tmplabel, "flex-tokenStream.c:flexstreamscan/tmplabel");

				/*	BUG: this should be appending to end of errstr like we do in flexBufferAllocate, and incrementing errlen	*/
				snprintf(E->errstr, kFlexErrorStringLength, "Could not allocate memory for char *tmplabel->data, svm-consoleMain.c");

				return;
			}

			strncpy(tmplabel->data, tmpistream->data, strlen(tmpistream->data) - 1);
			tmplabel->data[strlen(tmpistream->data)-1] = '\0';

			if ((I->labellist.head == NULL)  || (I->labellist.tail == NULL))
			{
				I->labellist.tail = I->labellist.head = tmplabel;

				I->labellist.head->prev = NULL;
				I->labellist.head->next = NULL;

				I->labellist.tail->prev = NULL;
				I->labellist.tail->next = NULL;
			}
			else
			{
				/*
				 *	Add new datum to _tail_ of list.
				 */
				tmplabel->next = I->labellist.tail;
				I->labellist.tail->prev = tmplabel;
				I->labellist.tail = tmplabel;
			}
		}

		/*	If it is a global definition (".comm"), add it to labellist	*/
		if (!strcmp(tmpistream->data, ".comm"))
		{
			Datum* tmplabel = (Datum *) flexMalloc(E, M, P, sizeof(Datum),
						"flex-tokenStream.c:flexstreamscan/tmplabel");
			if (tmplabel == NULL)
			{
				/*	BUG: this should be appending to end of errstr like we do in flexBufferAllocate, and incrementing errlen	*/
				snprintf(E->errstr, kFlexErrorStringLength,
					"Could not allocate memory for flex-tokenStream.c:flexstreamscan/tmplabel");

				return;
			}

			/*	Token we just passed is global var name		*/
			tmpistream = tmpistream->prev;
			if (tmpistream == NULL)
			{
				/*	BUG: this should be appending to end of errstr like we do in flexBufferAllocate, and incrementing errlen	*/
				snprintf(E->errstr, kFlexErrorStringLength,
					"Badly formed input stream: \".comm\" without a var name");

				return;
			}

			tmplabel->next = NULL;
			tmplabel->prev = NULL;

			tmplabel->data = (char*) flexMalloc(E, M, P, strlen(tmpistream->data)*sizeof(char),
					"flex-tokenStream.c:flexstreamscan/tmplabel->data");
			if (tmplabel->data == NULL)
			{
				/*	BUG: this should be appending to end of errstr like we do in flexBufferAllocate, and incrementing errlen	*/
				snprintf(E->errstr, kFlexErrorStringLength,
					"Could not allocate memory for flex-tokenStream.c:flexstreamscan/tmplabel->data");

				return;
			}

			strncpy(tmplabel->data, tmpistream->data, strlen(tmpistream->data));

			/*	We went one step back. Step forward again	*/
			tmpistream = tmpistream->next;

			if ((I->labellist.head == NULL)  || (I->labellist.tail == NULL))
			{
				I->labellist.tail = I->labellist.head = tmplabel;

				I->labellist.head->prev = NULL;
				I->labellist.head->next = NULL;

				I->labellist.tail->prev = NULL;
				I->labellist.tail->next = NULL;
			}
			else
			{
				/*
				 *	Add new datum to _tail_ of list.
				 */
				tmplabel->next = I->labellist.tail;
				I->labellist.tail->prev = tmplabel;
				I->labellist.tail = tmplabel;
			}
		}
		tmpistream = tmpistream->next;
	}

	/*	We screwed up istream.head, so reset it :	*/
	I->istream.head = I->istream.masthead;

	return;
}

void
flexStreamCheck(FlexErrState *E, FlexMstate *M, FlexPrintBuf *P, FlexIstream *I, int maxtokens, int fmtchars)
{
	int	tripchars = 0, done = 0;
	Datum	*tmp = I->istream.head;


	if (tmp != NULL && tmp->data != NULL)
	{
		flexPrint(E, M, P, "\tline %5d, token %3d\t", tmp->linenum, tmp->colnum);
	}

	while (tmp != NULL)
	{
		if (maxtokens > 0 && (done++ > maxtokens))
		{
			flexPrint(E, M, P, "...");
			break;
		}

		if (tmp->data != NULL && !strncmp(tmp->data, "\n", 1))
		{
			flexPrint(E, M, P, "(newline)");
			tripchars = 0;

			if (tmp->prev != NULL)
			{
				/*	If this is not end of stream, generate the \n\t for next line		*/
				flexPrint(E, M, P, "\n\tline %5d\t\t", tmp->linenum+1);
			}
		}
		else
		{
			flexPrint(E, M, P, "'%s%s%s' ", (tmp->quoted ? "\"" : ""), tmp->data, (tmp->quoted ? "\"" : ""));

			/*
			 *	Account for the output string and the
			 *	two guarding "'" quotes.
			 */
			tripchars += strlen(tmp->data) + 2;

			if (tripchars >= fmtchars)
			{
				tripchars = 0;
				flexPrint(E, M, P, "\n\t\t\t\t");
			}
		}

		tmp = tmp->prev;
	}
	flexPrint(E, M, P, "\n");

	return;
}
