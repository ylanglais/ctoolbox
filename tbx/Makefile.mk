include ../base.mk

SHELL=/bin/bash

SRCS = atr.c atree.c btree.c csv.c fsf.c tree.c err.c getl.c hash.c list.c index.c graph.c mem.c storage.c option.c rexp.c str.c dyna.c map.c pal.c pstore.c coverage.c shmem.c cli.c srv.c tstmr.c 
OBJS = ${SRCS:.c=.o}

#TARGETS = test-list test-atr test-index test-atree cmp-atha test-btree test-mem-clean test-mem-dirty test-rexp test-str test-hash
TARGETS = test-atr${exe} test-atree${exe} test-btree${exe} test-csv${exe} test-fsf${exe} test-tree${exe} test-hash${exe} test-getl${exe} test-graph${exe} cmp-atha${exe} cmp-indexing${exe} cmp-indexing-map${exe} test-dyna${exe} test-list${exe} test-index${exe} test-str${exe} test-rexp${exe} test-map${exe} test-pal${exe} test-pstore${exe} test-shmem${exe} test-srv${exe} test-cli${exe} test-cli2${exe} test-srv${exe} test-srv2${exe} test-srv-cli${exe} test-tstmr${exe} 
TARGETSPP = test-atr++${exe} test-dyna++${exe} test-hash++${exe} test-timestap++${exe} test-timer++${exe}


all: conf tbx  

alltest: conf tbx test 

test: conf testc testcpp

testc: conf $(TARGETS)
testcpp: conf $(TARGETSPP)

include ../conf.mk

testall: test 
	@echo ">> Test all <<"; for i in ${TARGETS} ; do print -n "    $$i...	"; $$i 1>/dev/null 2>&1 && echo "ok" || echo "error"; done

clean:
	@echo ">> Clean all <<"; \
	for i in *.o ${TARGETS} *.exe *.gcov *.tcov *.d libtbx${lib}; do [ -f $$i ] && { echo "    removing $$i"; rm $$i; }; done; \
	[ -d db ] && ( cd db; make clean ) ; \
	for i in *.profile; do [ -d $$i ] && { echo "    removing $$i"; rm -rf $$i; }; done; true

covtest: clean
ifeq ($(OS),SunOS) 
	@COV=1 make test; \
	for target in ${TARGETS}; do echo "*** coverage analysis of $$target ***"; $$target 1>/dev/null 2>&1; tcov -x $$target.profile `echo $$target |sed -e 's/test-//'`.c; done
else
	@COV=1 make test; for target in ${TARGETS}; do  echo "*** coverage analysis of $$target ***"; $$target 1>/dev/null 2>&1; gcov `echo $$target |sed -e 's/test-//'`.c; done
endif

%.o: %.c
	@echo -n compile "${bold}"$<"${norm}"...; \
	${CC} -c ${CCFLAGS} $< && \
	echo "${green}${bold}ok${norm}" || \
	echo "${red}${bold}failed${norm}"

%.optimized.o: %.c
	@echo -n compile optimized "${bold}"$<"${norm}"...; \
	${CC} -c $< -I. -o `basename $< .c`.optimized.o -O && \
	echo "${green}${bold}ok${norm}" || \
	echo "${red}${bold}failed${norm}"

ATR_O   = hash.o storage.o mem.o err.o tstmr.o
test-atr${exe}: ${ATR_O} atr.c
	@echo "*** Make test-atr${exe} ***" ; \
	${CC} -c ${CCFLAGS} -o test-atr.o atr.c -D_test_atr_ ; \
	${CC}    ${CCFLAGS} -o test-atr${exe} test-atr.o ${ATR_O}

INDEX_O   = atr.o list.o mem.o 
test-index${exe}: ${INDEX_O} index.c
	@echo "*** Make test-index${exe} ***" ; \
	${CC} -c ${CCFLAGS} -o test-index.o index.c -D_test_index_ ; \
	${CC}    ${CCFLAGS} -o test-index${exe} test-index.o ${INDEX_O}

GRAPH_O   = hash.o index.o atr.o list.o mem.o err.o tstmr.o
test-graph${exe}: ${GRAPH_O} graph.c
	@echo "*** Make test-graph ***" ; \
	${CC} -c ${CCFLAGS} -o test-graph.o graph.c -D_test_graph_ ; \
	${CC}    ${CCFLAGS} -o test-graph test-graph.o ${GRAPH_O}

