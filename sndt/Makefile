include ../base.mk

TARGETS = libsndt${lib} 


all: ${TARGETS} 
test: test-ampl

SNDT_SRCS = wav.c ampl.c frq.c
SNDT_OBJS = ${SNDT_SRCS:.c=.o}

clean: 
	@echo ">> Clean all <<"; \
    for i in *.o ${TARGETS} ; do [ -f $$i ] && { echo "    removing $$i"; rm $$i; }; done; \
    [ -d db ] && ( cd db; make clean ) ; \
    for i in *.profile; do [ -d $$i ] && { echo "    removing $$i"; rm -rf $$i; }; done; true

%.o: %.c %.h
	@$(call compile,$<)
	
libsndt${lib}: ${SNDT_OBJS}
	@echo -n ">>> Make sndt library $@...";  \
	${CC}    ${CCFLAGS} ${LDOPT} -o $@ ${SNDT_OBJS} ${LIBOPT} ;\
	$(call status)

test-ampl: ampl.c wav.o
	@echo ">> make ${bold}${blue}"$@"${norm}"; \
	${CC} -g -c ${CCFLAGS} ampl.c -o test-ampl.o -D_test_ampl_ ; \
	${CC} -g -o $@ test-ampl.o ${LDOPT} wav.o ${EXEOPT}; \
	$(call status) 

install: all
	@echo ">>> Install in $(DEST):"; \
    [ -d $(DEST)/include/tbx/sndt ] || mkdir -p $(DEST)/include/tbx/sndt ; \
    echo "    Install header files in $(DEST)/include/tbx/sndt"; \
    install *.h $(DEST)/include/tbx/sndt;  \
    echo "    Install libsndt${lib} in $(DEST)/lib"; \
    install libsndt${lib} $(DEST)/lib 

