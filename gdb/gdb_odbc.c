#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <bits/byteswap.h>

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#define ODCHAR 	  	SQL_CHAR
#define ODVARCHAR 	SQL_VARCHAR
#define ODBOOL 		SQL_TINYINT
#define ODBOOL2 	SQL_BIT
#define ODINT2		SQL_SMALLINT
#define ODINT4		SQL_INTEGER
#define ODINT8		SQL_BIGINT
#define ODNUMERIC   SQL_NUMERIC
#define ODFLOAT4	SQL_REAL
#define ODFLOAT8	SQL_DOUBLE
#define ODDOUBLE	SQL_FLOAT

#include <tbx/err.h>


const char __db_prefix__[] = "od"; 

char _tmp_name[101];

typedef struct {
	SQLHENV	env;	
	SQLHDBC hdbc;
	int 	debug_level;
	gdberrcodes_t  last_error;
} gdb_odbc_t, *pgdb_odbc_t;

typedef struct {
	SQLHSTMT hstmt;	
	int		 currow;
} gdb_odbc_query_t, *pgdb_odbc_query_t;

gdbdrv_t
gdb_odbc_driver() {
	gdbdrv_t drv;
	drv.drv_new               = gdb_odbc_new;
	drv.drv_destroy           = gdb_odbc_destroy;
	drv.drv_error             = gdb_odbc_error;
	drv.drv_query_new         = gdb_odbc_query_new;
	drv.drv_query_destroy     = gdb_odbc_query_destroy;
	drv.drv_query_row_next    = gdb_odbc_query_row_next;
	drv.drv_query_col         = gdb_odbc_query_col;
	drv.drv_query_col_id      = gdb_odbc_query_col_id;
	drv.drv_query_col_by_name = gdb_odbc_query_col_by_name;
	drv.drv_query_col_size    = gdb_odbc_query_col_size;
	drv.drv_query_col_name    = gdb_odbc_query_col_name;
	drv.drv_query_col_type    = gdb_odbc_query_col_type;

	return drv;
}


/* Return last od error code and reset error code to gdbNOERROR */
int
gdb_odbc_error(pgdb_t db) {
	pgdb_odbc_t gdb_odbc = (pgdb_odbc_t) db;
	if (!gdb_odbc) return gdbNOERROR;
	int r;
	r = gdb_odbc->last_error;
	gdb_odbc->last_error = gdbNOERROR;
	return r;
}

pgdb_t
gdb_odbc_destroy(pgdb_t db) {
	pgdb_odbc_t gdb_odbc = (pgdb_odbc_t) db;
	if (!gdb_odbc) return NULL;
	if (gdb_odbc->hdbc) {
		SQLDisconnect(gdb_odbc->hdbc);
		SQLFreeHandle(SQL_HANDLE_DBC, gdb_odbc->hdbc);
		gdb_odbc->hdbc = NULL;
	}
	if (gdb_odbc->env) {
		SQLFreeHandle(SQL_HANDLE_ENV, gdb_odbc->env);
		gdb_odbc->env = NULL;
	}
	free(gdb_odbc);
	gdb_odbc = NULL;
	return NULL;
}

pgdb_t
gdb_odbc_new(char *server, int port, char *dbname, char *login, char *passwd) {
	char *dbglvl;
	long erg;

	pgdb_odbc_t gdb_odbc

	if (!login || strlen(login) < 2) {
		err_error("gInvalid login");
		return NULL;	
	}

	if (!(gdb_odbc = (pgdb_odbc_t) malloc(sizeof(gdb_odbc_t)))) {
		err_error("gNo memory");
		return NULL;
	}

	if ((dbglvl = getenv("ODDBG"))) {
		switch (*dbglvl) {
		case 1: // err_MESSAGE
			gdb_odbc->debug_level = err_MESSAGE;	
			break;
		case 2: // err_ERROR
			gdb_odbc->debug_level = err_ERROR;	
			break;
		case 3: // err_WARN
			gdb_odbc->debug_level = err_WARNING;	
			break;
		case 4: // err_INFO
			gdb_odbc->debug_level = err_INFO;	
			break;
		case 5: // err_DEBUG
			gdb_odbc->debug_level = err_DEBUG;	
			break;
		default: // err_NONE (keep quiet)
			gdb_odbc->debug_level = err_NONE;	
			break;
		}	
	}
	memset(gdb_odbc, 0, sizeof(gdb_odbc_t));

	// 1. allocate Environment handle and register version 
	erg = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &gdb_odbc->env);

	if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
		err_error("gCannot allocate env handle"); 
		return gdb_odbc_destroy(gdb_odbc);	
	}

	erg = SQLSetEnvAttr(gdb_odbc->env, SQL_ATTR_ODBC_VERSION, (void*) SQL_OV_ODBC3, 0); 

	if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
		err_error("gCannot set env");
		return gdb_odbc_destroy(gdb_odbc);	
	}

	// 2. allocate connection handle, set timeout
	erg = SQLAllocHandle(SQL_HANDLE_DBC, gdb_odbc->env, &gdb_odbc->hdbc); 
	if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
		err_error("gCannot allocate handle");
		return gdb_odbc_destroy(gdb_odbc);	
	}
	SQLSetConnectAttr(gdb_odbc->hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER *) 5, 0);

	// 3. Connect to the datasource "web" 
	erg = SQLConnect(gdb_odbc->hdbc, (SQLCHAR *) server, SQL_NTS, (SQLCHAR *) login, SQL_NTS, (SQLCHAR *) passwd, SQL_NTS);

	if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
		char stat[10], msg[200];	
		int  err, mlen;

		SQLGetDiagRec(SQL_HANDLE_DBC, gdb_odbc->hdbc, 1, (SQLCHAR *) stat, (SQLINTEGER *) &err, (SQLCHAR *) msg, 100, (SQLSMALLINT *) &mlen);
		err_error("gCannot connect (%d return code) : %s (%d)", erg, msg, err);

		return gdb_odbc_destroy(gdb_odbc);	
	}

	return (pgdb_t) gdb_odbc;
}

