#///////////////////////////////////////////////////////////////////////////////
#//
#//  Makefile for the VOTable Interface
#//
#///////////////////////////////////////////////////////////////////////////////

# primary dependencies

NAME       	 = VOClient
VERSION    	 = 1.0
PLATFORM  	:= $(shell uname -s)
PLMACH  	:= $(shell uname -m)
HERE      	:= $(shell /bin/pwd)

#BINDIR    	:= ../../bin/			# IRAF version
#LIBDIR    	:= ../../lib/
#INCDIR    	:= ../../lib/

BINDIR  	:= $(shell ./install_env --bindir)
LIBDIR  	:= $(shell ./install_env --libdir)
INCDIR  	:= $(shell ./install_env --incdir)
MANDIR  	:= $(shell ./install_env --mandir)



# includes, flags and libraries
CC 	  	= gcc
CINCS  	  	= -I$(INCDIR) -I./
ifeq  ($(PLATFORM), "Darwin")
    ifeq  ($(PLATFORM), "x86_64")
        CARCH	= -m64 -mmacosx-version-min=10.5
    else
        CARCH	= -arch i386 -arch ppc -m32 -mmacosx-version-min=10.4
    endif
else
    CARCH	= 
endif

CFLAGS 		= -g -Wall $(CARCH) -D$(PLATFORM) $(CINCS) -L./
LIBS		= -lm -lc -lpthread


all:
	(cd common      ; make all  ; make install)
	(cd libsamp     ; make all  ; make install)
	(cd libvotable  ; make all  ; make install)
	(cd libvoclient ; make all  ; make install)
	(cd voapps      ; make all  ; make install)
	(cd libvo       ; make all  ; make install)
	cp voclient.jar voclientd bin/
	/bin/rm -rf bin/curl* lib/pkgconfig
	/bin/rm -rf lib/*.dylib lib/*.la lib/*.so

libs:
	(cd common      ; make libs)
	(cd libvotable  ; make libs)
	(cd libsamp     ; make libs)
	(cd libvoclient ; make libs)
	(cd voapps      ; make libs)
	(cd libvo       ; make libs)

apps:
	(cd common      ; make apps)
	(cd libvotable  ; make apps)
	(cd libsamp     ; make apps)
	(cd libvoclient ; make apps)
	(cd voapps      ; make apps)

examples:
	(cd common      ; make examples)
	(cd libvotable  ; make examples)
	(cd libsamp     ; make examples)
	(cd libvoclient ; make examples)
	(cd voapps      ; make examples)

install:
	(cd common      ;  make install)
	(cd libsamp     ;  make install)
	(cd libvotable  ;  make install)
	(cd libvoclient ;  make install)
	(cd voapps      ;  make install)
	(cd libvo       ;  make install)
	/bin/rm -rf bin/curl* lib/pkgconfig
	/bin/rm -rf lib/*.dylib lib/*.la lib/*.so
	cp voclient.jar voclientd bin/
	cp -rp bin/*      $(BINDIR)
	cp -rp lib/*      $(LIBDIR)
	cp -rp include/*  $(INCDIR)
	cp -rp doc/*.man  $(MANDIR)

clean:
	(cd common      ; make distclean)
	(cd libvotable  ; make clean)
	(cd libsamp     ; make clean)
	(cd libvoclient ; make clean)
	(cd voapps      ; make clean)
	(cd libvo       ; make clean)
	/bin/rm -rf voclient/lib/libvoclient.*
	/bin/rm -rf bin/* lib/* include/* *spool* */*spool* */*/*spool*
	/bin/rm -rf */config.log */*/config.log */*/*/config.log



###############################################################################
# Leave this stuff alone.
###############################################################################

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
