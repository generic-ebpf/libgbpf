#pragma once

#include <stdint.h>

struct gbpf_driver;
struct gbpf_module;
struct gbpf_prog;
struct gbpf_map;

struct gbpf_prog_load_attr {
	uint32_t env;
	uint32_t type;
	void *prog;
	uint32_t prog_len;
	uint32_t *prog_descp;
};

struct gbpf_map_create_attr {
	uint32_t env;
	uint32_t type;
	uint32_t key_size;
	uint32_t value_size;
	uint32_t max_entries;
	uint32_t flags;
	uint32_t *map_descp;
};

struct gbpf_map_destroy_attr {
	uint32_t map_desc;
};

struct gbpf_map_lookup_attr {
	uint32_t map_desc;
	void *key;
	void *value;
};

struct gbpf_map_update_attr {
	uint32_t map_desc;
	void *key;
	void *value;
	uint64_t flags;
};

struct gbpf_map_delete_attr {
	uint32_t map_desc;
	void *key;
};

struct gbpf_map_get_next_key_attr {
	uint32_t map_desc;
	void *key;
	void *next_key;
};

int gbpf_freebsd_driver_create(struct gbpf_driver **);
int gbpf_prog_load(struct gbpf_driver *, struct gbpf_prog_load_attr *);
int gbpf_map_create(struct gbpf_driver *, struct gbpf_map_create_attr *);
int gbpf_map_destroy(struct gbpf_driver *, struct gbpf_map_destroy_attr *);
int gbpf_map_lookup_elem(struct gbpf_driver *, struct gbpf_map_lookup_attr *);
int gbpf_map_update_elem(struct gbpf_driver *, struct gbpf_map_update_attr *);
int gbpf_map_delete_elem(struct gbpf_driver *, struct gbpf_map_delete_attr *);
int gbpf_map_get_next_key(struct gbpf_driver *, struct gbpf_map_get_next_key_attr *);
int gbpf_driver_destroy(struct gbpf_driver *driver);
