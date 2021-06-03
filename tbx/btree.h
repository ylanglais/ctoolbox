#ifndef __btree_h__
#define __btree_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __btree_c__
typedef int (*comp_t)(void *, void *);
typedef void (*print_t)(void *);
typedef struct _btree_t_ *pbtree_t;
#endif

size_t    btree_sizeof();

/* Constructor/Destructor: */
pbtree_t  btree_new(comp_t comp);
pbtree_t  btree_destroy(pbtree_t pb);

/* set/get: */
void     *btree_store(pbtree_t pb, void *data);
void     *btree_get(pbtree_t pb, void *data);

/* navigate: */
void     *btree_goto(pbtree_t pb, void *data);
void     *btree_leftmost(pbtree_t pb);
void     *btree_rightmost(pbtree_t pb);
void     *btree_left(pbtree_t pb);
void     *btree_right(pbtree_t pb);
void     *btree_parent(pbtree_t pb);
void     *btree_prev(pbtree_t pb);
void     *btree_next(pbtree_t pb);

/* Maintenance: */
void     *btree_save(pbtree_t pb);
void     *btree_restore(pbtree_t pb, void *mark);

/* Foreach */
void      btree_foreach_raw(pbtree_t pb,  void *userdata, void (*userf)(void *, void *));
void      btree_foreach(pbtree_t pb,      void *userdata, void (*userf)(void *, void *));
void      btree_foreach_desc(pbtree_t pb, void *userdata, void (*userf)(void *, void *));

#ifdef __cplusplus
}
#endif

#endif
