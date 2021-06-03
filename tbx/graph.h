#ifndef _graph_h_
#define _graph_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _graph_c_
typedef void *pgraph_t;
#endif

typedef char *graph_key_t;

#include <tbx/list.h>

size_t   graph_sizeof();

pgraph_t graph_destroy(pgraph_t g);
pgraph_t graph_new();

int     graph_vertex_add(pgraph_t g, char *key, void *data);
int     graph_edge_add(pgraph_t g, graph_key_t v1, graph_key_t v2, void *data);
void *  graph_vertex_retrieve(pgraph_t g, graph_key_t key);
plist_t graph_vertex_edge_list(pgraph_t g, graph_key_t key);

#ifdef __cplusplus
}
#endif


#endif

