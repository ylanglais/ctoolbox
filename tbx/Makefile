include ../base.mk

#
# Check if syslog is used:
#HASSYSLOG   = $(shell ps auxw | egrep "syslo[g]|metalo[g]" 2>/dev/null && echo 1)
HASSYSLOG    = 1
#
# Check if journald is used:
#HASJOURNALD = $(shell [ -f /usr/include/systemd/sd-journal.h ] && echo 1)
HASJOURNALD  = 1
#
# DEFINE LOG flags:
LOG_CCFLAGS  =
LOG_LDFLAGS  =
ifeq ($(HASSYSLOG),1)
LOG_CCFLAGS += -DHASSYSLOG
endif
ifeq ($(HASJOURNALD),1)
LOG_CCFLAGS += -DHASJOURNALD
LOG_LDFLAGS += -lsystemd
endif
#
#
RE_H = re.h re_posix.h
RE_C = re.c re_posix.c
RE_CCFLAGS = 
RE_LDFLAGS = 
#
HASPCRE1=$(shell [ -x /usr/sbin/pcre-config  ] && echo 1)
HASPCRE2=$(shell [ -x /usr/sbin/pcre2-config ] && echo 1)
#
ifeq (${HASPCRE1},1)
	PCRE_REGEX=1
	PCRE_CCFLAGS=$(shell pcre-config --cflags) -DHASPCRE1
	PCRE_LDFLAGS=$(shell pcre-config --libs)
	RE_CCFLAGS += ${PCRE_CCFLAGS}
	RE_LDFLAGS += ${PCRE_LDFLAGS}
	RE_H += re_pcre.h
	RE_C += re_pcre.c
endif
ifeq (${HASPCRE2},1)
	PCRE2_CCFLAGS=$(shell pcre2-config --cflags) -DHASPCRE2 
	PCRE2_LDFLAGS=$(shell pcre2-config --libs8)
	RE_CCFLAGS += ${PCRE2_CCFLAGS}
	RE_LDFLAGS += ${PCRE2_LDFLAGS}
	RE_H += re_pcre2.h
	RE_C += re_pcre2.c
endif

#
# TBX sources:
TBX_SRCS = argts.c atr.c atree.c patr.c b64.c btree.c byar.c cli.c cmbs.c coverage.c crc_32.c cryp.c csv.c dyna.c err.c fmt.c fsf.c getl.c graph.c hash.c hds.c index.c fifo.c lifo.c list.c log.c map.c mem.c option.c pal.c pipe.c pstore.c parr.c ${RE_C} rexp.c shmem.c srv.c ssl.c storage.c str.c stral.c sthpo.c thpo.c tree.c tstmr.c tuples.c udp.c zprs.c futl.c mlock.c

TBX_OBJS = ${TBX_SRCS:.c=.o}
TBX_HDRS = ${TBX_SRCS:.c=.h}

TARGETS_TEST_RE	 = test-re-pcre test-re-pcre2 test-re-posix

TARGETS_TEST     = test-argts test-atr test-atree test-patr test-btree test-cmbs test-csv test-fsf  \
				   test-tree test-hash test-getl test-graph test-dyna test-fifo test-lifo test-list test-index test-str \
				   test-re test-rexp test-map test-pal test-pipe test-parr test-sthpo test-thpo test-mlock test-pstore test-tstmr  \
				   test-shmem test-byar test-fmt test-crc_32 test-cryp test-tuples test-udp test-log \
				   test-zprs test-b64 $(TARGETS_TEST_RE) test-hds


TARGETS_TEST_NET = test-srv test-cli test-cli2 test-srv test-srv2 test-srv-cli test-ssl-cli test-ssl-srv
TARGETS_CPP      = test-atr++ test-dyna++ test-hash++ test-timestamp++ test-timer++
TARGETS_CMP      = cmp-atha cmp-indexing cmp-indexing-map 
TARGETS          = ${TARGETS_TEST} ${TARGETS_CPP} ${TARGETS_CMP} ${TARGETS_TEST_NET} ${TARGETS_TEST_RE}

