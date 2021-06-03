#ifndef _tree_h_
#define _tree_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _tree_c_
/* ptree_t is private: */
typedef void *ptree_t;
#endif

size_t   tree_sizeof();
ptree_t  tree_new();
ptree_t  tree_destroy(ptree_t t);

/* navigation: */
int      tree_children_count(ptree_t t);

void    *tree_current(ptree_t t);
void    *tree_root(ptree_t t);

void    *tree_down(ptree_t t);
void    *tree_parent(ptree_t t);
void    *tree_up(ptree_t t);
void    *tree_child(ptree_t t);

void    *tree_child_first(ptree_t t);
void    *tree_child_next(ptree_t t);
void    *tree_child_prev(ptree_t t);
void    *tree_child_last(ptree_t t);

void    *tree_brother_first(ptree_t t);
void    *tree_brother_next(ptree_t t);
void    *tree_brother_prev(ptree_t t);
void    *tree_brother_last(ptree_t t);

/* save/restore position: */
void    *tree_pos_save(ptree_t t);
void    *tree_pos_restore(ptree_t t, void *pos);

/* add/insert/delete: */
void    *tree_child_add(ptree_t t, void *data);
void    *tree_child_insert(ptree_t t, void *data);
void    *tree_child_del(ptree_t t);

/* Foreach: */
int      tree_foreach(ptree_t t, void *param, void (*userf)(void *, int, int, void *));

/* reparent: */
void    *tree_subtree_add(ptree_t dest, ptree_t child);
void    *tree_subtree_insert(ptree_t dest, ptree_t child);
ptree_t  tree_subtree_cut(ptree_t t);
ptree_t  tree_subtree_destroy(ptree_t t);
int      tree_subtree_foreach(ptree_t t, void *param, void (*userf)(void *, int, int, void *));

#ifdef __cplusplus
}
#endif

#endif
