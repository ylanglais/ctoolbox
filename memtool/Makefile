CCFLAGS = -D__HAS_FUNC__ -D__DEBUG__ -g -I. -DERR_LEVEL -I${HOME}/include
OS      = $(shell uname -s)

ifeq (${OS},SunOS)
	CC = cc -g -D_SUN_ -DSUN -DSOLARIS -v -w 
	LDOPT = -B dynamic -KPIC -G
	NETLIBS = -lsocket -lnsl
	SYBINC=-I/soft/sybase1251_ebf13140/OCS-12_5/include
	SYBLIB=-L/soft/sybase1251_ebf13140/OCS-12_5/lib -lsybdb

ifdef COV
	CC += -xprofile=tcov
endif
else
	CC = gcc -fPIC -D__USE_GNU -Wall
	#CC = /opt/sunstudio12/bin/cc 
	LDOPT = -shared
ifdef COV
	CC += -ftest-coverage
endif
ifeq (${OS},Linux)
	CC += -DLINUX -D_GNU_SOURCE -D__USE_GNU
#	CC += -DLINUX -D_GNU_SOURCE -D_SUN_
endif
endif

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

#TARGETS = test-list test-atr test-index test-atree cmp-atha test-btree test-mem-clean test-mem-dirty test-rexp test-str test-hash

all: nddals nddal mprof mstats 

conf:
	@echo ">> configuration: <<"; \
	echo "OS      = ${OS}"; \
	echo "CC      = ${CC}"; \
	echo "CCFLAGS = ${CCFLAGS}"; \
	echo "LDOPT   = ${LDOPT}"; 

test: $(TARGETS)

testall: test 
	@echo ">> Test all <<"; for i in ${TARGETS} ; do print -n "    $$i...	"; $$i 1>/dev/null 2>&1 && echo "ok" || echo "error"; done

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
	@echo compile $<; \
	${CC} -c ${CCFLAGS} $<

ATR_O   = hash.o storage.o mem.o err.o tstmr.o
test-atr: ${ATR_O} atr.c
	@echo "*** Make test-atr ***" ; \
	${CC} -c ${CCFLAGS} -o test-atr.o atr.c -D_test_atr_ ; \
	${CC}    ${CCFLAGS} -o test-atr test-atr.o ${ATR_O}

INDEX_O   = atr.o list.o mem.o 
test-index: ${INDEX_O} index.c
	@echo "*** Make test-index ***" ; \
	${CC} -c ${CCFLAGS} -o test-index.o index.c -D_test_index_ ; \
	${CC}    ${CCFLAGS} -o test-index test-index.o ${INDEX_O}

GRAPH_O   = hash.o index.o atr.o list.o mem.o err.o tstmr.o
test-graph: ${GRAPH_O} graph.c
	@echo "*** Make test-graph ***" ; \
	${CC} -c ${CCFLAGS} -o test-graph.o graph.c -D_test_graph_ ; \
	${CC}    ${CCFLAGS} -o test-graph test-graph.o ${GRAPH_O}

ATREE_O   = hash.o storage.o mem.o err.o tstmr.o
test-atree: ${ATREE_O} atree.c
	@echo "*** Make test-atree ***" ; \
	${CC} -c ${CCFLAGS} -o test-atree.o atree.c -D_test_atree_ ; \
	${CC}    ${CCFLAGS} -o test-atree test-atree.o ${ATREE_O}

BTREE_O   =  hash.o storage.o mem.o err.o tstmr.o
test-btree: ${BTREE_O} btree.c
	@echo "*** Make test-btree ***" ; \
	${CC} -c ${CCFLAGS} -o test-btree.o btree.c -D_test_btree_ ; \
	${CC}    ${CCFLAGS} -o test-btree test-btree.o ${BTREE_O}

TREE_O    =  err.o tstmr.o
test-tree: tree.c
	@echo "*** Make test-tree ***" ; \
	${CC} -c ${CCFLAGS} -o test-tree.o tree.c -D_test_tree_ ; \
	${CC}    ${CCFLAGS} -o test-tree test-tree.o ${TREE_O}

HASH_O    = mem.o storage.o err.o coverage.o tstmr.o
test-hash: hash.c ${HASH_O}
	@echo "*** Make test-hash ***" ; \
	${CC} -c ${CCFLAGS} -o test-hash.o hash.c -D_test_hash_ -D__HASH_STATISTICS__ ; \
	${CC}    ${CCFLAGS} -o test-hash test-hash.o ${HASH_O}

CMPATHA_C =atree.c atr.c storage.c mem.c btree.c hash.c index.c list.c coverage.c err.c tstmr.c
CMPATHA_O=${CMPATHA_C:.c=.optimized.o}
cmp-atha: ${CMPATHA_C} cmp-atha.c
	@echo "*** Make cmp-atha ***" ; \
	for i in ${CMPATHA_C}; do \
		${CC} -c $$i -I. -o `basename $$i .c`.optimized.o -O ; \
	done; \
	${CC} -c ${CCFLAGS} -O -o cmp-atha.o cmp-atha.c ; \
	${CC}    ${CCFLAGS} -O -o cmp-atha cmp-atha.o ${CMPATHA_O}

