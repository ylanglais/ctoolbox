SHELL=/bin/bash
#
# Replace DEST by destination directory:
#
DEST = ${HOME}
#
# Replace TBX by TBX home directory:
TBX  = ${HOME}
#
# 
TBXINC = -I${TBX}/include
TBXLIB = -L${TBX}/lib -ltbx


#.SILENT:

REENTRANT = -D__REENTRANT__ -D__reentrant__

THR_CCFLAGS = ${REENTRANT}
THR_LDFLAGS = -lpthread

SSL_CCFLAGS = 
SSL_LDFLAGS = -lssl -lcrypto

CCFLAGS = -D__HAS_FUNC__ -D__DEBUG__ -g -I. -DERR_LEVEL ${THR_CCFLAGS} ${TBXINC}
OS      = $(shell uname -s)
OS2     = $(shell uname -o)

#CLANG   = $(shell ls /usr/bin/clang)
#CLANGPP = $(shell ls /usr/bin/clang++)
GCC     = $(shell ls /usr/bin/gcc)
GPP     = $(shell ls /usr/bin/g++)

#$(shell echo "CLANG = ${CLANG}")

ifeq (${OS2},Cygwin)
	OS = ${OS2}
endif

exe     =
lib		= .so

ifeq (${OS},SunOS)
	CC  = cc -g -D_SUN_ -DSUN -DSOLARIS -v -w -m64 -KPIC -xmodel=medium
	CCC = CC -g -D_SUN_ -DSUN -DSOLARIS -v -w -m64 -KPIC -xmodel=medium
	#CC = gcc -g -D_SUN_ -DSUN -DSOLARIS  -Wall 
	LDOPT = -G -B dynamic -KPIC
	NETLIBS = -lsocket -lnsl
ifdef COV
	CC += -xprofile=tcov
endif
else
ifeq (${OS},Linux) 
ifeq (${CLANG},/usr/bin/clang) 
		CC  = clang -Wall -Wno-ignored-attributes -Wno-pointer-bool-conversion -Wno-empty-body -Wno-format -fPIC -DLINUX -D__USE_GNU -D_GNU_SOURCE 
else
		CC  = gcc -Wall -Wno-format -fPIC -DLINUX -D__USE_GNU -D_GNU_SOURCE 
endif
ifeq (${CLANGPP},/usr/bin/clang++)
		CCC = clang++ -Wall -Wno-format -fPIC -DLINUX -D__USE_GNU -D_GNU_SOURCE
else 
		CCC = g++ -Wall -Wno-format -fPIC -DLINUX -D__USE_GNU -D_GNU_SOURCE
endif
		LDOPT = -shared 
endif
ifeq (${OS},Cygwin) 
		exe     = .exe
		lib		= .dll
		CC  = gcc -Wall -Wno-format -DCYGWIN -D__USE_GNU -D_GNU_SOURCE
		CCC = g++ -Wall -Wno-format -DCYGWIN -D__USE_GNU -D_GNU_SOURCE
		LDOPT = -shared 
endif
ifdef COV
	CC += -ftest-coverage
endif
endif

status = \
	[ $$? -eq 0 ] &&  echo "${green}${bold}ok${norm}" || { echo "${red}${bold}failed${norm}"; false; }


#
# Compile function:
# arg1 = .c to compile
# arg2 = compilation flags
compile = \
	@echo -n "    compile ${bold}"$(1)"${norm}..."; \
	${CC} -c ${CCFLAGS} $(1) $(2) ; \
	$(call status)


#	echo "${green}${bold}ok${norm}" || \
	echo "${red}${bold}failed${norm}"

#
# Build LIB:
# arg1 = compile options
# arg2 = link options
build_lib = \
	@echo -n ">>> link ${bold}$@${norm}..."; \
	${CC} ${CCFLAGS} ${LDOPT} -o $@ $(1) ${TBXLIB} $(2); \
	$(call status)

#
# Build EXE:
# arg1 = compile options
# arg2 = link options
build_exe = \
	echo -n ">>> build ${bold}$@${norm}..."; \
	${CC} ${CCFLAGS} -o $@ $(basename $<).o ${TBXLIB} $(2); \
	$(call status)

