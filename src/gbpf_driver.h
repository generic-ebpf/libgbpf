#pragma once

#include <gbpf/gbpf.h>
#include <gbpf_error.h>

struct gbpf_driver {
	/*
	 * TODO: Comment
	 */
	int (*prog_load)(struct gbpf_driver *,
			struct gbpf_prog_load_attr *);

	/*
	 * TODO: Comment
	 */
	int (*map_create)(struct gbpf_driver *,
			struct gbpf_map_create_attr *);

	/*
	 * TODO: Comment
	 */
	int (*map_destroy)(struct gbpf_driver *,
			struct gbpf_map_destroy_attr *);

	/*
	 * TODO: Comment
	 */
	int (*map_lookup_elem)(struct gbpf_driver *,
			struct gbpf_map_lookup_attr *);

	/*
	 * TODO: Comment
	 */
	int (*map_update_elem)(struct gbpf_driver *,
			struct gbpf_map_update_attr *);

	/*
	 * TODO: Comment
	 */
	int (*map_delete_elem)(struct gbpf_driver *,
			struct gbpf_map_delete_attr *);

	/*
	 * TODO: Comment
	 */
	int (*map_get_next_key)(struct gbpf_driver *,
			struct gbpf_map_get_next_key_attr *);

	/*
	 * TODO: Comment
	 */
	int (*destroy)(struct gbpf_driver *);
};