all:     conf libtbx.so
alltest: conf libtbx.so test
test:    conf testc testcpp testcmp testnet
testc:   conf $(TARGETS_TEST)
testcmp: conf $(TARGETS_CMP)
testnet: conf $(TARGETS_TEST_NET)
testcpp: conf $(TARGETS_CPP)

include ../conf.mk

#testall: 
#	@echo ">>> Test all"; for i in ${TARGETS} ; do print -n "    $$i...	"; $$i 1>/dev/null 2>&1 && echo "ok" || echo "error"; done

clean:
	@echo ">>> Clean all "; \
	for i in *.o ${TARGETS} *.exe *.gcov *.tcov *.d libtbx.so; do [ -f $$i ] && { echo "    removing $$i"; rm $$i; }; done; \
	for i in *.profile; do [ -d $$i ] && { echo "    removing $$i"; rm -rf $$i; }; done; true

covtest: clean
ifeq ($(OS),SunOS) 
	@COV=1 make test; \
	for target in ${TARGETS}; do echo ">>> coverage analysis of $$target"; $$target 1>/dev/null 2>&1; tcov -x $$target.profile `echo $$target |sed -e 's/test-//'`.c; done
else
	@COV=1 make test; for target in ${TARGETS}; do  echo ">>> coverage analysis of $$target"; $$target 1>/dev/null 2>&1; gcov `echo $$target |sed -e 's/test-//'`.c; done
endif

%.o: %.c %.h
	@$(call compile,$<)

%.optimized.o: %.c
	@echo -n "    compile ${yellow}optimized${norm} ${bold}"$<"${norm}"...; \
	${CC} -c ${TBXINC} $< -I. -o `basename $< .c`.optimized.o -O; \
	$(call status)

log.o: log.c
	@$(call compile,$<, ${LOG_CCFLAGS})

re.o: re.c
	@$(call compile,$<, ${RE_CCFLAGS})

re_pcre.o: re_pcre.c
	@$(call compile,$<, ${PCRE_CCFLAGS})

re_pcre2.o: re_pcre2.c
	@$(call compile,$<, ${PCRE2_CCFLAGS})

libtbx.so: ${TBX_OBJS} ${TBX_SRCS} ${TBX_HDRS}
	@echo -n ">>> Make tbx library $@...";  \
	${CC}    ${CCFLAGS} ${LDOPT} -o $@ ${TBX_OBJS} $(LOG_LDFLAGS) $(RE_LDFLAGS) ${THR_LDFLAGS} $(NETLIBS) $(SSL_LDFLAGS) -lz -ldl ;\
	$(call status)


install: all
	@echo ">>> Install in $(DEST):"; \
	[ -d $(DEST)/include/tbx ] || mkdir -p $(DEST)/include/tbx ; \
	echo "    Install header files in $(DEST)/include/tbx"; \
	install *.h $(DEST)/include/tbx;  \
	echo "    Install C++ header files in $(DEST)/include/tbx"; \
	install *.hh $(DEST)/include/tbx;  \
	echo "    Install libtbx.so in $(DEST)/lib"; \
	install libtbx.so $(DEST)/lib 


# arg 1 = list .o files
# arg 2 = Compilation extra flags
# arg 3 = Link extra flags
build_test = \
	@echo -n ">>> Make $@..." ; \
	${CC} -c ${CCFLAGS} -o    test-$(basename $<).o $< -D_test_$(basename $<)_ ${2} && \
	${CC}    ${CCFLAGS} -o $@ test-$(basename $<).o ${1} ${3} ;\
	$(call status)

#	echo ${CC}    ${CCFLAGS} -o $@ test-$(basename $<).o ${1} ${3} ; \

