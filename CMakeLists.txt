cmake_minimum_required(VERSION 3.1)

project(gbpf_lib
	VERSION 0.0.1
	DESCRIPTION "Generic eBPF Library"
	LANGUAGES C
)

set(gbpf-src
	src/gbpf_driver.c
	src/gbpf_elf_core.c
	src/gbpf_error.c
	src/gbpf_freebsd_driver.c
)

add_library(gbpf STATIC SHARED ${gbpf-src})

target_link_libraries(gbpf PRIVATE elf)
target_include_directories(gbpf
	PUBLIC ${PROJECT_SOURCE_DIR}/include
	PUBLIC ${PROJECT_SOURCE_DIR}/src
)
