#pragma once

struct gbpf_error;

enum gbpf_error_types {
	GBPF_ERROR_TYPE_UNKNOWN,
	GBPF_ERROR_TYPE_LIBGBPF,
	GBPF_ERROR_TYPE_SYSTEM,
	GBPF_ERROR_TYPE_LIBELF
};

int gbpf_errno_init(void);
int gbpf_errno_deinit(void);
void gbpf_set_error(enum gbpf_error_types type, int errid);