pquery_t
gdb_odbc_query_destroy(pquery_t query) {
	pgdb_odbc_query_t q = (pgdb_odbc_query_t) query;
	if (q) { 
		if (q->hstmt) { 
			SQLFreeHandle(SQL_HANDLE_STMT, q->hstmt);
			q->hstmt = NULL;
		}
		free(q);
	}
	return NULL;
}

pquery_t 
gdb_odbc_query_new(pgdb_t db, char *sql) {
	pgdb_odbc_t gdb_odbc = (pgdb_odbc_t) db;
	pgdb_odbc_query_t q;
	long erg;
	int err;
	char msg[100], stat[10];

	if (!gdb_odbc) {
		gdb_odbc->last_error = gdbNOCONNECTION;
		err_error("No connection");
		return NULL;
	}
	if (!(q = (pgdb_odbc_query_t) malloc(sizeof(gdb_odbc_query_t)))) {
		gdb_odbc->last_error = gdbNOMEMORY;
		err_error("No memory");
		return NULL;
	}

	erg = SQLAllocHandle(SQL_HANDLE_STMT, gdb_odbc->hdbc, &q->hstmt);
	if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
		int mlen;

		SQLGetDiagRec(SQL_HANDLE_DBC, gdb_odbc->hdbc, 1, (SQLCHAR *) stat, (SQLINTEGER *) &err, (SQLCHAR *) msg, 100, (SQLSMALLINT *) &mlen);
		err_error("%s (%d)\n", msg, err);
		gdb_odbc->last_error = gdbCANNOTCREATEQUERY;
		return gdb_odbc_query_destroy(q);
	}

	erg = SQLExecDirect(q->hstmt, (SQLCHAR *) sql, SQL_NTS);   
    if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
		char stat[10], msg[200];	
		int  err, mlen;

		SQLGetDiagRec(SQL_HANDLE_STMT, q->hstmt, 1, (SQLCHAR *) stat, (SQLINTEGER *) &err, (SQLCHAR *) msg, 100, (SQLSMALLINT *) &mlen);
		err_error("%s (%d)\n", msg, err);
		gdb_odbc->last_error = gdbCANNOTCREATEQUERY;
		return gdb_odbc_query_destroy(q);
    }

	q->currow = -1;
	return (pquery_t) q;
}

int
gdb_odbc_query_row_next(pquery_t query) {
	pgdb_odbc_query_t q = (pgdb_odbc_query_t) query;
	if (!q || !q->hstmt) return 0;
	++q->currow;

	if (SQLFetch(q->hstmt) != SQL_NO_DATA) return 1;

	SQLFreeHandle(SQL_HANDLE_STMT, q->hstmt);
	q->hstmt = NULL;
	
	return 0;
}