# arg 1 = list .o files
# arg 2 = Compilation extra flags
# arg 3 = Link extra flags
build_cmp = \
	@echo -n ">>> Make $@..." ; \
	${CC} -c ${CCFLAGS} -o    $(basename $<).o $< ${2} && \
	${CC}    ${CCFLAGS} -o $@ $(basename $<).o ${1} ${3} ;\
	$(call status)
#
# test/demonstrators:
#

ifeq (${HASPCRE1},1) 
REPCRE_O  = re_pcre.o mem.o err.o tstmr.o
test-re-pcre: re.c ${REPCRE_O}
	@echo -n ">>> Make $@..." ; \
	${CC} -c ${CCFLAGS} -o    test-$(basename $<)-pcre.o $< -D_test_$(basename $<)_  ${PCRE_CCFLAGS} && \
	${CC}    ${CCFLAGS} -o $@ test-$(basename $<)-pcre.o ${REPCRE_O} ${PCRE_LDFLAGS} ;\
	$(call status)
endif

ifeq (${HASPCRE2},1) 
REPCRE2_O  = re_pcre2.o mem.o err.o tstmr.o
test-re-pcre2: re.c ${REPCRE2_O}
	@echo -n ">>> Make $@..." ; \
	${CC} -c ${CCFLAGS} -o    test-$(basename $<)-pcre2.o $< -D_test_$(basename $<)_ ${PCRE2_CCFLAGS} && \
	${CC}    ${CCFLAGS} -o $@ test-$(basename $<)-pcre2.o ${REPCRE2_O} ${PCRE2_LDFLAGS} ;\
	$(call status)
endif

REPOSIX_O = re_posix.o mem.o err.o tstmr.o
test-re-posix: re.c ${REPOSIX_O}
	@echo -n ">>> Make $@..." ; \
	${CC} -c ${CCFLAGS} -o    test-$(basename $<)-posix.o $< -D_test_$(basename $<)_ && \
	${CC}    ${CCFLAGS} -o $@ test-$(basename $<)-posix.o ${REPOSIX_O};\
	$(call status)
	
ARGTS_O = 
test-argts: argts.c ${ARGTS_O}
	@$(call build_test,${ARGTS_O},,)

CMBS_O = 
test-cmbs: cmbs.c ${CMBS_O}
	@$(call build_test,${CMBS_O},,)

TUPLES_O = 
test-tuples: tuples.c ${TUPLES_O}
	@$(call build_test,${TUPLES_O},,)

ATR_O   = hash.o storage.o mem.o err.o tstmr.o
test-atr: atr.c ${ATR_O} 
	@$(call build_test,${ATR_O},,)

PATR_O   = hash.o storage.o mem.o err.o tstmr.o
test-patr: patr.c ${PATR_O} 
	@$(call build_test,${PATR_O},,)

PARR_O   = pal.o err.o tstmr.o futl.o mem.o crc_32.o fmt.o
test-parr: parr.c ${PARR_O} 
	@$(call build_test,${PARR_O},,)

INDEX_O   = atr.o list.o mem.o 
test-index: index.c ${INDEX_O}
	@$(call build_test,${INDEX_O},,)

GRAPH_O   = hash.o index.o atr.o list.o mem.o err.o tstmr.o
test-graph: graph.c ${GRAPH_O} 
	@$(call build_test,${GRAPH_O},,)

ATREE_O   = hash.o storage.o mem.o err.o tstmr.o
test-atree: atree.c ${ATREE_O}
	@$(call build_test,${ATREE_O},,)

BTREE_O   =  hash.o storage.o mem.o err.o tstmr.o
test-btree: btree.c ${BTREE_O}
	@$(call build_test,${BTREE_O},,)

CRC32_0   =  
test-crc_32: crc_32.c ${CRC32_O}
	@$(call build_test,${CSV_O},,)

CSV_O   = err.o tstmr.o mem.o
test-csv: csv.c ${CSV_O}
	@$(call build_test,${CSV_O},,)

