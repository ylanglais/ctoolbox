
typedef struct {
	long	 magic;
	size_t	 grain;	
	size_t   size;
	long	 used;
	long     allocated;
	long	 free;
} mabhdr_t, *pmabhdr_t;
	slot_t 	 freestack[];

typedef struct {
	int 	ncustom;
	pmab_t  *bcustom;
} masb_t, *pmasb_t;

void
ma_init() {
}

void
ma_exit() {
}

ppages_t
ma_pages_destroy(ppages_t p) {
}

ppages_t 
ma_pages_new(int num) {
	
}
pmab_t
ma_block_new(pmasb_t sb, size_t size, size_t grain) {
}

pmab_t
ma_block_destroy(psasb_t sb, pmab_t b) {
}


pmabs_t
ma_bspecial_new(pmasb_t sb, size_t size) {

}

pmabs_t 
ma_bspecial_destroy(pmabsb_t sb, pmabs_t bs) {
}

void *
ma_malloc(size_t size) {
	/* check required size: */
	
}