ATREE_O   = hash.o storage.o mem.o err.o tstmr.o
test-atree${exe}: ${ATREE_O} atree.c
	@echo "*** Make test-atree${exe} ***" ; \
	${CC} -c ${CCFLAGS} -o test-atree.o atree.c -D_test_atree_ ; \
	${CC}    ${CCFLAGS} -o test-atree${exe} test-atree.o ${ATREE_O}

BTREE_O   =  hash.o storage.o mem.o err.o tstmr.o
test-btree${exe}: ${BTREE_O} btree.c
	@echo "*** Make test-btree${exe} ***" ; \
	${CC} -c ${CCFLAGS} -o test-btree.o btree.c -D_test_btree_ ; \
	${CC}    ${CCFLAGS} -o test-btree${exe} test-btree.o ${BTREE_O}

CSV_O   =  err.o tstmr.o
test-csv${exe}: ${CSV_O} csv.c 
	@echo "*** Make test-csv ***" ; \
	${CC} -c ${CCFLAGS} -o test-csv.o csv.c -D_test_csv_ ; \
	${CC}    ${CCFLAGS} -o test-csv${exe} test-csv.o ${CSV_O}

FSF_O   =  err.o tstmr.o csv.o hash.o storage.o mem.o
test-fsf${exe}: ${FSF_O} fsf.c 
	@echo "*** Make test-fsf${exe} ***" ; \
	${CC} -c ${CCFLAGS} -o test-fsf.o fsf.c -D_test_fsf_ ; \
	${CC}    ${CCFLAGS} -o test-fsf test-fsf.o ${FSF_O}

TREE_O    =  err.o tstmr.o
test-tree${exe}: tree.c
	@echo "*** Make test-tree${exe} ***" ; \
	${CC} -c ${CCFLAGS} -o test-tree.o tree.c -D_test_tree_ ; \
	${CC}    ${CCFLAGS} -o test-tree${exe} test-tree.o ${TREE_O}

HASH_O    = mem.o storage.o err.o coverage.o tstmr.o
test-hash${exe}: hash.c ${HASH_O}
	@echo "*** Make test-hash${exe} ***" ; \
	${CC} -c ${CCFLAGS} -o test-hash.o hash.c -D_test_hash_ -D__HASH_STATISTICS__ ; \
	${CC}    ${CCFLAGS} -o test-hash${exe} test-hash.o ${HASH_O}

GETL_O  = srv.o cli.o err.o tstmr.o 
test-getl${exe}: ${GETL_O} getl.c 
	@echo "*** Make test-getl${exe} ***" ; \
	${CC} -c ${CCFLAGS} -o test-getl.o getl.c -D_test_getl_ -D__HASH_STATISTICS__ ; \
	${CC}    ${CCFLAGS} -o test-getl${exe} test-getl.o ${GETL_O} -lpthread

CMPATHA_C =atree.c atr.c storage.c mem.c btree.c hash.c index.c list.c coverage.c err.c tstmr.c
CMPATHA_O=${CMPATHA_C:.c=.optimized.o}
cmp-atha${exe}: ${CMPATHA_O} cmp-atha.c
	@echo "*** Make cmp-atha ***" ; \
	${CC} -c ${CCFLAGS} -O -o cmp-atha.o cmp-atha.c ; \
	${CC}    ${CCFLAGS} -O -o cmp-atha${exe} cmp-atha.o ${CMPATHA_O}

CMPINDX_C =atree.c atr.c storage.c mem.c btree.c hash.c index.c list.c coverage.c err.c tstmr.c
CMPINDX_O=${CMPINDX_C:.c=.optimized.o}
cmp-indexing${exe}: ${CMPINDX_O} cmp-indexing.c
	@echo "*** Make cmp-indexing${exe} ***" ; \
	${CC} -c ${CCFLAGS} -O -o cmp-indexing.o cmp-indexing.c ; \
	${CC}    ${CCFLAGS} -O -o cmp-indexing${exe} cmp-indexing.o ${CMPINDX_O}

