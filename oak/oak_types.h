#ifndef _oak_types_h_
#define _oak_types_h_

#define oakNAMESIZE 60
#define oakDSCSIZE  60
#define oakFMTSIZE  60

#define oakFALSE	  0
#define oakTRUE		  1
p

typedef void *        oak_ptr_t;

typedef unsigned long oak_id_t;
typedef char          oak_name_t[oakNAMESIZE + 1];
typedef char 		  oak_dsc_t[oakDSCSIZE   + 1];
typedef char 		  oak_fmt_t[oakFMTSIZE   + 1];
typedef unsigned long oak_any_t;
typedef char *		  oak_dlink_t;
typedef char          oak_byte_t;
typedef unsigned char oak_bool_t;
typedef short         oak_word_t;
typedef int			  oak_int_t;
typedef long          oak_long_t;
typedef char *		  oak_link_t;


typedef unsigned int  oak_type_t;			  





#endif
