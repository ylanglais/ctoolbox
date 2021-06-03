;
/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
	24/01/2018	 1.0 Creation
*/   

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>

#include <tbx/err.h>


#define _gdb_c_
#define  pgdb_t 	struct _pgdb_t *
#define  pquery_t 	struct _pquery_t *
#include "gdb.h"
#include "gdb_defs.h"
#undef   pgdb_t
#undef   pquery_t
#undef _gdb_c_

char gdb_NAME[]    = "Generic Database access";
char gdb_VERSION[] = "1.0";
char gdb_DATEVER[] = "24/01/2018";


typedef struct _pgdb_t {
	void      *engine;
	gdbtype_t  type;	
	gdbdrv_t   drv;
} gdb_t, *pgdb_t;

typedef struct _pquery_t {
	gdbdrv_t	drv;
} query_t, *pquery_t;

static gdberrcodes_t gdblast_error_ = gdbNOERROR;

gdbtype_t _gdbtypes_[] = { gdbPOSTGRES, gdbMYSQL, gdbODBC, gdbUNKNOWN };

char *_drv_libs_[] = {
	"libgdb_pgsql.so",         
	"libgdb_mysql.so",       
	"libgdb_odbc.so" 
};

char *_gdbprefix_[]  = {
	"pgsql",         
	"mysql",       
	"odbc" ,
	"None"
};
char *_gdbdrvname_[] = {
	"PostgreSQL", 
	"MySQL",         
	"ODBC",
	"None"
};

/*
static char *gdbtype_str[] = {
#ifdef HAS_POSTGRESL
	"postgresql",
#endif
#ifdef HAS_MYSQL
	"mysql",
#endif
#ifdef HAS_ODBC
	"odbc",
#endif
	"undef"
};
*/

static char *_gdb_error_strings[] = {
	"No error",
	"Cannot load driver",
	"Cannot resolv driver entry point",
	"No valid gdb connection",
	"Already connected",
	"No memory",
	"Invalid login",
	"Invalid password",
	"Incorrect login/password pair",

	"Cannot allocate ODBC environment handle",
	"Cannot set ODBC environment",
	"Cannot allocate ODBC handle",

	"Cannot connect to gdb server",
	"Cannot create query",
	"Bad statement",
	
	"Invalid error code"
};

char *
gdb_error_string(gdberrcodes_t i) {
	if (i < gdbNOERROR || i >= gdbNOMORE)  return _gdb_error_strings[gdbNOMORE];
	return _gdb_error_strings[i];
}

#define gdb_resolv(name) if (!(gdb->drv.drv_##name = (name##_f) gdb_dlsym(_gdbprefix_[type], #name))) { gdb_destroy(); err_error("cannot resolve %s reason: %s", #name, dlerror()); return gdblast_error_ = gdbCANNOTRESOLV; }

#if 0
static void *
_gdb_dlsym(const char prefix[], const char *function) {
	char b[150];
	if (!gdb || !gdb->engine) return NULL;
	sprintf(b, "%s_%s", prefix, function);
	return dlsym(gdb->engine, b);
}
#endif

/*
static char *
_gdb_type_to_name(gdbtype_t type) {
	if (type < gdbPOSTGRES || type >= gdbUNKNOWN) {
		return gdbtype_str[gdbUNKNOWN];
	}
	return gdbtype_str[type];
}
*/
/*
static gdbtype_t 
_gdb_name_to_type(char *name) {
	int i;
	for (i = 0; i < gdbUNKNOWN; i++) {
		if (!strcmp(name, gdbtype_str[i])) return i;
	}
	return gdbUNKNOWN;
}
*/

gdbdrv_t gdb_driver(pgdb_t gdb) {
	gdbdrv_t drv0;

	if (!gdb) {
		memset(&drv0, 0, sizeof(gdbdrv_t));
		return drv0;
	}
	return gdb->drv;
}