CMPINDXMAP_C =atree.c atr.c mem.c btree.c hash.c index.c list.c coverage.c err.c tstmr.c map.c storage.c
CMPINDXMAP_O=${CMPINDXMAP_C:.c=.optimized.o}
cmp-indexing-map${exe}: ${CMPINDXMAP_O} cmp-indexing-map.c
	@echo "*** Make cmp-indexing-map${exe} ***" ; \
	${CC} -c ${CCFLAGS} -O -o cmp-indexing-map.o cmp-indexing-map.c ; \
	${CC}    ${CCFLAGS} -O -o cmp-indexing-map${exe} cmp-indexing-map.o ${CMPINDXMAP_O}

LIST_O = mem.o storage.o err.o tstmr.o
test-list${exe}: ${LIST_O} list.c
	@echo "*** Make test-list${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-list.o list.c -D_test_list_; \
	${CC}    ${CCFLAGS} -o test-list${exe} test-list.o ${LIST_O}

MAP_O = 
test-map${exe}: ${MAP_O} map.c  err.o tstmr.o
	@echo "*** Make test-map${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-map.o map.c -D_test_map_; \
	${CC}    ${CCFLAGS} -o test-map${exe} test-map.o ${MAP_O}

MEM_O = 
test-mem-clean${exe}: ${MEM_O} mem.c  err.o tstmr.o
	@echo "*** Make test-mem-clean${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-mem-clean.o mem.c -D_test_mem_clean_; \
	${CC}    ${CCFLAGS} -o test-mem-clean${exe} test-mem-clean.o ${MEM_O}

test-mem-dirty${exe}: ${MEM_O} mem.c err.o tstmr.o
	@echo "*** Make test-mem-dirty${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-mem-dirty.o mem.c -D_test_mem_dirty_; \
	${CC}    ${CCFLAGS} -o test-mem-dirty${exe} test-mem-dirty.o ${MEM_O}

REXP_O = str.o mem.o  err.o tstmr.o
test-rexp${exe}: ${REXP_O} rexp.c 
	@echo "*** Make test-rexp ***"; \
	${CC} -c ${CCFLAGS} -o test-rexp.o rexp.c -D_test_rexp_c_; \
	${CC}    ${CCFLAGS} -o test-rexp${exe} test-rexp.o ${REXP_O}

STR_O = mem.o err.o tstmr.o
test-str${exe}: ${STR_O} str.c
	@echo "*** Make test-str${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-str.o str.c -D_test_str_c_; \
	${CC}    ${CCFLAGS} -o test-str${exe} test-str.o ${STR_O}

DYNA_O = mem.o err.o tstmr.o
test-dyna${exe}: ${DYNA_O} dyna.c
	@echo "*** Make test-dyna${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-dyna.o dyna.c -D_test_dyna_c_; \
	${CC}    ${CCFLAGS} -o test-dyna${exe} test-dyna.o ${DYNA_O}

TSTMR_O = err.o 
test-tstmr${exe}:  ${TSTMR_O} tstmr.c err.o 
	@echo "*** Make test-tstmr${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-tstmr.o tstmr.c -D_test_tstmr_; \
	${CC}    ${CCFLAGS} -o test-tstmr${exe} test-tstmr.o ${TSTMR_O} 

#tsline${exe}:  tstmr.c 
#	@echo "*** Make tsline ***"; \
#	${CC} -g ${CCFLAGS} -o tsline${exe} tstmr.c -D_tsline_

BENCH_O = tstmr.o err.o
test-bench${exe}: ${BENCH_O} bench.c
	@echo "*** Make test-bench${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-bench.o bench.c -D_test_bench_; \
	${CC}    ${CCFLAGS} -o test-bench${exe} test-bench.o ${BENCH_O} 

PAL_O = err.o tstmr.o
test-pal${exe}: ${PAL_O} pal.c
	@echo "*** Make test-pal${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-pal.o pal.c -D_test_pal_; \
	${CC}    ${CCFLAGS} -o test-pal${exe} test-pal.o ${PAL_O} 

PSTORE_O = err.o pal.o tstmr.o
test-pstore${exe}: ${PSTORE_O} pstore.c
	@echo "*** Make test-pstore${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-pstore.o pstore.c -D_test_pstore_; \
	${CC}    ${CCFLAGS} -o test-pstore${exe} test-pstore.o ${PSTORE_O} 

SHMEM_O = shmem.o  err.o tstmr.o
test-shmem${exe}: ${SHMEM_O} #test-shmem.c 
	@echo "*** Make test-shmem ***"; \
	echo "${red}${bold}TODO${norm}" ; \
	#${CC} -c ${CCFLAGS} -o test-shmem.o test-shmem.c ; \
	#${CC}    ${CCFLAGS} -o test-shmem${exe} test-shmem.o ${SHMEM_O}

