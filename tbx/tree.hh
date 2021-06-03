#ifndef _Tree_h_
#define _Tree_h_

#include <tree.h>

class Tree {
private: 
	treeree;

public: 
	Tree()  { tree = tree_new(); }
	Tree(ptree_t t) { tree = t; }
	~Tree() { tree = tree_destroy(tree); }

	int  children_count() { return tree_children_count(tree); }

	/* navigation: */
	void * current() { return tree_current(tree); }
	void * root()    { return tree_root(tree);    }
	void * down()    { return tree_down(tree);    }
	void * parent()  { return tree_parent(tree);  }
	void * up()      { return tree_up(tree);      }
	void * child()   { return tree_child(tree);   }

	void * child_first() { return tree_child_first(tree); }
	void * child_next()  { return tree_child_next(tree);  }
	void * child_prev()  { return tree_child_prev(tree);  }
	void * child_last()  { return tree_child_last(tree);  }

	void * brother_first() { return tree_brother_first(tree); }
	void * brother_next()  { return tree_brother_next(tree);  }
	void * brother_prev()  { return tree_brother_prev(tree);  }
	void * brother_last()  { return tree_brother_last(tree);  } 

	/* save/restore position: */
	void * pos_save()             { return tree_pos_save(tree);         }
	void * pos_restore(void *pos) { return tree_pos_restore(tree, pos); }

	/* add/insert/delete: */
	void * child_add(void *data)    { return tree_child_add(tree, data);    }
	void * child_insert(void *data) { return tree_child_insert(tree, data); }
	void * child_del()              { return tree_child_del(tree);          }

	/* Foreach: */
	int   foreach(void *param, void (*userf)(void *, int, int, void *)) {
		return tree_foreach(tree, param, userf);
	}

	/* Reparent: */
	void * subtree_add(Tree child)    { return tree_subtree_add(tree, child.tree); }
	void * subtree_insert(Tree child) { return tree_subtree_insert(tree, child.tree); }

	Tree  subtree_cut() { 
		ptree_t cut = tree_subtree_cut(tree);
		Tree t(tree_subtree_cut(tree));
		return t;
	}

	Tree  subtree_destroy(tree) {
		return Tree(tree_subtree_destroy(tree));
	}

	int   subtree_foreach(void *param, void (*userf)(void *, int, int, void *)) {
		return tree_subtree_foreach(tree, param, userf); 
	}
}

#endif
