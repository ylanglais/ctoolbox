#ifndef _hds_h_
#define _hds_h_

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
	char 		       hds_char;
	unsigned char      hds_uchar;
	int  		       hds_int;
	unsigned int       hds_uint;
	long		       hds_long;
	unsigned long	   hds_ulong;
	long long		   hds_longlong;
	unsigned long long hds_ulonglong;
	float			   hds_float;
	double			   hds_double;
	void *			   hds_ptr;
} hds_any_t;

typedef enum {
	hdsTYPE_NULL,
	hdsTYPE_PTR,
	hdsTYPE_CHAR,
	hdsTYPE_UCHAR,
	hdsTYPE_INT,
	hdsTYPE_UINT,
	hdsTYPE_LONG,
	hdsTYPE_ULONG,
	hdsTYPE_LONGLONG,
	hdsTYPE_ULONGLONG,
	hdsTYPE_FLOAT,
	hdsTYPE_DOUBLE,	
	hdsTYPE_OBJECT,
	hdsTYPE_ARRAY
} hds_type_t;

#ifndef _hds_c_
typedef void *phds_t;
#endif

char *     hds_error_string(hds_error_code_t errcode);
size_t     hds_sizeof();

phds_t     hds_new();
phds_t     hds_destroy(phds_t);

int        hds_put(phds_t h, char *path, hds_type_t  type, hds_any_t  data);
int        hds_get(phds_t h, char *path, hds_type_t *type, hds_any_t *data);

hds_any_t  hds_data_get(phds_t h, char *path);
hds_type_t hds_type_get(phds_t h, char *path);

#ifdef __cplusplus
}
#endif

#endif
