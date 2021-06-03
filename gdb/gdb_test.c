static char * gdb_call(type_names)[] = {
	"gdbUNDEF",  // 0
	"gdbBOOL",   // 1
	"gdbSTRING", // 2
	"gdbINT2",   // 3
	"gdbINT4",   // 4
	"gdbINT8",   // 5
	"gdbFLOAT4", // 6
	"gdbFLOAT8"  // 7
};

static char *drvname[] = {"PostgreSQL", "MySQL",    "ODBC" };

int
gdb_test(gdbtype_t igdb, char *server, int port, char *login, char *passwd, char *gdbname) {
	int r = 0;
	pquery_t q;
	pgdb_t gdb;

	printf("\ngdb test using %s driver on server %s:\n", drvname[igdb], server); 

#ifdef _test_gdb_
	if (!(gdb = gdb_call(new)(igdb, server, port, login, passwd, gdbname))) {
		err_error("Cannot open connection to %s reason: %s", drvname[igdb], dlerror());
		return 1;
	}
#else 
	if (!(gdb = gdb_call(new)(igdb, server, port, login, passwd, gdbname))) {
		err_error("Cannot open connection to %s reason", drvname[igdb]);
		return 1;
	}
#endif

	
/* 
 * SQL portability issue: 
 * ----------------------

#define dropsql "drop table if exists t1"

*  does not work with MS SQL
*/
#define dropsql "drop table t1"
	if (!(q = gdb_call(query_new)(gdb, dropsql))) {
		err_error("cannot drop table t1");
		//gdb_call(destroy)(gdb);
		//return 2;
	}

/* 
 * SQL portability issue: 
 * ----------------------

#define createsql "create table t1 ( name varchar(10), bval boolean, sval smallint, ival integer, lval bigint, rval real, dval double precision)"
	
 * Boolean does not exist in MSSQL
 */

#define createsql "create table t1 (name varchar(10), bval smallint, sval smallint, ival integer, lval bigint, rval real, dval double precision)"

	if (!(q = gdb_call(query_new)(gdb, createsql))) {
		err_error("cannot create query");
		gdb_call(destroy)(gdb);
		return 3;
	}
	q = gdb_call(query_new)(gdb, "insert into t1 values ('zero',  0, 0, 0, 0, 0.0, 0.0)"); 
	gdb_call(query_destroy)(q);
	q = gdb_call(query_new)(gdb, "insert into t1 values ('one',   1,  1, 1, 1, 1.1, 1.1)"); 
	gdb_call(query_destroy)(q);
	q = gdb_call(query_new)(gdb, "insert into t1 values ('two',   1,  2, 2, 2, 2.2, 2.2)"); 
	gdb_call(query_destroy)(q);
	q = gdb_call(query_new)(gdb, "insert into t1 values ('three', 1,  3, 3, 3, 3.3, 3.3)"); 
	gdb_call(query_destroy)(q);
	q = gdb_call(query_new)(gdb, "insert into t1 values ('four',  1,  4, 4, 4, 4.4, 4.4)"); 
	gdb_call(query_destroy)(q);
	q = gdb_call(query_new)(gdb, "insert into t1 values ('five',  1,  5, 5, 5, 5.5, 5.5)"); 
	gdb_call(query_destroy)(q);
	q = gdb_call(query_new)(gdb, "insert into t1 values ('six',   1,  6, 6, 6, 6.6, 6.6)"); 
	gdb_call(query_destroy)(q);

	q = gdb_call(query_new)(gdb, "select * from t1"); 

	r = 0;
	while (gdb_call(query_row_next)(q)) {
		any_t any;
		if (r == 0) {
			int i;
			printf("Columns are:\n");
			for (i = 0; i <= 6; i++) {
				printf("%d: name = %s type = %-10s (%d) size = %2d\n", 
					i,
					gdb_call(query_col_name)(q, i),
					gdb_call(type_names)[gdb_call(query_col_type)(q, i)],
					gdb_call(query_col_type)(q, i),
					gdb_call(query_col_size)(q, i));
			}	
			any = gdb_call(query_col_by_name)(q, "name");

			printf("\nquery_col_by_name(q, \"name\") = \"%s\"\n", any.val._string);
			free(any.val._string);
			printf("\nDump t1 table:\n");
			r++;
		}	
		char *name;
		char   v_char;
		short  v_short;
		int    v_int;
		long   v_long;
		float  v_float;
		double v_double;

		any  = gdb_call(query_col)(q, 0);
		name = any.val._string;
		any  = gdb_call(query_col)(q, 1);
		v_char = any.val._bool;
		any  = gdb_call(query_col)(q, 2);
		v_short = any.val._int2;
		any  = gdb_call(query_col)(q, 3);
		v_int = any.val._int4;
		any  = gdb_call(query_col)(q, 4);
		v_long = any.val._int8;
		any  = gdb_call(query_col)(q, 5);
		v_float = any.val._float4;
		any  = gdb_call(query_col)(q, 6);
		v_double = any.val._float8;

		printf("%s;%d;%d;%d;%d;%f;%f\n", name, 
			v_char, v_short, v_int, v_long, v_float, v_double);

		free(name);
	}
	gdb_call(query_destroy)(q);
	gdb_call(destroy)(gdb);
	return 0;
}

