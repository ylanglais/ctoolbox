#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "err.h"
#include "mem.h"
#include  "map.h"


typedef struct {
} csvs_t, *pcsvs_t; 

pcsvs_t 
cscs_new() {
	return mem_zmalloc(sizeof cscs_t);
}

pcsvs_t
csvs_destroy(pcsvs_t stats) {
	if (stats) {
		free(stats);
	}
	return NULL;
}

pcsvs_t
csv_analyse(char *filename) {
	pcsvs_t stats;
	if (!(stats = csvs_new())) {
		err_error("cannot allocate csv stat sturcture");
		return NULL;
	}

	pmap_t map;
	if (!(map = map_new(filename, mapRDONLY))) {
		err_error("cannot map %s", filename);
		return csvs_destroy(stats);
	}

	char *start, *end, *p;
	
	start = map_data_get(map);
	end   = start + map_size_get(map);
	
	for (p = start; p < end && p != '\n'; p++);
	
	
	
			
	

	
	

}
