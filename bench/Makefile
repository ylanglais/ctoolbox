include ../base.mk

SHELL=/bin/bash

TARGETS = cpu-bench${exe} db-bench${exe} 

all: conf ${TARGETS}  

include ../conf.mk

clean:
	@echo ">> Clean all <<"; \
	for i in *.o ${TARGETS} *.exe *.so ; do [ -f $$i ] && { echo "    removing $$i"; rm $$i; }; done; \
	[ -d db ] && ( cd db; make clean ) ; \
	for i in *.profile; do [ -d $$i ] && { echo "    removing $$i"; rm -rf $$i; }; done; true

%.o: %.c
	@$(call compile,$<)

db-bench${exe}: db-bench.o bench.o
	@echo "${yellow}${bold}>>> make $@...${norm}"; \
	${CC}  ${CCFLAGS} ${TBXLIB} -O -o $@ $^

cpu-bench${exe}: bench.o cpu-bench.o
	@echo "${yellow}${bold}>>> make $@...${norm}"; \
	${CC}    ${CCFLAGS} ${TBXLIB} -O -o $@ $^

#install: all
#	@echo "*** Install *bench ***"; \
#	install *bench${EXE} ${HOME}/bin