any_t
gdb_odbc_query_col(pquery_t query, int colnum) {
	pgdb_odbc_query_t q = (pgdb_odbc_query_t) query;
	any_t any = { .type = gdbUNDEF, .size = 0, .mustfree = 0 };

	SQLSMALLINT type, digits, nullable;
	SQLULEN colsize;
	SQLSMALLINT n;
	SQLLEN rc;
	char *map;
	char name[101];
	SQLSMALLINT len, rlen;
	long erg;
	

	if (!q || !q->hstmt) 
		return any;

	erg = SQLRowCount(q->hstmt, &rc);
	if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
		char stat[10], msg[200];	
		int  err, mlen;

		SQLGetDiagRec(SQL_HANDLE_STMT, q->hstmt, 1, (SQLCHAR *) stat, (SQLINTEGER *) &err, (SQLCHAR *) msg, 100, (SQLSMALLINT *) &mlen);
		err_error("error getting row count(returned %d): %s (%d)", erg, msg, err);
	}

	if (q->currow < 0 ) // || q->currow >= rc) 
		return any;

	/* ODBC colunms start at 1 !!!! */
	++colnum;

	erg = SQLNumResultCols(q->hstmt, &n);
	if (colnum <= 0 || colnum > n)
		return any;

	erg = SQLDescribeCol(q->hstmt, (SQLSMALLINT) colnum, (SQLCHAR *) name, 100, &len, &type, &colsize, &digits, &nullable);
	if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
		char stat[10], msg[200];	
		int  err, mlen;

		SQLGetDiagRec(SQL_HANDLE_STMT, q->hstmt, 1, (SQLCHAR *) stat, (SQLINTEGER *) &err, (SQLCHAR *) msg, 100, (SQLSMALLINT *) &mlen);
		err_error("error getting description of column %d (returned %d): %s (%d)", colnum - 1, erg, msg, err);
	}

	switch (type) {
	case ODCHAR:
	case ODVARCHAR:
		any.must_free = 1;
		any.type      = gdbSTRING;
		any.size      = colsize;
		any.val._string = malloc(colsize + 1);
		erg = SQLGetData(q->hstmt, colnum, SQL_C_CHAR,    any.val._string, colsize, (SQLLEN *) &rlen); 
		break;

	case ODBOOL:
	case ODBOOL2:

		any.mustfree = 0;
		any.type     = gdbBOOL;
		any.size     = sizeof(char);
		erg = SQLGetData(q->hstmt, colnum, SQL_C_TINYINT, &(any.val._bool), 1,          (SQLLEN *) &rlen); 
		break;

	case ODINT2:
		any.mustfree = 0;
		any.type     = gdbINT2;
		any.size     = sizeof(short);

		erg = SQLGetData(q->hstmt, colnum, SQL_C_SHORT,   &(any.val._int2),   colsize, (SQLLEN *) &rlen); 
		break;

	case ODINT4:
		any.mustfree = 0;
		any.type     = gdbINT4;
		any.size     = sizeof(int);
		erg = SQLGetData(q->hstmt, colnum, SQL_C_LONG,    &(any.val._int4),   colsize, (SQLLEN *) &rlen); 
		break;

	case ODNUMERIC:
	case ODINT8:
		any.mustfree = 0;
		any.type     = gdbINT8;
		any.size     = sizeof(long long);
		erg = SQLGetData(q->hstmt, colnum, SQL_C_LONG,    &(any.val._int8),   colsize, (SQLLEN *) &rlen); 
		break;

	case ODFLOAT4:
		any.mustfree = 0;
		any.type     = gdbFLOAT4;
		any.size     = sizeof(float);
		erg = SQLGetData(q->hstmt, colnum, SQL_C_FLOAT,   &(any.val._float4), colsize, (SQLLEN *) &rlen); 
		break;

	case ODDOUBLE:
	case ODFLOAT8:
		any.mustfree = 0;
		any.type     = gdbFLOAT8;
		any.size     = sizeof(double);
		erg = SQLGetData(q->hstmt, colnum, SQL_C_DOUBLE,  &(any.val._float8), colsize, (SQLLEN *) &rlen); 
		break;

	default:
		err_error(">>> column %d: unsopported datatype = %hd returned as STRING", colnum - 1, type);
		any.mustfree = 1;
		any.type     = gdbUNDEF;
		any.val._string = malloc(colsize + 1);
		erg = SQLGetData(q->hstmt, colnum, SQL_C_CHAR,    any.val._string,    colsize, (SQLLEN *) &rlen); 

		break;
	}

	if ((erg != SQL_SUCCESS) && (erg != SQL_SUCCESS_WITH_INFO)) {
		char stat[10], msg[200];	
		int  err, mlen;

		SQLGetDiagRec(SQL_HANDLE_STMT, q->hstmt, 1, (SQLCHAR *) stat, (SQLINTEGER *) &err, (SQLCHAR *) msg, 100, (SQLSMALLINT *) &mlen);
		err_error("error getting data for column %d (returned %d): %s (%d)", colnum - 1, erg, msg, err);
	}

	return (void *) map;
}

