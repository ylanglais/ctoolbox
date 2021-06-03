

static char * db_call(type_names)[] = {
	"dbUNDEF",  // 0
	"dbBOOL",   // 1
	"dbSTRING", // 2
	"dbINT2",   // 3
	"dbINT4",   // 4
	"dbINT8",   // 5
	"dbFLOAT4", // 6
	"dbFLOAT8"  // 7
};

static char *drvname[] = {"PostgreSQL", "MySQL",    "ODBC" };

int
db_test(dbtype_t idb, char *server, int port, char *login, char *passwd, char *db) {
	int r = 0;
	pquery_t q;

	printf("\ndb test using %s driver on server %s:\n", drvname[idb], server); 

#ifdef _test_db_
	if (db_call(new)(idb, server, port, db, login, passwd)) {
		err_error("Cannot open connection to %s reason: %s", drvname[idb], dlerror());
		return 1;
	}
#else 
	if (db_call(new)(server, port, db, login, passwd)) {
		err_error("Cannot open connection to %s reason", drvname[idb]);
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
	if (!(q = db_call(query_new)(dropsql))) {
		err_error("cannot drop table t1");
		//db_call(destroy)();
		//return 2;
	}

/* 
 * SQL portability issue: 
 * ----------------------

#define createsql "create table t1 ( name varchar(10), bval boolean, sval smallint, ival integer, lval bigint, rval real, dval double precision)"
	
 * Boolean does not exist in MSSQL
 */

#define createsql "create table t1 (name varchar(10), bval smallint, sval smallint, ival integer, lval bigint, rval real, dval double precision)"

	if (!(q = db_call(query_new)(createsql))) {
		err_error("cannot create query");
		db_call(destroy)();
		return 3;
	}
	q = db_call(query_new)("insert into t1 values ('zero',  0, 0, 0, 0, 0.0, 0.0)"); 
	db_call(query_destroy)(q);
	q = db_call(query_new)("insert into t1 values ('one',   1,  1, 1, 1, 1.1, 1.1)"); 
	db_call(query_destroy)(q);
	q = db_call(query_new)("insert into t1 values ('two',   1,  2, 2, 2, 2.2, 2.2)"); 
	db_call(query_destroy)(q);
	q = db_call(query_new)("insert into t1 values ('three', 1,  3, 3, 3, 3.3, 3.3)"); 
	db_call(query_destroy)(q);
	q = db_call(query_new)("insert into t1 values ('four',  1,  4, 4, 4, 4.4, 4.4)"); 
	db_call(query_destroy)(q);
	q = db_call(query_new)("insert into t1 values ('five',  1,  5, 5, 5, 5.5, 5.5)"); 
	db_call(query_destroy)(q);
	q = db_call(query_new)("insert into t1 values ('six',   1,  6, 6, 6, 6.6, 6.6)"); 
	db_call(query_destroy)(q);

	q = db_call(query_new)("select * from t1"); 

	r = 0;
	while (db_call(query_row_next)(q)) {
		char *name;
		if (r == 0) {
			int i;
			printf("Columns are:\n");
			for (i = 0; i <= 6; i++) {
				printf("%d: name = %s type = %-10s (%d) size = %2d\n", 
					i,
					db_call(query_col_name)(q, i),
					db_call(type_names)[db_call(query_col_type)(q, i)],
					db_call(query_col_type)(q, i),
					db_call(query_col_size)(q, i));
			}	

			printf("\nquery_col_by_name(q, \"name\") = \"%s\"\n", (char *) (name = db_call(query_col_by_name)(q, "name")));
			free(name);
			printf("\nDump t1 table:\n");
			r++;
		}	

		printf("%s;%d;%d;%d;%d;%f;%f\n",
			name =  (char   *) db_call(query_col)(q, 0),
			* (char   *) db_call(query_col)(q, 1),
			* (short  *) db_call(query_col)(q, 2),
			* (int    *) db_call(query_col)(q, 3),
			* (long   *) db_call(query_col)(q, 4),
			* (float  *) db_call(query_col)(q, 5),
			* (double *) db_call(query_col)(q, 6)
		);
		free(name);
	}
	db_call(query_destroy)(q);
	db_call(destroy)();
	return 0;
}

