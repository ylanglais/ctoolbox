include ../base.mk

XML2INC = $(shell xml2-config --cflags)
XML2LIB = $(shell xml2-config --libs)

# OBJ sources:
OBJ_SRCS =  
OBJ_OBJS = ${OBJ_SRCS:.c=.o}
OBJ_HDRS = ${OBJ_SRCS:.c=.h}

QDXML_SRCS = qdxml.c
QDXML_OBJS = ${QDXML_SRCS:.c=.o}
QDXML_HDRS = ${QDXML_SRCS:.c=.h}

SPEC_SRCS  = spec.c
SPEC_OBJS  = ${SPEC_SRCS:.c=.o}
SPEC_HDRS  = ${SPEC_SRCS:.c=.o}

#TGTS_TSTS = test-obj${exe} test-spec${exe}
TARGETS   = ${TGTS_TSTS} libqdxml${lib}

all:     conf test libqdxml${lib} 
alltest: test
test:    ${TGTS_TSTS} 

install: install-qdxml

include ../conf.mk

DEST=${HOME}

clean:
	@echo ">>> Clean all "; \
	for i in *.o ${TARGETS} *.exe *.gcov *.tcov *.d; do [ -f $$i ] && { echo "    removing $$i"; rm $$i; }; done; \
	for i in *.profile; do [ -d $$i ] && { echo "    removing $$i"; rm -rf $$i; }; done; true

.c.o:  %.c %.h
	@$(call compile,$<)


%.o: %.c %.h
	@$(call compile,$<)

spec.o: spec.c spec.h
	@$(call compile,$<,${XML2INC},${XML2LIB})

build_test = \
	echo -n ">>> Make $@..." ; \
	${CC} -c ${CCFLAGS} -o    test-$(basename $<).o $< -D_test_$(basename $<)_ ${2} ${TBXINC} && \
	${CC}    ${CCFLAGS} -o $@ test-$(basename $<).o ${1} ${3} ${TBXLIB} ;\
	$(call status)

install-qdxml: libqdxml${lib}
	@echo ">>> Install qdxml.h in $(DEST)/include/tbx/xml" ;\
	[ -d  $(DEST)/include/tbx/xml ] || mkdir -p $(DEST)/include/tbx/xml; \
	install *.h $(DEST)/include/tbx/xml; \
	echo "    Install libqdxml${lib} in $(DEST)/lib"; \
	install libqdxml${lib} $(DEST)/lib 

libqdxml${lib}: ${QDXML_OBJS} ${QDXML_SRCS} ${QDXML_HDRS}
	@echo -n ">>> Make qdxml library $@..."; \
	${CC}    ${CCFLAGS} ${LDOPT} -o $@ ${QDXML_OBJS}; \
	$(call status)

#
# test/demonstrators:
#
OBS_O = 
test-obj${exe}: obj.c ${OBJ_O}
	@$(call build_test,${OBJ_O},,)

SPEC_O = obj.o qdxml.o
test-spec${exe}: spec.c ${SPEC_O} 
	@$(call build_test,${SPEC_O},${XML2INC},${XML2LIB})