LIST_O = mem.o storage.o err.o tstmr.o
test-list: ${LIST_O} list.c
	@echo "*** Make test-list ***"; \
	${CC} -c ${CCFLAGS} -o test-list.o list.c -D_test_list_; \
	${CC}    ${CCFLAGS} -o test-list test-list.o ${LIST_O}

MAP_O = 
test-map: ${MAP_O} map.c  err.o tstmr.o
	@echo "*** Make test-map ***"; \
	${CC} -c ${CCFLAGS} -o test-map.o map.c -D_test_map_; \
	${CC}    ${CCFLAGS} -o test-map test-map.o ${MAP_O}

MEM_O = 
test-mem-clean: ${MEM_O} mem.c  err.o tstmr.o
	@echo "*** Make test-mem-clean ***"; \
	${CC} -c ${CCFLAGS} -o test-mem-clean.o mem.c -D_test_mem_clean_; \
	${CC}    ${CCFLAGS} -o test-mem-clean test-mem-clean.o ${MEM_O}

test-mem-dirty: ${MEM_O} mem.c err.o tstmr.o
	@echo "*** Make test-mem-dirty ***"; \
	${CC} -c ${CCFLAGS} -o test-mem-dirty.o mem.c -D_test_mem_dirty_; \
	${CC}    ${CCFLAGS} -o test-mem-dirty test-mem-dirty.o ${MEM_O}

REXP_O = str.o mem.o  err.o tstmr.o
test-rexp: ${REXP_O} rexp.c 
	@echo "*** Make test-rexp ***"; \
	${CC} -c ${CCFLAGS} -o test-rexp.o rexp.c -D_test_rexp_c_; \
	${CC}    ${CCFLAGS} -o test-rexp test-rexp.o ${REXP_O}

STR_O = mem.o err.o tstmr.o
test-str: ${STR_O} str.c
	@echo "*** Make test-str ***"; \
	${CC} -c ${CCFLAGS} -o test-str.o str.c -D_test_str_c_; \
	${CC}    ${CCFLAGS} -o test-str test-str.o ${STR_O}

DYNA_O = mem.o err.o tstmr.o
test-dyna: ${DYNA_O} dyna.c
	@echo "*** Make test-dyna ***"; \
	${CC} -c ${CCFLAGS} -o test-dyna.o dyna.c -D_test_dyna_c_; \
	${CC}    ${CCFLAGS} -o test-dyna test-dyna.o ${DYNA_O}

TSTMR_O = 
test-tstmr:  ${TSTMR_O} tstmr.c 
	@echo "*** Make test-tstmr ***"; \
	${CC} -c ${CCFLAGS} -o test-tstmr.o tstmr.c -D_test_tstmr_; \
	${CC}    ${CCFLAGS} -o test-tstmr test-tstmr.o ${TSTMR_O} 

tsline:  tstmr.c 
	@echo "*** Make tsline ***"; \
	${CC} -g ${CCFLAGS} -o tsline tstmr.c -D_tsline_

BENCH_O = tstmr.o
test-bench: ${BENCH_O} bench.c
	@echo "*** Make test-bench ***"; \
	${CC} -c ${CCFLAGS} -o test-bench.o bench.c -D_test_bench_; \
	${CC}    ${CCFLAGS} -o test-bench test-bench.o ${BENCH_O} 

SHMEM_O = shmem.o  err.o tstmr.o
test-shmem: ${SHMEM_O} test-shmem.c 
	@echo "*** Make test-shmem ***"; \
	${CC} -c ${CCFLAGS} -o test-shmem.o test-shmem.c ; \
	${CC}    ${CCFLAGS} -o test-shmem test-shmem.o ${SHMEM_O}

CLI_O = cli.o err.o tstmr.o
test-cli: ${CLI_O} test-cli.c
	@echo "*** Make test-cli ***"; \
	${CC} -c ${CCFLAGS} -o test-cli.o test-cli.c; \
	${CC}    ${CCFLAGS} -o test-cli test-cli.o ${CLI_O} ${NETLIBS} 

SRV_O = srv.o err.o tstmr.o
test-srv: ${SRV_O} test-srv.c 
	@echo "*** Make test-srv ***"; \
	${CC} -c ${CCFLAGS} -o test-srv.o test-srv.c ; \
	${CC}    ${CCFLAGS} -o test-srv test-srv.o ${SRV_O} ${NETLIBS} -lpthread 
	
CLI2_O = cli.o err.o tstmr.o
test-cli2: ${CLI2_O} test-cli2.c
	@echo "*** Make test-cli2 ***"; \
	${CC} -c ${CCFLAGS} -o test-cli2.o test-cli2.c ; \
	${CC}    ${CCFLAGS} -o test-cli2 test-cli2.o ${CLI2_O} ${NETLIBS}

