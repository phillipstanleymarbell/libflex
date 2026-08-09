#
#       NOTE: For now, to build for Android, use
#
#                make OSTYPE=android MACHTYPE=x86_64 COMPILERVARIANT= WFLAGS=
#
include		config.local
COMPILERVARIANT = gcc	#clang
include		$(CONFIGPATH)/config.$(OSTYPE)-$(MACHTYPE).$(COMPILERVARIANT)

TARGET		= libflex-$(OSTYPE).a

#CC		= llvm-gcc #clang					#llvm-gcc
#LD		= llvm-gcc #clang					#llvm-gcc
#AR		= ar #llvm-ar				#ar
LINT		= /usr/local/bin/splint

INCDIRS		= -I.
OPTFLAGS	=
DBGFLAGS	=
WFLAGS		= -Wall #-Werror
CFLAGS		= $(PLATFORM_DBGFLAGS) $(PLATFORM_CFLAGS) $(PLATFORM_DFLAGS) $(PLATFORM_OPTFLAGS) -DFLEX64 $(INCDIRS)#-emit-llvm -DFLEX64 $(INCDIRS)	#-DFLEX64 $(INCDIRS)

LIBOBJS	=\
	flex-$(OSTYPE).$(OBJECTEXTENSION)\
	flex-tokenStream.$(OBJECTEXTENSION)\
	flex-memoryAllocation.$(OBJECTEXTENSION)\

HEADERS	=\
	flex-error.h\
	flex.h\

all:	libflex-$(TARGET).a

libflex-$(TARGET).a: Makefile $(LIBOBJS)
	$(AR) rv $(TARGET) $(LIBOBJS)

%.$(OBJECTEXTENSION): %.c $(HEADERS) Makefile
#	$(LINT) $(INCDIRS) $<
	$(CC) $(CFLAGS) $(WFLAGS) $(DBGFLAGS) $(OPTFLAGS) -c $<

flexversion.h: $(HEADERS) Makefile
	echo 'char FLEX_VERSION[] = "0.0-alpha (build '`date '+%m-%d-%Y-%H:%M:%S'`-`whoami`@`hostname`-`uname`\)\"\; > flexversion.h

clean:
	rm -f $(TARGET) $(LIBOBJS)
