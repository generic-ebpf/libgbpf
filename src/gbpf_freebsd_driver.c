#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/ebpf_dev.h>
#include <unistd.h>

#include <gbpf_driver.h>

struct gbpf_freebsd_driver {
	struct gbpf_driver base;
	int fd;
};

static int
map_create(struct gbpf_driver *_driver,
		struct gbpf_map_create_attr *attr)
{
	int error, fd;
	uint32_t map_desc;
	struct ebpf_map *em;
	struct ebpf_map_create_req req;
	struct gbpf_freebsd_driver *driver =
		(struct gbpf_freebsd_driver *)_driver;

	req.env = attr->env;
	req.type = attr->type;
	req.key_size = attr->key_size;
	req.value_size = attr->value_size;
	req.max_entries = attr->max_entries;
	req.flags = attr->flags;
	req.fdp = &fd;

	error = ioctl(driver->fd, EBPFIOC_MAP_CREATE, &req);
	if (error == -1) {
		gbpf_set_error(GBPF_ERROR_TYPE_SYSTEM, errno);
		return -1;
	}

	*attr->map_descp = (uint32_t)fd;

	return 0;
}

static int
map_destroy(struct gbpf_driver *_driver,
		struct gbpf_map_destroy_attr *attr)
{
	close((int)attr->map_desc);
	return 0;
}

static int
map_lookup_elem(struct gbpf_driver *_driver,
		struct gbpf_map_lookup_attr *attr)
{
	int error;
	struct ebpf_map_lookup_req req;
	struct gbpf_freebsd_driver *driver =
		(struct gbpf_freebsd_driver *)_driver;

	req.fd = (int)attr->map_desc;
	req.key = attr->key;
	req.value = attr->value;

	error = ioctl(driver->fd, EBPFIOC_MAP_LOOKUP_ELEM, &req);
	if (error == -1) {
		gbpf_set_error(GBPF_ERROR_TYPE_SYSTEM, errno);
		return -1;
	}

	return 0;
}

static int
map_update_elem(struct gbpf_driver *_driver,
		struct gbpf_map_update_attr *attr)
{
	int error;
	struct ebpf_map_update_req req;
	struct gbpf_freebsd_driver *driver =
		(struct gbpf_freebsd_driver *)_driver;

	req.fd = (int)attr->map_desc;
	req.key = attr->key;
	req.value = attr->value;
	req.flags = attr->flags;

	error = ioctl(driver->fd, EBPFIOC_MAP_UPDATE_ELEM, &req);
	if (error == -1) {
		gbpf_set_error(GBPF_ERROR_TYPE_SYSTEM, errno);
		return -1;
	}

	return 0;
}

static int
map_delete_elem(struct gbpf_driver *_driver,
		struct gbpf_map_delete_attr *attr)
{
	int error;
	struct ebpf_map_delete_req req;
	struct gbpf_freebsd_driver *driver =
		(struct gbpf_freebsd_driver *)_driver;

	req.fd = (int)attr->map_desc;
	req.key = attr->key;

	error = ioctl(driver->fd, EBPFIOC_MAP_DELETE_ELEM, &req);
	if (error == -1) {
		gbpf_set_error(GBPF_ERROR_TYPE_SYSTEM, errno);
		return -1;
	}

	return 0;
}

static int
map_get_next_key(struct gbpf_driver *_driver,
		struct gbpf_map_get_next_key_attr *attr)
{
	int error;
	struct ebpf_map_get_next_key_req req;
	struct gbpf_freebsd_driver *driver =
		(struct gbpf_freebsd_driver *)_driver;

	req.fd = (int)attr->map_desc;
	req.key = attr->key;
	req.next_key = attr->next_key;

	error = ioctl(driver->fd, EBPFIOC_MAP_GET_NEXT_KEY, &req);
	if (error == -1) {
		gbpf_set_error(GBPF_ERROR_TYPE_SYSTEM, errno);
		return -1;
	}

	return 0;
}

static int
gbpf_freebsd_driver_destroy(struct gbpf_driver *_driver)
{
	int error;
	struct gbpf_freebsd_driver *driver =
		(struct gbpf_freebsd_driver *)_driver;

	close(driver->fd);
	free(driver);

	return 0;
}

int
gbpf_freebsd_driver_create(struct gbpf_driver **driverp)
{
	int error;
	struct gbpf_freebsd_driver *driver;

	if (driverp == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_SYSTEM, EINVAL);
		return -1;
	}

	driver = malloc(sizeof(*driver));
	if (driver == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_SYSTEM, ENOMEM);
		return -1;
	}

	driver->base.map_create = map_create;
	driver->base.map_destroy = map_destroy;
	driver->base.map_lookup_elem = map_lookup_elem;
	driver->base.map_update_elem = map_update_elem;
	driver->base.map_delete_elem = map_delete_elem;
	driver->base.map_get_next_key = map_get_next_key;
	driver->base.destroy = gbpf_freebsd_driver_destroy;

	driver->fd = open("/dev/ebpf", O_RDWR);
	if (driver->fd == -1) {
		gbpf_set_error(GBPF_ERROR_TYPE_SYSTEM, errno);
		free(driver);
		return -1;
	}

	*driverp = (struct gbpf_driver *)driver;

	return 0;
}