int
gdb_odbc_query_col_id(pquery_t query, char *name) {
	pgdb_odbc_query_t q = (pgdb_odbc_query_t) query;
	int i;
	SQLSMALLINT n;
	SQLSMALLINT type, digits, nullable, len;
	SQLULEN colsize;
	char nname[101];

	if (!q || !q->hstmt )            return -1;
	if (!name || strlen(name) < 2)  return -2;

	SQLNumResultCols(q->hstmt, &n);

	for (i = 1; i <= n; i++) {
		SQLDescribeCol(q->hstmt, (SQLSMALLINT) i, (SQLCHAR *) nname, 100, &len, &type, &colsize, &digits, &nullable);
		if (!strcmp(name, nname)) 
			return i - 1;
	}
	return -3;
}

any_t
gdb_odbc_query_col_by_name(pquery_t query, char *name) {
	pgdb_odbc_query_t q = (pgdb_odbc_query_t) query;
	any_t any = { .type = gdbUNDEF, .size = 0, .mustfree = 0 };
	int i;
	SQLSMALLINT n;
	if (!q || !q->hstmt) 
		return any;

	SQLNumResultCols(q->hstmt, &n);

	if (q->currow < 0 || q->currow >= n)
		return any;

	if (!name || strlen(name) < 2) return any;

	if ((i = gdb_odbc_query_col_id(q, name)) > -1) 
		return gdb_odbc_query_col(q, i);

	return any;
}

size_t
gdb_odbc_query_col_size(pgdb_odbc_query_t q, int colnum) {
	pgdb_odbc_query_t q = (pgdb_odbc_query_t) query;
	SQLSMALLINT n;
	SQLSMALLINT type, digits, nullable, len;
	SQLULEN colsize;
	char nname[101];

	if (!q || !q->hstmt) return 0;
	SQLNumResultCols(q->hstmt, &n);

	/* ODBC colunms start at 1 !!!! */
	colnum++;

	if (colnum <= 0 || colnum > n) return 0;

	SQLDescribeCol(q->hstmt, colnum, (SQLCHAR *) nname, 100, &len, &type, &colsize, &digits, &nullable);
	return len;
}

char *
gdb_odbc_query_col_name(pgdb_odbc_query_t q, int colnum) {
	SQLSMALLINT n;

	if (!q) return NULL;

	SQLNumResultCols(q->hstmt, &n);
	/* ODBC colunms start at 1 !!!! */
	colnum++;
	if (colnum <= 0 || colnum > n) 
		return NULL;

	SQLSMALLINT type, digits, nullable, len;
	SQLULEN colsize;

	SQLDescribeCol(q->hstmt, colnum, (SQLCHAR *) _tmp_name, 100, &len, &type, &colsize, &digits, &nullable);

	return _tmp_name;
}

db_datatype_t
gdb_odbc_query_col_type(pgdb_odbc_query_t q, int colnum) {
	SQLSMALLINT n, type, digits, nullable, len;
	SQLULEN colsize;
	char nname[101];
	if (!q) return gdbUNDEF;

	SQLNumResultCols(q->hstmt, &n);
	/* ODBC colunms start at 1 !!!! */
	colnum++;
	if (colnum <= 0 || colnum > n) return gdbUNDEF;

	SQLDescribeCol(q->hstmt, colnum, (SQLCHAR *) nname, 100, &len, &type, &colsize, &digits, &nullable);

	switch (type) {
	case ODCHAR:
	case ODVARCHAR:
		return gdbSTRING;

	case ODBOOL:
	case ODBOOL2:
		return gdbBOOL;	

	case ODINT2:
		return gdbINT2;

	case ODINT4:
		return gdbINT4;

	case ODNUMERIC:
	case ODINT8:
		return gdbINT8;

	case ODFLOAT4:
		return gdbFLOAT4;

	case ODDOUBLE:
	case ODFLOAT8:
		return gdbFLOAT8;

	default:
		break;
	}
	return gdbUNDEF;
}

#ifdef _testgdb_odbc

#include "gdb_test.conf"

#define db_call(func) od##_##func

#include "gdb_test.c"

int
main(int n, char *a[]) {
	int i;

	if (n < 2) {
		db_test(dbODBC, "pg_test", dbport, dblogin, dbpasswd, dbname);
		db_test(dbODBC, "my_test", dbport, dblogin, dbpasswd, dbname);
	} else {
		for (i = 1; i < n; i++) db_test(dgbODBC, a[i], dbport, dblogin, dbpasswd, dbname);
	}
	return 0;
}

#endif
