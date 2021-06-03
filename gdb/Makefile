include ../base.mk

PGINC   = -I$(shell pg_config --includedir)
PGLIB   = -L$(shell pg_config --libdir) -lpq $(shell pg_config --libs) 

MYINC   = $(shell mysql_config --cflags)
MYLIB   = $(shell mysql_config --libs) 

ODINC   = 
ODLIB   = -lodbc

TARGETS = libgdb${lib} test-gdb${exe} test-pgsql${exe} test-mysql${exe} 

all: conf libs 

#libs: libgdb_pgsql${lib} libgdb_mysql${lib} libgdb_odbc${lib} libgdb${lib}
libs: libgdb_pgsql${lib} libgdb_mysql${lib} libgdb${lib}

#test: conf test-pgsql${exe} test-mysql${exe} test-odbc${exe} test-gdb${exe} 
test: conf test-pgsql${exe} test-mysql${exe} test-gdb${exe} 

include ../conf.mk

clean:
	@echo ">> Clean all <<"; \
	for i in *.o ${TARGETS} *${lib} *.exe; do [ -f $$i ] && { echo "    removing $$i"; rm $$i; }; done; \
	for i in *.profile; do [ -d $$i ] && { echo "    removing $$i"; rm -rf $$i; }; done; true

gdb_pgsql.c: gdb_pgsql.h pgtypes.h

%.c: %.h

%.o: %.c
	$(call compile,$<)

pgtypes.h:
	@echo -n "    Generate ${bold}pg_types.h${norm}..." ; \
	egrep "NAMEOID|TEXTOID|BPCHAROID|VARCHAROID|BOOLOID|INT2OID|INT4OID|INT8OID|FLOAT4OID|FLOAT8OID" `pg_config --includedir`/catalog/pg_type.h > pgtypes.h && echo "${green}${bold}ok${norm}" || \
	echo "${red}${bold}failed${norm}"

# arg 1 = Compilation extra flags
# arg 2 = Link extra flags
build_test = \
	@echo -n ">>> Make $@..." ; ( \
	${CC}    ${CCFLAGS} -o $@ $< ${1} ${2} -D_test_$(basename $<)_ ${TBXLIB}; \
	) && echo "${green}${bold}ok${norm}" || \
	echo "${red}${bold}failed${norm}"


#test-gdb${exe}: gdb.c libgdb_pgsql${lib} libgdb_mysql${lib} libgdb_odbc${lib} gdb.c gdb.h 
test-gdb${exe}: gdb.c gdb.h libgdb_pgsql${lib} libgdb_mysql${lib} 
	@$(call build_test,,-ldl)

test-mysql${exe}: gdb_mysql.c gdb_mysql.h
	@$(call build_test,,${MYLIB}) 

test-pgsql${exe}: gdb_pgsql.c gdb_pgsql.h
	@$(call build_test,,${PGLIB}) 

#
#	Libs:
#
PG_O = gdb_pgsql.o
libgdb_pgsql${lib}: gdb_pgsql.o gdb_pgsql.h pgtypes.h
	$(call build_lib,${PG_O},${PGLIB})

libgdb_mysql${lib}: gdb_mysql.o 
	@$(call build_lib,$<,${MYLIB})

libgdb_odbc${lib}: gdb_odbc.o 
	@$(call build_lib,$<,${ODLIB})

libgdb${lib}: gdb.o
	@$(call build_lib,$<,-ldl)

#
# 	install
#
install: all
	@echo ">>> Install in ${DEST}:" ; \
	echo "    Install header files in ${DEST}/include/tbx/gdb" ; \
	[ -d ${DEST}/include/tbx/gdb ] || mkdir -p ${DEST}/include/tbx/gdb; \
	install *.h ${DEST}/include/tbx/gdb ;  \
	echo "    Install libgdb${lib} int $(DEST)/lib"; \
	install libgdb${lib} ${DEST}/lib; \
	echo "    Install libgdb_pgsql${lib} int $(DEST)/lib"; \
	install libgdb_pgsql${lib}  ${DEST}/lib; \
	echo "    Install libgdb_odbc${lib} int $(DEST)/lib"; \
	install libod${lib}  ${DEST}/lib; \
	echo "    Install libgdb_mysql${lib} int $(DEST)/lib"; \
	install libgdb_mysql${lib}  ${DEST}/lib; 

