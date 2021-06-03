include ../base.mk

HRDS = smpp_def.h pdu.h smpp.h
SRCS = pdu.c smpp.c 
OBJS = ${SRCS:.c=.o}

all: conf campain${exe} campain_r${exe} test-smpp${exe} test-mblox-ref${exe}

test: 
	@true

install: 
	@true

include ../conf.mk

clean: 
	@echo ">>> Clean all:" ; \
	for i in *.o test-smpp${exe} test-mblox-ref${exe} campain${exe} campain_r${exe} ; do [ -f $$i ] && ( echo "    removing $$i"; rm $$i ); done || true

%.o: %.c
	@$(call compile,$<) 

build_test = \
	echo -n ">>> Make $@..." ; \
	${CC} -c ${CCFLAGS} -o    test-$(basename $<).o $< -D_test_$(basename $<)_ ${2} && \
	${CC}    ${CCFLAGS} -o $@ test-$(basename $<).o ${1} ${3} ;\
	$(call status)


test-smpp.o: smpp.c pdu.h smpp_def.h
	@echo -n "    compile ${bold}$@${norm}..." ; \
	${CC} -c ${CCFLAGS} -o $@ $< -D_test_smpp_ ; \
	$(call status)

test-mblox-ref.o: campain.c smpp.c pdu.h smpp_def.h
	@echo -n "    compile ${bold}$@${norm}..." ; \
	${CC} -c ${CCFLAGS} -o $@ $< -D_test_mblox_ref_ ; \
	$(call status)

SMPP_O 	= pdu.o test-smpp.o 
test-smpp${exe}: smpp.c smpp.h smpp_def.h pdu.h pdu.c ${SMPP_O}
	@echo -n "    compile ${bold}$@${norm}..." ; \
	${CC}	${CCFLAGS} -o $@ ${SMPP_O} ${TBXLIB} ; \
	$(call status)

smpp_r.o: smpp.c pdu.h smpp.h smpp_def.h
	@echo -n "    compile ${bold}$@${norm}..." ; \
	${CC} -c ${CCFLAGS} -D__REANTRANT__ -o $@ smpp.c; \
	$(call status)

TEST_MBLOX_REF_O = pdu.o smpp_r.o test-mblox-ref.o
test-mblox-ref${exe}: test-mblox-ref.o smpp_r.o smpp.c smpp.h pdu.h smpp_def.h pdu.c campain.c
	@echo -n "    link ${bold}$@${norm}..." ; \
	${CC}    ${CCFLAGS} -o $@ ${TEST_MBLOX_REF_O} ${TBXLIB}; \
	$(call status)

CAMPAIN_R_O	= pdu.o smpp_r.o ubterm.o
campain_r${exe}: campain.c ubterm.c smpp.c pdu.c ubterm.h smpp.h pdu.h smpp_def.h ${CAMPAIN_R_O}
	@echo -n "    link ${bold}$@${norm}..." ; \
	${CC} -c ${CCFLAGS} campain.c -o campain_r.o -D_threaded_campain_ ; \
	${CC}    ${CCFLAGS} -o $@ campain_r.o ${CAMPAIN_R_O} ${TBXLIB} -lpthread ; \
	$(call status)
	
CAMPAIN_O	= pdu.o smpp_r.o ubterm.o campain.o
campain${exe}: campain.c ubterm.c smpp.c pdu.c ubterm.h smpp.h pdu.h smpp_def.h ${CAMPAIN_O}
	@echo -n "    link ${bold}$@${norm}..." ; \
	${CC}    ${CCFLAGS} -o $@ ${CAMPAIN_O} ${TBXLIB}; \
	$(call status)
	

