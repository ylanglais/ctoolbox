

/*    
    This code is under GPL. Please read license terms and conditions at http://www.gnu.org/

    Yann LANGLAIS

    Changelog:
		26/09/2007	1.0	Creation
    
*/   

#include <stdlib.h>

char graph_MODULE[]  = "Graph structure";
char graph_PURPOSE[] = "Basic graph structure";
char graph_VERSION[] = "1.0";
char graph_DATEVER[] = "26/09/2007";

#include "storage.h"
#include "hash.h"
#include "index.h"

typedef struct {
	phash_t		vindex;	 /* vertices index   */
	pindex_t	veindex; /* index of edges form its vertices */
} graph_t, *pgraph_t;

#define  _graph_c_
#include "graph.h"
#undef   _graph_c_

pgraph_t 
graph_destroy(pgraph_t g) {
	if (!g) return NULL;
	if (g->vindex)  hash_destroy(g->vindex);
	if (g->veindex) index_destroy(g->veindex);
	free(g);	
	return NULL;
}

size_t graph_sizeof() {return sizeof(graph_t); }

pgraph_t
graph_new() {
	pgraph_t g;
	if (!(g = (pgraph_t) malloc(sizeof(graph_t)))) return NULL;
	if (!(g->vindex = hash_new()))   return graph_destroy(g);
	if (!(g->veindex = index_new())) return graph_destroy(g);
	return g;
}

int
graph_vertex_add(pgraph_t g, char *key, void *data) {
	if (!g || !g->vindex) return 1;	
	if (hash_insert(g->vindex, key, data)) return 2;
	return 0;
}

int
graph_edge_add(pgraph_t g, graph_key_t v1, graph_key_t v2, void *data) {
	int r;
	if (!g || !g->veindex) return 1;
	if (!hash_retrieve(g->vindex, v1) || !hash_retrieve(g->vindex, v2)) return 2;
	if (index_add(g->veindex, v1, data)) r = 4;
	if (index_add(g->veindex, v2, data)) r = 8;
	return r;
}

void *
graph_vertex_retrieve(pgraph_t g, graph_key_t key) {
	if (!g || !g->vindex) return NULL;
	return hash_retrieve(g->vindex, key);
}

plist_t
graph_vertex_edge_list(pgraph_t g, graph_key_t key) {
	if (!g || !g->veindex) return NULL;
	return index_list_retrieve(g->veindex, key);
}

#ifdef _test_graph_

#include <stdio.h>
#include <strings.h>
#include <string.h>

typedef struct {
	char name[32];
	int  x, y, z;
} vertex_t, *pvertex_t;

typedef struct {
	char name[32];
	int  weight, length;
} edge_t, *pedge_t;

pvertex_t vertex_new(char *name, int x, int y, int z) {
	pvertex_t v;

	if (!(v = malloc(sizeof(vertex_t)))) return NULL;
	strncpy(v->name, name, 31);
	v->name[31] = 0;
	v->x   = y;
	v->y   = y;
	v->z   = z;
	return v;
}	

pvertex_t vertex_destroy(pvertex_t v) {
	if (v) free(v);
	return NULL;
}

pedge_t edge_destroy(pedge_t e) {
	if (e) free(e);
	return NULL;
}

pedge_t edge_new(char *name, int weight, int length) {
	pedge_t e;

	if (!(e = malloc(sizeof(edge_t)))) return NULL;
	strncpy(e->name, name, 31);
	e->name[31] = 0;
	e->weight   = weight;
	e->length   = length;
	
	return e;
}

void edge_print(pedge_t e) {
	if (!e) return;
	printf("edge %s (%d, %d)\n", e->name, e->weight, e->length);
}
	
void vertex_print(pvertex_t v) {
	if (!v) return;
	printf("vertex %s (%d, %d, %d)\n", v->name, v->x, v->y, v->z);
}

#define echo_cmd(x) { printf(">>> perform command: %s\n", #x); x; }
#define vadd(strct) graph_vertex_add(g, strct->name, (void *) strct)
	
