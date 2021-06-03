#ifndef _Graph_h_
#define _Graph_h_


#include <list.hh>
#include <graph.h>

#ifndef graph_key_t
typedef char *graph_key_t;
#endif

class Graph {
private:
	pgraph_t graph;	

public: 
	Graph()  { graph = graph_new(); }
	~Graph() { graph = graph_destroy(graph); }

	int 	vertex_add(char *key, void *data) { return graph_vertex_add(graph, key, data); }
	int     edge_add(graph_key_t v1, graph_key_t v2, void *data) { return graph_edge_add(v1, v2, data); } 
	void *  vertex_retrieve(graph_key_t key) { return graph_vertex_retrieve(graph, key); }
	void *  operator [] (graph_key_t key)    { return graph_vertex_retrieve(graph, key); }

	List	vertex_edge_list(graph_key_t key) { return List(graph_vertex_edge_list(graph, graph_key_t key)); }
}


#endif

