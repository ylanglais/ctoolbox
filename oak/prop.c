

typedef struct {
	oak_id_t 	mod_id;
	oak_id_t 	id;
	oak_name_t 	name;
	oak_dsc_t  	dsc;
	oak_type_t	datatype;
	oak_type_t	subdatatype;
	oak_size_t  size;
	oak_bool_t	iskey;
	oak_bool_t	unique;
	oak_bool_t	not_null;

	oak_ref_t	ref;
	oak_prt_t   defval;
	oak_link_t 	datalink;
	
	oak_grp_t   
} prop_t, *pprop_t;

pprop_t
prop_new() {
}

pprop_t
prop_destroy(pprop_t f) {
}

/* load => type init */

oak_ui_t
prop_ui_ro(pprop_t f) {
}

oak_ui_t
prop_ui(pprop_t f) {
}

oak_ui_t
prop_sub_ui(pprop_t f) {
}

oak_ui_t
prop_ui_hidden(pprop_t f) {
}

prop_ui_read(oak_ui_t ui, pprop_t f) {
}

prop_to_str() {
}

prop_to_html() {
}

prop_vidate() {
}
