

typedef struct _argument_t {
	oak_name_t  name;
	oak_dsc_t   dsc
	oak_type_t  type;
} argument_t, *pargument_t;


typedef struct _method_t {
	oak_name_t name;
	oak_dsc_t  dsc;
	oak_prt_t  function_ptr;

} method_t, *pmethod_t;
