
#ifndef _Hash_h_
#define _Hash_h_
#include <hash.h>

class Hash {
private:
	phash_t h;

public:
	Hash() { h = hash_new(); };
	Hash(size_t size) { h = hash_new_sized(size); };
	Hash(f_hashcode_t f_code) { h = hash_new_fcode(f_code); };
	Hash(size_t size, f_hashcode_t f_code) { h = hash_new_set(size, f_code); };
	~Hash() { h = hash_destroy(h); };

	void code_function_set(f_hashcode_t f_code) { hash_set_code_function(h, f_code); };
	void resize(size_t size) { hash_resize(h, size); };
	void rehash(int size = 0) { hash_resize(h, size); };
	int	 decimal_code(char *name) { return hash_decimal_code(h, name); };
	int  classic_code(char *name) { return hash_classic_code(h, name); };
	int  insert(char *name, void *pdata) { return hash_insert(h, name, pdata); };
	void *retrieve(char *name) { return hash_retrieve(h, name); };

	void * operator [] (const char *name) { return retrieve((char *) name); };

	int  used() { return hash_used(h); };
	int  size() { return hash_size(h); };

	void *remove(char *name) { return hash_delete(h, name); };

#ifdef __HASH_STATISTICS__
	void stats() { hash_stats(h); };
	void stats_reset() { hash_stats_reset(h); };
#endif
	void dump() { hash_dump(h); };
};

#endif 
