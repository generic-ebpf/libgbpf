#include <stdlib.h>
#include <pthread.h>

#include <gbpf_error.h>

struct gbpf_error {
	enum gbpf_error_types type;
	union {
		int libgbpf_errno;
		int system_errno;
		int libelf_errno;
	};
};

static pthread_key_t libgbpf_errno;

static void
set_error(struct gbpf_error *gbpf_error,
		enum gbpf_error_types type, int errid)
{
	switch (type) {
	case GBPF_ERROR_TYPE_LIBGBPF:
		gbpf_error->libgbpf_errno = errid;
		break;
	case GBPF_ERROR_TYPE_SYSTEM:
		gbpf_error->system_errno = errid;
		break;
	case GBPF_ERROR_TYPE_LIBELF:
		gbpf_error->libelf_errno = errid;
		break;
	default:
		/* print error message */
		gbpf_error->type = GBPF_ERROR_TYPE_UNKNOWN;
		return;
	}

	gbpf_error->type = type;
}

static void
libgbpf_errno_dtor(void *specific)
{
	free(specific);
}

int
gbpf_errno_init(void)
{
	return pthread_key_create(&libgbpf_errno,
			libgbpf_errno_dtor);
}

int
gbpf_errno_deinit(void)
{
	return pthread_key_delete(libgbpf_errno);
}

void
gbpf_set_error(enum gbpf_error_types type, int errid)
{
	int error;
	struct gbpf_error *gbpf_error;

	gbpf_error = pthread_getspecific(libgbpf_errno);
	if (gbpf_error == NULL) {
		gbpf_error = malloc(sizeof(*gbpf_error));
		if (gbpf_error == NULL) {
			/* print error message */
			return;
		}

		set_error(gbpf_error, type, errid);

		error = pthread_setspecific(libgbpf_errno, gbpf_error);
		if (error != 0) {
			/* print error message */
			free(gbpf_error);
			return;
		}
	} else {
		set_error(gbpf_error, type, errid);
	}
}
