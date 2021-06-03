#ifndef _block_h_
#define _block_h_
    
#ifndef _block_c_
typedef void *pbhead_t;
#endif

size_t    block_unit_get(pbheadd_t b);
size_t    block_mapsize_get(pbheadd_t b);
size_t    block_nused_get(pbhead_t b);
size_t    block_nfree_get(pbhead_t b);
size_t    block_nslot_get(pbhead_t b);
void     *block_from_get(pbhead_t  b);
void     *block_to_get(pbhead_t    b);

pbhead_t  block_init(char *b, size_t unit) { 
void     *block_slot_alloc(pbhead_t b) {
int       block_slot_in_use(pbhead_t b, void *p) {
int       block_slot_free(pbhead_t b, void *p) {

#endif

