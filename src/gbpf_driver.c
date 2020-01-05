#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

#include <gbpf/gbpf.h>
#include <gbpf_driver.h>

int
gbpf_prog_load(struct gbpf_driver *driver,
		struct gbpf_prog_load_attr *attr)
{
	if (driver == NULL || attr == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	return driver->prog_load(driver, attr);
}

int
gbpf_map_create(struct gbpf_driver *driver,
		struct gbpf_map_create_attr *attr)
{
	if (driver == NULL || attr == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	return driver->map_create(driver, attr);
}

int
gbpf_map_destroy(struct gbpf_driver *driver,
		struct gbpf_map_destroy_attr *attr)
{
	if (driver == NULL || attr == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	return driver->map_destroy(driver, attr);
}

int
gbpf_map_lookup_elem(struct gbpf_driver *driver,
		struct gbpf_map_lookup_attr *attr)
{
	if (driver == NULL || attr == NULL || attr->key == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	return driver->map_lookup_elem(driver, attr);
}

int
gbpf_map_update_elem(struct gbpf_driver *driver,
		struct gbpf_map_update_attr *attr)
{
	if (driver == NULL || attr == NULL || attr->key == NULL ||
			attr->value == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	return driver->map_update_elem(driver, attr);
}

int
gbpf_map_delete_elem(struct gbpf_driver *driver,
		struct gbpf_map_delete_attr *attr)
{
	if (driver == NULL || attr == NULL || attr->key == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	return driver->map_delete_elem(driver, attr);
}

int
gbpf_map_get_next_key(struct gbpf_driver *driver,
		struct gbpf_map_get_next_key_attr *attr)
{
	if (driver == NULL || attr == NULL || attr->next_key == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	return driver->map_get_next_key(driver, attr);
}

int
gbpf_driver_destroy(struct gbpf_driver *driver)
{
	if (driver == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	return driver->destroy(driver);
}
