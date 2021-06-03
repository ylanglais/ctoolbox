#ifndef _Map_h_
#define _Map_h_

#include <map.h>

/* Map a file to a pointer: */

class Map {
private:
	pmap_t map;

public:²
	Map(char *filename, int flag) { map = map_new(filename, flag); }
	~Map()                        { map = map_destroy(map); }

	/* Acces to map internal:      */
	char * data() { return map_data_get(map);  }
    size_t size() { return map_data_size(map); }
	int    flag() { return map_flag_get(map);  }

	/* Modifying data: */
	/* insert size bytes at p from data: */
	int    insert_at(char *p, char *data, size_t size) { return map_insert_at(map, p, data, size); }
	/* delete size bytes at p: */
	int    delete_at(char *p, size_t size)             { return map_delete_at(map, p, size);       }

	/* replace oldsize byte from p by newsize byte from data: */
	int    replate_at(char *p, char *data, size_t oldsize, size_t newsize) {return map_replace_at(map, p, data, oldsize, newsize); }
};

#endif