SRV2_O = srv.o err.o tstmr.o
test-srv2: ${SRV2_O} test-srv2.c
	@echo "*** Make test-srv2 ***"; \
	${CC} -c ${CCFLAGS} -o test-srv2.o test-srv2.c ; \
	${CC}    ${CCFLAGS} -o test-srv2 test-srv2.o ${SRV2_O} ${NETLIBS}

SRV_CLI_O = srv.o cli.o err.o tstmr.o
test-srv-cli:  ${SRV_CLI_O} test-srv-cli.c 
	@echo "*** Make test-srv-cli ***"; \
	${CC} -c ${CCFLAGS} -o test-srv-cli.o test-srv-cli.c; \
	${CC}    ${CCFLAGS} -o test-srv-cli test-srv-cli.o ${SRV_CLI_O} ${NETLIBS}

test-Hash: test-Hash.cc Hash.h hash.c err.o mem.o storage.o tstmr.o coverage.o
	${CC} -c hash.c ${CCFLAGS} -o hash.stats.o -D__HASH_STATISTICS__ ; \
	 CC ${CCFLAGS} test-Hash.cc -D__HASH_STATISTICS__ -I. -o test-Hash err.o mem.o storage.o tstmr.o coverage.o hash.stats.o

test-Atr: test-Atr.cc Storage.h Atr.h mem.o storage.o atr.o
	 CC ${CCFLAGS} test-Atr.cc -I. -o test-Atr storage.o atr.o mem.o 

test-Timestap: Timestamp.cc Timestamp.h
	CC -g -c -o Timestamp.o Timestamp.cc -I. ;\
	CC test-Timestamp.cc -o test-Timestamp Timestamp.o -I.

test-Timer: Timer.cc Timer.h Timestamp.o
	CC -g -c -o Timer.o Timer.cc -I. ; \
	CC test-Timer.cc -o test-Timer Timestamp.o Timer.o -I.

test-Dyna: Dyna.h test-Dyna.cc dyna.o mem.o 
	CC -g -c -o test-Dyna.o test-Dyna.cc -I. ;\
	CC test-Dyna.o -o test-Dyna dyna.o mem.o 

tbx: ${OBJS}
	@echo "*** Make tbx library libtbx.so ***"; \
	${CC}    ${CCFLAGS} ${LDOPT} -o libtbx.so ${OBJS} -lpthread ${NETLIBS} -ldl

dlspy: dlspy.c 
	@echo "*** Make dlspy library (dlspy.so) ***"; \
	${CC}    ${CCFLAGS} ${LDOPT} -o dlspy.so dlspy.c -ldl

nddals: ${nddalsOBJS}
	@echo "*** Make nddals library (nddals.so) ***"; \
	${CC}    ${CCFLAGS} ${LDOPT} -o nddals.so ${nddalsOBJS} -ldl

nddal: ${nddalOBJS}
	@echo "*** Make nddal library (nddal.so) ***"; \
	${CC}    ${CCFLAGS} ${LDOPT} -o nddal.so ${nddalOBJS} -ldl

mprof: ${mprofOBJS} 
	@echo "*** Make mprof library (mprof.so) ***"; \
	${CC}    ${CCFLAGS} ${LDOPT} -o mprof.so ${mprofOBJS} -ldl

mstats: ${mstatsOBJS} 
	@echo "*** Make mstats library (mstats.so) ***"; \
	${CC}    ${CCFLAGS} ${LDOPT} -o mstats.so ${mstatsOBJS} -lpthread -ldl

libsybase:  sybase.c sybase.h
	echo "*** Make libsybase library (libsybase.so) ***"; \
	${CC}    ${CCFLAGS} ${LDOPT} ${SYBINC} sybase.c ${SYBLIB} -o libsybase.so ${sybOBJS}
	

install: all
	@echo "*** Install header files ***"; \
	install *.h ${HOME}/include;  \
	echo "*** Install tsline ***"; \
	install tsline ${HOME}/bin; \
	echo "*** Install libtbx.so ***"; \
	install libtbx.so ${HOME}/lib; \
	echo "*** Install dlspy.so ***"; \
	install dlspy.so  ${HOME}/lib; \
	echo "*** Install nddal.so ***"; \
	install nddal.so   ${HOME}/lib ; \
	echo "*** Install nddals.so ***"; \
	install nddals.so   ${HOME}/lib ; \
	echo "*** Install mprof.so ***"; \
	install mprof.so   ${HOME}/lib; \
	echo "*** Install mstats.so ***"; \
	install mstats.so   ${HOME}/lib ; \
	[ -f libsybase.so ] && { \
			echo "*** Install libsybase.so ***"; \
			install libsybase.so ${HOME}/lib; } \
	|| true;


