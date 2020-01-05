from ctypes import *


class gbpf_driver(Structure):
    pass


class gbpf_module(Structure):
    pass


class gbpf_prog(Structure):
    pass


class gbpf_map(Structure):
    pass


class gbpf_prog_load_attr(Structure):
    _fields_ = [
        ("env", c_uint32),
        ("type", c_uint32),
        ("prog", c_void_p),
        ("prog_len", c_uint32),
        ("prog_descp", POINTER(c_uint32)),
    ]


class gbpf_map_create_attr(Structure):
    _fields_ = [
        ("env", c_uint32),
        ("type", c_uint32),
        ("key_size", c_uint32),
        ("value_size", c_uint32),
        ("max_entries", c_uint32),
        ("flags", c_uint32),
        ("map_descp", POINTER(c_uint32)),
    ]


class gbpf_map_destroy_attr(Structure):
    _fields_ = [
        ("map_desc", c_uint32)
    ]


class gbpf_map_lookup_attr(Structure):
    _fields_ = [
        ("map_desc", c_uint32),
        ("key", c_void_p),
        ("value", c_void_p),
    ]


class gbpf_map_update_attr(Structure):
    _fields_ = [
        ("map_desc", c_uint32),
        ("key", c_void_p),
        ("value", c_void_p),
        ("flags", c_uint64),
    ]


class gbpf_map_delete_attr(Structure):
    _fields_ = [
        ("map_desc", c_uint32),
        ("key", c_void_p),
    ]


class gbpf_map_get_next_key_attr(Structure):
    _fields_ = [
        ("map_desc", c_uint32),
        ("key", c_void_p),
        ("next_key", c_void_p),
    ]


libgbpf = CDLL("libgbpf.so", use_errno=True)
libgbpf.gbpf_freebsd_driver_create.restype = c_int
libgbpf.gbpf_freebsd_driver_create.argtypes = [POINTER(POINTER(gbpf_driver))]
libgbpf.gbpf_prog_load.restype = c_int
libgbpf.gbpf_prog_load.argtypes = [POINTER(gbpf_driver),
        POINTER(gbpf_prog_load_attr)]
libgbpf.gbpf_map_create.restype = c_int
libgbpf.gbpf_map_create.argtypes = [POINTER(gbpf_driver),
        POINTER(gbpf_map_create_attr)]
libgbpf.gbpf_map_destroy.restype = c_int
libgbpf.gbpf_map_destroy.argtypes = [POINTER(gbpf_driver),
        POINTER(gbpf_map_destroy_attr)]
libgbpf.gbpf_map_lookup_elem.restype = c_int
libgbpf.gbpf_map_lookup_elem.argtypes = [POINTER(gbpf_driver),
        POINTER(gbpf_map_lookup_attr)]
libgbpf.gbpf_map_update_elem.restype = c_int
libgbpf.gbpf_map_update_elem.argtypes = [POINTER(gbpf_driver),
        POINTER(gbpf_map_update_attr)]
libgbpf.gbpf_map_delete_elem.restype = c_int
libgbpf.gbpf_map_delete_elem.argtypes = [POINTER(gbpf_driver),
        POINTER(gbpf_map_delete_attr)]
libgbpf.gbpf_map_get_next_key.restype = c_int
libgbpf.gbpf_map_get_next_key.argtypes = [POINTER(gbpf_driver),
        POINTER(gbpf_map_get_next_key_attr)]