FSF_O   = err.o tstmr.o csv.o hash.o storage.o mem.o map.o
test-fsf: fsf.c ${FSF_O}
	@$(call build_test,${FSF_O},,)

TREE_O    = err.o tstmr.o
test-tree: tree.c ${TREE_O}
	@$(call build_test,${TREE_O},,)

HASH_O    = mem.o storage.o err.o coverage.o tstmr.o
test-hash: hash.c ${HASH_O}
	@$(call build_test,${HASH_O},-D__HASH_STATISTICS__,)

GETL_O  = srv.o cli.o ssl.o err.o tstmr.o mem.o 
test-getl: getl.c ${GETL_O}
	@$(call build_test,${GETL_O},-D__HASH_STATISTICS__,$(THR_LDFLAGS) $(SSL_LDFLAGS))

LIST_O = mem.o storage.o err.o tstmr.o
test-list: list.c ${LIST_O} 
	@$(call build_test,${LIST_O},,)

FIFO_O = 
test-fifo: fifo.c ${FIFO_O} 
	@$(call build_test,${FIFO_O},,)

LIFO_O = 
test-lifo: lifo.c ${LIFO_O} 
	@$(call build_test,${LIFO_O},,)


LIST_O = mem.o storage.o err.o tstmr.o
test-list: list.c ${LIST_O} 
	@$(call build_test,${LIST_O},,)

MAP_O = err.o tstmr.o 
test-map: map.c ${MAP_O} 
	@$(call build_test,${MAP_O},,)

MLOCK_O = err.o tstmr.o  
test-mlock: mlock.c ${MLOCK_O} 
	@$(call build_test,${MLOCK_O},,)

PIPE_O = err.o futl.o tstmr.o mem.o crc_32.o
test-pipe: pipe.c ${PIPE_O} 
	@$(call build_test,${PIPE_O},,)

T_RE_O = mem.o err.o tstmr.o re_pcre.o re_pcre2.o re_posix.o 
test-re: re.c ${T_RE_O}
	@$(call build_test,${T_RE_O},${RE_CCFLAGS},${RE_LDFLAGS})

FMT_O =
test-fmt: fmt.c ${FMT_O}
	@$(call build_test,${FMT_O},,)

HDS_O   = patr.o storage.o mem.o err.o tstmr.o
test-hds: hds.c ${HDS_O} 
	@$(call build_test,${HDS_O},,)

BYAR_O = mem.o fmt.o
test-byar: byar.c ${BYAR_O}
	@$(call build_test,${BYAR_O},,)

REXP_O = str.o mem.o err.o tstmr.o
test-rexp: rexp.c ${REXP_O}
	@$(call build_test,${REXP_O},,)

STR_O = mem.o err.o tstmr.o
test-str: str.c ${STR_O} 
	@$(call build_test,${STR_O},,)

DYNA_O = mem.o err.o tstmr.o
test-dyna: dyna.c ${DYNA_O} 
	@$(call build_test,${DYNA_O},,)

STRAL_O = mem.o fmt.o
test-stral: stral.c ${STRAL_O} 
	@$(call build_test,${STRAL_O},,)

TSTMR_O = err.o 
test-tstmr: tstmr.c ${TSTMR_O} 
	@$(call build_test,${TSTMR_O},,)

STHPO_O = tstmr.o list.o mem.o err.o
test-sthpo: sthpo.c ${STHPO_O} 
	@$(call build_test,${STHPO_O} ${THR_LDFLAGS},,)

THPO_O = tstmr.o list.o mem.o err.o storage.o
test-thpo: thpo.c ${THPO_O} 
	@$(call build_test,${THPO_O} ${THR_LDFLAGS},,)

PAL_O = err.o tstmr.o fmt.o
test-pal: pal.c ${PAL_O}
	@$(call build_test,${PAL_O},,)

