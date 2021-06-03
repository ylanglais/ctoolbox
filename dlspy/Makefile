include ../base.mk

SRCS = 
OBJS = ${SRCS:.c=.o}

all: conf config.h dlspy${lib} libhelloworld${lib}

include ../conf.mk

clean:
	@echo ">>> Clean all:"; \
	for i in *.o ${TARGETS} *.exe dlspy${lib} dltest${exe} libhelloworld${lib} config.h ; do [ -f $$i ] && { echo "    removing $$i"; rm $$i; }; done; \
	for i in *.profile; do [ -d $$i ] && { echo "    removing $$i"; rm -rf $$i; }; done; true

%.o: %.c
	@$(call compile,$<)

config.h:
	@echo -n "    get GLIBC version..."; \
	getGLIBC_VERSION.ksh

dlspy${lib}: dlspy.c config.h
	@[ ${OS} = "Cygwin" ] && echo "Cannot build on Cygwin environment" || ( \
		echo -n ">>> Make dlspy${lib}..."; \
		${CC}    ${CCFLAGS} ${LDOPT} -o dlspy${lib} dlspy.c -ldl; \
		$(call status) \
	)

dltest${exe}:
	@echo "${bold}${yellow}>>> build dltest${exe}...${norm}"; \
	gcc dltest.c -o dltest${exe} -ldl
	
libhelloworld${lib}:
	@echo "${bold}${yellow}>>> build libhelloworld${lib}...${norm}"; \
	gcc helloworld.c -o libhelloworld.so ${LDOPT} -ldl

install: all
	@echo ">>> Install in ${DEST}"; \
	[ ${OS} = "Cygwin" ] || { \
		echo "    Install header files in ${DEST}/include"; \
		install *.h ${DEST}/include;  \
		echo "    Install dlspy${lib} in ${DEST}/lib"; \
		install dlspy${lib}  ${DEST}/lib; \
	}	

test: dlspy${lib} dltest${exe} libhelloworld${lib} 
	@echo "${bold}${yellow}>>> w/o dlspy:${norm}"; \
	./dltest${exe} ; \
	echo "${bold}${yellow}>>> w/ dlspy:${norm}" ; \
	LD_PRELOAD=./dlspy${lib} ./dltest${exe}