CLI_O = cli.o err.o tstmr.o
test-cli${exe}: ${CLI_O} test-cli.c
	@echo "*** Make test-cli${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-cli.o test-cli.c; \
	${CC}    ${CCFLAGS} -o test-cli${exe} test-cli.o ${CLI_O} ${NETLIBS} 

SRV_O = srv.o err.o tstmr.o
test-srv${exe}: ${SRV_O} test-srv.c 
	@echo "*** Make test-srv${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-srv.o test-srv.c ; \
	${CC}    ${CCFLAGS} -o test-srv${exe} test-srv.o ${SRV_O} ${NETLIBS} -lpthread 
	
CLI2_O = cli.o err.o tstmr.o
test-cli2${exe}: ${CLI2_O} test-cli2.c
	@echo "*** Make test-cli2${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-cli2.o test-cli2.c ; \
	${CC}    ${CCFLAGS} -o test-cli2${exe} test-cli2.o ${CLI2_O} ${NETLIBS}

SRV2_O = srv.o err.o tstmr.o
test-srv2: ${SRV2_O} test-srv2.c
	@echo "*** Make test-srv2${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-srv2.o test-srv2.c ; \
	${CC}    ${CCFLAGS} -o test-srv2${exe} test-srv2.o ${SRV2_O} ${NETLIBS} -lpthread

SRV_CLI_O = srv.o cli.o err.o tstmr.o
test-srv-cli${exe}:  ${SRV_CLI_O} test-srv-cli.c 
	@echo "*** Make test-srv-cli${exe} ***"; \
	${CC} -c ${CCFLAGS} -o test-srv-cli.o test-srv-cli.c; \
	${CC}    ${CCFLAGS} -o test-srv-cli${exe} test-srv-cli.o ${SRV_CLI_O} ${NETLIBS} -lpthread

test-hash++${exe}: test-hash++.cc hash.hh hash.c err.o mem.o storage.o tstmr.o coverage.o
	@echo "*** Make test-hash++${exe} ***"; \
	${CC} -c hash.c ${CCFLAGS} -o hash.stats.o -D__HASH_STATISTICS__ ; \
 	${CCC} ${CCFLAGS} test-hash++.cc -D__HASH_STATISTICS__ -I. -o test-hash++ err.o mem.o storage.o tstmr.o coverage.o hash.stats.o

test-atr++${exe}: test-atr++.cc storage.hh atr.hh atr.h mem.o storage.o atr.o
	@echo "*** Make test-atr++${exe} ***"; \
	${CCC} ${CCFLAGS} test-atr++.cc -I. -o test-atr++${exe} storage.o atr.o mem.o 

test-timestap++${exe}: timestamp.cc timestamp.hh
	@echo "*** Make test-timestamp++${exe} ***"; \
	${CCC} -g -c -o timestamp.o timestamp.cc -I. ;\
	${CCC} test-timestamp.cc -o test-timestamp++${exe} timestamp.o -I.

test-timer++${exe}: timer.cc timer.hh timestamp.o
	@echo "*** Make test-timer++${exe} ***"; \
	${CCC} -g -c -o timer.o timer.cc -I. ; \
	${CCC} test-timer++.cc -o test-timer++${exe} timestamp.o timer.o -I.

test-dyna++${exe}: dyna.hh test-dyna++.cc dyna.o mem.o 
	@echo "*** Make test-dyna++${exe} ***"; \
	${CCC} -g -c -o test-dyna.o test-dyna++.cc -I. ;\
	${CCC} test-dyna.o -o test-dyna++${exe} dyna.o mem.o 

tbx: ${OBJS}
	@echo "*** Make tbx library libtbx${lib} ***"; \
	${CC}    ${CCFLAGS} ${LDOPT} -o libtbx${lib} ${OBJS} -lpthread ${NETLIBS} -ldl

install: all
	@echo "*** Install header files ***"; \
	install *.h ${HOME}/include;  \
	echo "*** Install C++ header files ***"; \
	install *.hh ${HOME}/include;  \
	echo "*** Install libtbx${lib} ***"; \
	install libtbx${lib} ${HOME}/lib; \


