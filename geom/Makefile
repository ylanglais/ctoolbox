include ../base.mk

# TBX sources:
GEOM_SRCS = mat3.c mat4.c trsfm3.c trsfm2.c

GEOM_OBJS = ${GEOM_SRCS:.c=.o}

TARGETS   = test-mat3${exe} test-mat4${exe} test-trsfm2${exe} test-trsfm3${exe} cmp-mat4${exe} libgeom${lib}

CCFLAGS += -Wmissing-braces

all:     conf libgeom${lib}  
alltest: conf libgeom${lib} test 
test:    conf ${TARGETS}

include ../conf.mk

#testall: 
#	@echo ">>> Test all"; for i in ${TARGETS} ; do print -n "    $$i...	"; $$i 1>/dev/null 2>&1 && echo "ok" || echo "error"; done

clean:
	@echo ">>> Clean all "; \
	for i in *.o ${TARGETS} *.exe *.gcov *.tcov *.d libtbx${lib}; do [ -f $$i ] && { echo "    removing $$i"; rm $$i; }; done; \
	for i in *.profile; do [ -d $$i ] && { echo "    removing $$i"; rm -rf $$i; }; done; true

%.o: %.c
	@$(call compile,$<)

libgeom${lib}: ${GEOM_OBJS}
	@echo -n ">>> Make geom library $@...";  \
	${CC}    ${CCFLAGS} ${LDOPT} -o $@ ${GEOM_OBJS} -lm ;\
	$(call status)

install: all
	@echo ">>> Install in $(DEST):"; \
	echo "    Install header files in $(DEST)/include/tbx/geom"; \
	[ -d  $(DEST)/include/tbx/geom ] || mkdir -p $(DEST)/include/tbx/geom;  \
	install *.h $(DEST)/include/tbx/geom;  \
	echo "    Install libgeom${lib} in $(DEST)/lib"; \
	install libgeom${lib} $(DEST)/lib 


# arg 1 = list .o files
# arg 2 = Compilation extra flags
# arg 3 = Link extra flags
build_test = \
	echo -n ">>> Make $@..." ; \
	${CC} -c ${CCFLAGS} -o    test-$(basename $<).o $< -D_test_$(basename $<)_ ${2} && \
	${CC}    ${CCFLAGS} -o $@ test-$(basename $<).o ${1} ${3} ;\
	$(call status)

#
# test/demonstrators:
#
MATRIX3_O = 
test-mat3${exe}: mat3.c ${MATRIX3_O}
	@$(call build_test,${MATRIX3_O},,-lm)

MATRIX4_O = 
test-mat4${exe}: mat4.c ${MATRIX3_O}
	@$(call build_test,${MATRIX4_O},,-lm)

TRSFM2_O = mat3.o
test-trsfm2${exe}: trsfm2.c ${TRSFM2_O}
	@$(call build_test,${TRSFM2_O},,-lm)

TRSFM3_O = mat4.o
test-trsfm3${exe}: trsfm3.c ${TRSFM3_O}
	@$(call build_test,${TRSFM3_O},,-lm)

# arg 1 = list .o files
# arg 2 = Compilation extra flags
# arg 3 = Link extra flags
build_cmp = \
	echo -n ">>> Make $@..." ; \
	${CC} -c ${CCFLAGS} -o    $(basename $<).o $< ${2} && \
	${CC}    ${CCFLAGS} -o $@ $(basename $<).o ${1} ${3} ;\
	$(call status)

CMPMATRIX4_C =
CMPMATRIX4_O=${CMPMATRIX4_C:.c=.optimized.o}
cmp-mat4${exe}: cmp-mat4.c ${CMPMATRIX4_O}
	@$(call build_cmp,${CMPMATRIX4_O},${TBXINC},${TBXLIB})