PSTORE_O = err.o pal.o tstmr.o fmt.o mem.o
test-pstore: pstore.c ${PSTORE_O}
	@$(call build_test,${PSTORE_O},,)

SHMEM_O = 
test-shmem: shmem.c ${SHMEM_O}
	@$(call build_test,${SHMEM_O},,)

LOG_O = patr.o storage.o tstmr.o mem.o re.o err.o re_pcre2.o re_pcre.o re_posix.o
test-log: log.c ${LOG_O}
	@$(call build_test,${LOG_O},${RE_CCFLAGS} ${LOG_CCFLAGS},${RE_LDFLAGS} ${LOG_LDFLAGS})
	
ZPRS_O = err.o tstmr.o 
test-zprs: zprs.c ${ZPRS_O}
	@$(call build_test,${ZPRS_O},,-lz)

B64_O = err.o mem.o futl.o crc_32.o tstmr.o
test-b64: b64.c ${B64_O}
	@$(call build_test,${B64_O},,)

CRYP_O = err.o mem.o futl.o tstmr.o crc_32.o
test-cryp: cryp.c ${CRYP_O}
	@$(call build_test,${CRYP_O},,$(SSL_LDFLAGS))
	
UDP_O = err.o tstmr.o
test-udp: udp.c ${UDP_O}
	@$(call build_test,${UDP_O},,)

#@$(call build_test,${SHMEM_O},,)

#
# Perf Comparisons:
#
CMPATHA_C =atree.c atr.c patr.c storage.c mem.c btree.c hash.c index.c list.c coverage.c err.c tstmr.c
CMPATHA_O=${CMPATHA_C:.c=.optimized.o}
cmp-atha: cmp-atha.c ${CMPATHA_O}
	@$(call build_cmp,${CMPATHA_O},,)

CMPINDX_C =atree.c atr.c patr.c storage.c mem.c btree.c hash.c index.c list.c coverage.c err.c tstmr.c
CMPINDX_O=${CMPINDX_C:.c=.optimized.o}
cmp-indexing: cmp-indexing.c ${CMPINDX_O}
	@$(call build_cmp,${CMPINDX_O},,)

CMPINDXMAP_C =atree.c atr.c patr.c mem.c btree.c hash.c index.c list.c coverage.c err.c tstmr.c map.c storage.c
CMPINDXMAP_O=${CMPINDXMAP_C:.c=.optimized.o}
cmp-indexing-map: cmp-indexing-map.c ${CMPINDXMAP_O}
	@$(call build_cmp,${CMPINDXMAP_O},,)

#
# Client server specific:
#
CLI_O = cli.o ssl.o err.o tstmr.o mem.o
test-cli: test-cli.c ${CLI_O}
	@echo -n ">>> Make $@..."; \
	${CC} -c ${CCFLAGS} -o test-cli.o test-cli.c; \
	${CC}    ${CCFLAGS} -o test-cli test-cli.o ${CLI_O} ${NETLIBS} $(SSL_LDFLAGS) ; \
	$(call status)

SRV_O = srv.o ssl.o err.o tstmr.o mem.o
test-srv: test-srv.c ${SRV_O} 
	@echo -n ">>> Make $@..."; \
	${CC} -c ${CCFLAGS} -o test-srv.o test-srv.c ; \
	${CC}    ${CCFLAGS} -o test-srv test-srv.o ${SRV_O} ${NETLIBS} $(SSL_LDFLAGS) ${THR_LDFLAGS} ; \
	$(call status)
	
CLI2_O = cli.o ssl.o err.o tstmr.o mem.o
test-cli2: test-cli2.c ${CLI2_O}
	@echo -n ">>> Make $@..."; \
	${CC} -c ${CCFLAGS} -o test-cli2.o test-cli2.c ; \
	${CC}    ${CCFLAGS} -o test-cli2 test-cli2.o ${CLI2_O} ${NETLIBS}  $(SSL_LDFLAGS); \
	$(call status)

