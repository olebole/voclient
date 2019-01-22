#///////////////////////////////////////////////////////////////////////////////
#//
#//
#///////////////////////////////////////////////////////////////////////////////

# primary dependencies

NAME     = xrpc
VERSION  = 1.0
HERE    := $(shell /bin/pwd)


# secondary dependencies

LIBBASE     = lib$(NAME)
STATICLIB   = $(HERE)/$(LIBBASE).a
SHAREDLIB   = $(HERE)/$(LIBBASE).so.$(VERSION)


# stuff that's precious to keep

.PRECIOUS:
.KEEP_STATE:


# includes, flags and libraries
CINCS   = -I./ -I../include -Ixmlrpc-c-1.25.23/lib/abyss/src
#CFLAGS  = -O2 -m32 -g $(CINCS) 
CFLAGS  = -m32 -g -Wall $(CINCS)


# list of source and include files

SRCS  = xrClient.c xrServer.c xrMethod.c xrUtil.c xrStruct.c xrArray.c
OBJS  = xrClient.o xrServer.o xrMethod.o xrUtil.o xrStruct.o xrArray.o
INCS  = xrpc.h xrpcP.h


LFLAGS 	= -L. -L./lib 
LIBS 	=


# targets

all:  xrpc

clean:
	(./mkclean)
	/bin/rm -rf Shared Static UnitTests/* *.o *.a *.e

install: xrpc
	(cp libxrpc.a ../lib)
	cp -rp $(INCS) ./include/* ../include

World:
everything:
	make clean
	make xrpc
	make base
	make install

include:

#
base:
	(./mklibs)


xrpc: $(SRCS:%.c=%.o)
	(./mklibs)
	cp $(OBJS) lib/build/
	/usr/bin/ar rv libxrpc.a $?
	/usr/bin/ar sv libxrpc.a $?
	(cp libxrpc.a ../lib)
	(ranlib ../lib/libxrpc.a)
	cp -rp $(INCS) ./include/* ../include

%.o: %.c $(INCS)
	/usr/bin/gcc -Wall $(CINCS) $(CFLAGS) -c $< -o $@


#=======================
# leave this stuff alone
#=======================

$(STATICLIB): $(SRCS:%.c=Static/%.o)
	/usr/bin/ar rv $@ $?
Static/%.o: %.c $(INCS)
	/usr/bin/gcc $(CINCS) $(CFLAGS) -c $< -o $@
Static:
	/bin/mkdir $@
	chmod 777 $@

$(SHAREDLIB): $(SRCS:%.c=Shared/%.o)
	/usr/bin/ld -shared -o $@ $? -lc -ldl
Shared/%.o: %.c $(INCS)
	/usr/bin/gcc $(CINCS) $(CFLAGS) -fpic -shared -c $< -o $@
Shared:
	/bin/mkdir $@
	chmod 777 $@