pgdb_t
gdb_new(gdbtype_t engine, char *server, int port, char *login, char *passwd, char *gdbname) {
	pgdb_t gdb;

	if (!(gdb = (pgdb_t) malloc(sizeof(gdb_t)))) {
		err_error("No memory");
		return NULL;
	}

	
	gdbtype_t dbtype = engine;

/***
	gdbtype_t dbtype = gdbUNKNOWN;

	int i;
	for (i = 0; strcmp(_gdbprefix_[i], "None"); i++) {
		if (!strcasecmp(_gdbprefix_[i], engine)) {
			dbtype = _gdbtypes_[i];
			break;
		}
	} 
	if (dbtype == gdbUNKNOWN) {
		for (i = 0; strcmp(_gdbdrvname_[i], "None"); i++) {
			if (!strcasecmp(_gdbdrvname_[i], engine)) {
				dbtype = _gdbtypes_[i];
				break;
			}
		}
	} 
***/
	if (dbtype == gdbUNKNOWN) {
		err_error("unkown engine type (%s)", engine);
		return NULL;
	}

	if (!(gdb->engine = dlopen(_drv_libs_[dbtype], RTLD_NOW))) {
		err_error("cannot load %s driver", engine);
		return gdb_destroy(gdb);
	}
	
	char tmp[500];

	sprintf(tmp, "gdb_%s_driver", _gdbprefix_[dbtype]);
		
	gdbdrv_t (*drv_get)();

	if (!(drv_get = (gdbdrv_t (*)()) dlsym(gdb->engine, tmp))) {
		err_error("cannot find %s entry point in %s", tmp, _drv_libs_[dbtype]);
		exit(1);
	}	

	gdb->drv = drv_get(); 

#if 0
	/* Resolve entry points: */
	gdb_resolv(new);
	gdb_resolv(destroy);
	gdb_resolv(error);
	gdb_resolv(query_new);
	gdb_resolv(query_destroy);
	gdb_resolv(query_row_next);
	gdb_resolv(query_col);
	gdb_resolv(query_col_id);
	gdb_resolv(query_col_by_name);
	gdb_resolv(query_col_size);
	gdb_resolv(query_col_name);
	gdb_resolv(query_col_type);
#endif

	/* call driver's constructor: */
	if (!(gdb->engine =  gdb->drv.drv_new(server, port, login, passwd, gdbname)))
		return gdb_destroy(gdb);
	return gdb;
}

pgdb_t 
gdb_destroy(pgdb_t gdb) {
	if (!gdb) return NULL;
	if (gdb->engine) {
		gdb->drv.drv_destroy(gdb);
		dlclose(gdb->engine);	
		gdb->engine = NULL;
	}
	memset(&gdb->drv, 0, sizeof(gdbdrv_t));
	free(gdb);
	return NULL;
}

/* Return last gdbsql error code and reset error code to gdbNOERROR */
int
gdb_error(pgdb_t gdb) {
	if (!gdb || !gdb->engine) {
		gdblast_error_ = gdbNOERROR;
		return gdblast_error_;
	} 
	return gdb->drv.drv_error(gdb);
}

/* Query execution & free: */
pquery_t
gdb_query_new(pgdb_t gdb, char *sql) {
	if (!gdb|| !gdb->engine) return NULL;
	return gdb->drv.drv_query_new(gdb, sql);
}
pquery_t
gdb_query_destroy(pquery_t q) {
	if (!q) return NULL;
	return q->drv.drv_query_destroy(q);
}

/* Fetch next row: */
int
gdb_query_row_next(pquery_t q) {
	if (!q) return 0;
	return q->drv.drv_query_row_next(q);
}

/* Get column (value or new allocated pointer to char): */
any_t
gdb_query_col(pquery_t q, int colnum) {
	if (!q) {
		any_t empty = { .type = gdbUNDEF, .size = 0, .mustfree = 0 };
		return empty;
	}
	return q->drv.drv_query_col(q, colnum);
}

any_t
gdb_query_col_by_name(pquery_t q, char *name) {
	if (!q) {
		any_t empty = { .type = gdbUNDEF, .size = 0, .mustfree = 0 };
		return empty;
	}
	return q->drv.drv_query_col_by_name(q, name);
}

/* Other optional stuff: */
size_t
gdb_query_col_size(pquery_t q, int colnum) {
	if (!q) return 0;
	return q->drv.drv_query_col_size(q, colnum);
}

char *
gdb_query_col_name(pquery_t q, int colnum) {
	if (!q) return NULL;
	return q->drv.drv_query_col_name(q, colnum);
}

gdb_datatype_t
gdb_query_col_type(pquery_t q, int colnum) {
	if (!q) return gdbUNDEF;
	return q->drv.drv_query_col_type(q, colnum);
}
 
#ifdef _test_gdb_

#define gdb_call(func) gdb##_##func

#include "gdb_test.c"

#include "gdb_test.conf"

int main(int n, char *a[]) {
	gdb_test(gdbMYSQL,    "localhost", dbport, dblogin, dbpasswd, dbname);
	gdb_test(gdbPOSTGRES, "localhost", dbport, dblogin, dbpasswd, dbname);
/*
	gdb_test(gdbODBC,     "pg_test",   dbport, dblogin, dbpasswd, dbname);
	gdb_test(gdbODBC,     "my_test",   dbport, dblogin, dbpasswd, dbname);
*/
	return 0;
}

#endif