SRV2_O = srv.o ssl.o err.o tstmr.o mem.o
test-srv2: test-srv2.c ${SRV2_O} 
	@echo -n ">>> Make $@..." ; \
	${CC} -c ${CCFLAGS} -o test-srv2.o test-srv2.c ; \
	${CC}    ${CCFLAGS} -o test-srv2 test-srv2.o ${SRV2_O} ${NETLIBS} $(SSL_LDFLAGS) ${THR_LDFLAGS} ; \
	$(call status)

SRV_CLI_O = srv.o cli.o ssl.o err.o tstmr.o mem.o
test-srv-cli: test-srv-cli.c ${SRV_CLI_O}
	@echo -n ">>> Make $@..."; \
	${CC} -c ${CCFLAGS} -o test-srv-cli.o test-srv-cli.c; \
	${CC}    ${CCFLAGS} -o test-srv-cli test-srv-cli.o ${SRV_CLI_O} ${NETLIBS} $(SSL_LDFLAGS) ${THR_LDFLAGS} ; \
	$(call status)

SSL_CLI_O = cli.o ssl.o err.o tstmr.o mem.o
test-ssl-cli: test-ssl-cli.c ${SSL_CLI_O}
	@echo -n ">>> Make $@..."; \
	${CC} -c ${CCFLAGS} -o test-ssl-cli.o test-ssl-cli.c; \
	${CC}    ${CCFLAGS} -o test-ssl-cli   test-ssl-cli.o ${SSL_CLI_O} ${NETLIBS} $(SSL_LDFLAGS) ${THR_LDFLAGS} ; \
	$(call status)

SSL_SRV_O = srv.o ssl.o err.o tstmr.o mem.o
test-ssl-srv: test-ssl-srv.c ${SSL_SRV_O}
	@echo -n ">>> Make $@..."; \
	${CC} -c ${CCFLAGS} -o test-ssl-srv.o test-ssl-srv.c; \
	${CC}    ${CCFLAGS} -o test-ssl-srv   test-ssl-srv.o ${SSL_SRV_O} ${NETLIBS} $(SSL_LDFLAGS) ${THR_LDFLAGS} ; \
	$(call status)

#
# C++ bindings:
#
test-hash++: test-hash++.cc hash.hh hash.c err.o mem.o storage.o tstmr.o coverage.o
	@echo -n ">>> Make $@..."; \
	${CC} -c hash.c ${CCFLAGS} -o hash.stats.o -D__HASH_STATISTICS__ ; \
 	${CCC} ${CCFLAGS} test-hash++.cc -D__HASH_STATISTICS__ -I. -o test-hash++ err.o mem.o storage.o tstmr.o coverage.o hash.stats.o; \
	$(call status)

test-atr++: test-atr++.cc storage.hh atr.hh atr.h mem.o storage.o atr.o
	@echo -n ">>> Make $@..."; \
	${CCC} ${CCFLAGS} test-atr++.cc -I. -o test-atr++ storage.o atr.o mem.o ; \
	$(call status)

test-dyna++: dyna.hh test-dyna++.cc dyna.o mem.o 
	@echo -n ">>> Make $@..."; \
	${CCC} -g -c -o test-dyna.o test-dyna++.cc -I. ;\
	${CCC} test-dyna.o -o test-dyna++ dyna.o mem.o ; \
	$(call status)

test-timestamp++: timestamp.cc timestamp.hh
	@echo -n ">>> Make $@..."; \
	${CCC} -g -c -o timestamp.o timestamp.cc -I. ;\
	${CCC} test-timestamp.cc -o test-timestamp++ timestamp.o -I. ; \
	$(call status)

test-timer++: timer.cc timer.hh timestamp.o
	@echo -n ">>> Make $@..."; \
	${CCC} -g -c -o timer.o timer.cc -I. ; \
	${CCC} test-timer++.cc -o test-timer++ timestamp.o timer.o -I. ; \
	$(call status)