int main(void) {
	pgraph_t g;
	plist_t l;
	pvertex_t massy, amboise, tours, orleans, paris, blere;
	pedge_t   a10_massy_orleans, a10_orleans_amboise, a10_amboise_tours, a6_massy_paris,
			  dx_amboise_blere, dy_orleans_blere, dz_blere_tours, e; 

	echo_cmd(massy   = vertex_new("Massy", 0, 0, 100));
	echo_cmd(amboise = vertex_new("Amboise", 100, 100, 50));
	echo_cmd(orleans = vertex_new("Orleans", 50, 75, 60));
	echo_cmd(tours   = vertex_new("Tours", 150, 150, 45));
	echo_cmd(paris   = vertex_new("Paris", 0, 11, 30));
	echo_cmd(blere   = vertex_new("Blere", 100, 120, 50));

	echo_cmd(a10_massy_orleans   = edge_new("A10_Massy_Orleans", 120, 100));
	echo_cmd(a10_orleans_amboise = edge_new("A10_Orleans_Amboise", 130, 100));
	echo_cmd(a10_amboise_tours   = edge_new("A10_Amboise_Tours", 120, 40));
	echo_cmd(a6_massy_paris      = edge_new("A6_Massy_Paris", 70, 15));
	echo_cmd(dx_amboise_blere    = edge_new("DX_Amboise_Blere", 70, 7));
	echo_cmd(dy_orleans_blere    = edge_new("DY_Orleans_Blere", 50, 200));
	echo_cmd(dz_blere_tours      = edge_new("DZ_Blere_Tours", 50, 40));
	
	g = graph_new();
	
	echo_cmd(vadd(massy));
	echo_cmd(vadd(amboise));
	echo_cmd(vadd(tours));
	echo_cmd(vadd(orleans));
	echo_cmd(vadd(paris));
	echo_cmd(vadd(blere));

	echo_cmd(graph_edge_add(g,  "Massy",   "Orleans", (void *) a10_massy_orleans));
	echo_cmd(graph_edge_add(g,  "Amboise", "Orleans", (void *) a10_orleans_amboise));
	echo_cmd(graph_edge_add(g,  "Amboise", "Tours",   (void *) a10_amboise_tours));
	echo_cmd(graph_edge_add(g,  "Massy",   "Paris",   (void *) a6_massy_paris));
	echo_cmd(graph_edge_add(g,  "Amboise", "Blere",   (void *) dx_amboise_blere));
	echo_cmd(graph_edge_add(g,  "Blere",   "Orleans", (void *) dy_orleans_blere));
	echo_cmd(graph_edge_add(g,  "Blere",   "Tours",   (void *) dz_blere_tours));
	
	echo_cmd(vertex_print(graph_vertex_retrieve(g, "Massy")));
	echo_cmd(vertex_print(graph_vertex_retrieve(g, "Blere")));
	
	echo_cmd(index_foreach(g->veindex, (f_index_hook_t) edge_print));

	echo_cmd(l = graph_vertex_edge_list(g, "Massy"));
	echo_cmd(for (e = list_first(l); e; e = list_next(l)) edge_print(e));
	echo_cmd(l = graph_vertex_edge_list(g, "Orleans"));
	echo_cmd(for (e = list_first(l); e; e = list_next(l)) edge_print(e));

	/* echo_cmd(hash_dump(g->vindex)); */
	echo_cmd(index_foreach(g->veindex, (f_index_hook_t) edge_print));

	vertex_destroy(massy);
	vertex_destroy(amboise);
	vertex_destroy(orleans);
	vertex_destroy(tours);
	vertex_destroy(paris);
	vertex_destroy(blere);

	edge_destroy(a10_massy_orleans);
	edge_destroy(a10_orleans_amboise);
	edge_destroy(a10_amboise_tours);
	edge_destroy(a6_massy_paris);
	edge_destroy(dx_amboise_blere);
	edge_destroy(dy_orleans_blere);
	edge_destroy(dz_blere_tours);

	graph_destroy(g);

	return 0;
}
#endif


