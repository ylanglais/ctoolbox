include ../base.mk

SRCS = 
OBJS = ${SRCS:.c=.o}

nddalsSRCS = nddals.c fmt.c
nddalsOBJS = ${nddalsSRCS:.c=.o}

nddalSRCS = stack.c nddal.c fmt.c
nddalOBJS = ${nddalSRCS:.c=.o}

mprofSRC  = mprof.c
mprofOBJS = ${mprofSRC:.c=.o}

mstatsSRC  = mstats.c
mstatsOBJS = ${mstatsSRC:.c=.o}

all: nddals${lib} nddal${lib} mprof${lib} mstats${lib} 

conf:
	@echo ">> configuration: <<"; \
	echo "OS      = ${OS}"; \
	echo "CC      = ${CC}"; \
	echo "CCFLAGS = ${CCFLAGS}"; \
	echo "LDOPT   = ${LDOPT}"; 

test:
	@true

clean:
	@echo ">> Clean all <<"; \
	for i in *.o ${TARGETS} *.gcov *.tcov *.d libtbx.so dlspy.so nddal.so nddals.so mprof.so mstats.so; do [ -f $$i ] && { echo "    removing $$i"; rm $$i; }; done; \
	for i in *.profile; do [ -d $$i ] && { echo "    removing $$i"; rm -rf $$i; }; done; true 

covtest: clean
ifeq ($(OS),SunOS) 
	@COV=1 make test; for target in ${TARGETS}; do echo "*** coverage analysis of $$target ***"; $$target 1>/dev/null 2>&1; tcov -x $$target.profile `echo $$target |sed -e 's/test-//'`.c; done
else
	@COV=1 make test; for target in ${TARGETS}; do  echo "*** coverage analysis of $$target ***"; $$target 1>/dev/null 2>&1; gcov `echo $$target |sed -e 's/test-//'`.c; done
endif

%.o: %.c
	@$(call compile,$<)

build_mlib=\
	echo -n ">>> link ${bold}$@${norm}..."; \
	${CC} ${CCFLAGS} ${LDOPT} -o $@ $? -ldl; \
	$(call status)

nddals${lib}: ${nddalsOBJS}
	@$(call build_mlib)

nddal${lib}: ${nddalOBJS}
	@$(call build_mlib)

mprof${lib}: ${mprofOBJS} 
	@$(call build_mlib)

mstats${lib}: ${mstatsOBJS} 
	@$(call build_mlib)

install: all
	@echo ">>> Install in $(DEST):" ; \
	echo "    header files in $(DEST)/include"; \
	install *.h ${DEST}/include;  \
	echo "    Install nddal.so in $(DEST)/lib"; \
	install nddal.so   ${DEST}/lib ; \
	echo "    Install nddals.so in $(DEST)/lib"; \
	install nddals.so   ${DEST}/lib ; \
	echo "    Install mprof.so in $(DEST)/lib"; \
	install mprof.so   ${DEST}/lib; \
	echo "    Install mstats.so in $(DEST)/lib"; \
	install mstats.so   ${DEST}/lib 


