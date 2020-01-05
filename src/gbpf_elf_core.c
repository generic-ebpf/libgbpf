#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <libelf.h>
#include <gelf.h>

#include <gbpf_error.h>

#ifndef EM_BPF
#define EM_BPF 247
#endif

struct sec_info {
	uint32_t nsecs;
	struct sec_info_ent {
		int idx;
		GElf_Shdr sh;
		Elf_Data *data;
	} *secs;
};

struct gbpf_elf_file {
	int fd;
	Elf *elf;
	GElf_Ehdr e;
	int strtab_idx;
	struct sec_info *maps;
	struct sec_info *symtabs;
	struct sec_info *progs;
	struct sec_info *relocs;
};

struct gbpf_prog {
	int sec_idx;
	uint32_t desc;
	size_t sec_offset;
};

struct gbpf_map {
	int sec_idx;
	uint32_t desc;
	size_t sec_offset;
};

struct gbpf_object {
	struct gbpf_prog *progs;
	uint32_t nprogs;
	struct gbpf_map *maps;
	uint32_t nmaps;
	struct gbpf_elf_file efile;
};

static int
sec_info_create(struct sec_info **sip)
{
	struct sec_info *ret;

	ret = malloc(sizeof(*ret));
	if (ret == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_SYSTEM, ENOMEM);
		return -1;
	}

	ret->nsecs = 0;
	ret->secs = NULL;

	*sip = ret;

	return 0;
}

static void
sec_info_destroy(struct sec_info *si)
{
	if (si->secs != NULL) {
		free(si->secs);
	}
	free(si);
}

static int
sec_info_append(struct sec_info *si, int idx,
		GElf_Shdr *shp, Elf_Data *data)
{
	void *secs;
	uint32_t nsecs = si->nsecs;

	/*
	 * Don't allow empty
	 */
	if (data->d_size == 0) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	secs = reallocarray(si->secs, nsecs + 1,
			sizeof(*si->secs));
	if (secs == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_SYSTEM, ENOMEM);
		return -1;
	}

	si->secs = secs;
	si->nsecs++;
	si->secs[nsecs].idx = idx;
	si->secs[nsecs].sh = *shp;
	si->secs[nsecs].data = data;

	return 0;
}

static void
sec_info_dump(struct sec_info *si)
{
	for (uint32_t i = 0; i < si->nsecs; i++) {
		printf("secs[%d] idx: %d\n", i, si->secs[i].idx);
	}
}

static bool
is_little_endian(void)
{
	uint32_t test_num = 0xdeadbeef;
	if (*((uint8_t *)&test_num) == 0xef) {
		/* little endian */
		return true;
	} else if (*((uint8_t *)&test_num) == 0xde) {
		/* big endian */
		return false;
	} else {
		/* Unknown endian */
		assert(false);
	}
}

static bool
check_elf_header(GElf_Ehdr *ep)
{
	/* check magic number */
	if (ep->e_ident[EI_MAG0] != 0x7f ||
			ep->e_ident[EI_MAG1] != 'E' ||
			ep->e_ident[EI_MAG2] != 'L' ||
			ep->e_ident[EI_MAG3] != 'F') {
		return false;
	}

	/* Don't care about the EI_CLASS */

	/* Check endianness */
	if (is_little_endian()) {
		if (ep->e_ident[EI_DATA] != ELFDATA2LSB) {
			return false;
		}
	} else {
		if (ep->e_ident[EI_DATA] != ELFDATA2MSB) {
			return false;
		}
	}

	/* e_type should be relocatable file */
	if (ep->e_type != ET_REL) {
		return false;
	}

	/* e_machine should be EM_NONE (for old LLVM) or EM_BPF */
	if (ep->e_machine != EM_NONE && ep->e_machine != EM_BPF) {
		return false;
	}

	/* Don't care about the e_version, e_entry, e_phoff */

	/* Need section header */
	if (ep->e_shoff == 0) {
		return false;
	}

	/* Don't care about the e_flags, e_ehsize, e_phentsize and e_phnum */

	/* Need non-zero section header entries */
	if (ep->e_shentsize == 0 || ep->e_shnum == 0) {
		return false;
	}

	/* Need string table. Do extra check as well for corrupted file */
	if (ep->e_shstrndx == SHN_UNDEF) {
		return false;
	}

	return true;
}

static int
efile_open(const char *fname, struct gbpf_elf_file *efile)
{
	efile->fd = open(fname, O_RDONLY);
	if (efile->fd == -1) {
		gbpf_set_error(GBPF_ERROR_TYPE_SYSTEM, errno);
		return -1;
	}

	efile->elf = elf_begin(efile->fd, ELF_C_READ, NULL);
	if (efile->elf == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBELF, elf_errno());
		goto err0;
	}

	if (!gelf_getehdr(efile->elf, &efile->e)) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBELF, elf_errno());
		goto err1;
	}

	if (!check_elf_header(&efile->e)) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		goto err1;
	}

	efile->strtab_idx = -1;
	if (sec_info_create(&efile->maps) != 0) goto err1;
	if (sec_info_create(&efile->progs) != 0) goto err2;
	if (sec_info_create(&efile->relocs) != 0) goto err3;
	if (sec_info_create(&efile->symtabs) != 0) goto err4;

	return 0;

err4:
	sec_info_destroy(efile->relocs);
err3:
	sec_info_destroy(efile->progs);
err2:
	sec_info_destroy(efile->maps);
err1:
	elf_end(efile->elf);
err0:
	close(efile->fd);
	return -1;
}

static void
efile_close(struct gbpf_elf_file *efile)
{
	sec_info_destroy(efile->symtabs);
	sec_info_destroy(efile->relocs);
	sec_info_destroy(efile->progs);
	sec_info_destroy(efile->maps);
	elf_end(efile->elf);
	close(efile->fd);
}

/*
 * Borrowed from libbpf.
 * File: https://github.com/libbpf/libbpf/blob/master/src/libbpf.c
 * Function: section_have_execinstr
 */
static bool
section_have_execinstr(struct gbpf_elf_file *efile, int idx)
{
	Elf_Scn *scn;
	GElf_Shdr sh;

	scn = elf_getscn(efile->elf, idx);
	if (!scn)
		return false;

	if (gelf_getshdr(scn, &sh) != &sh)
		return false;

	if (sh.sh_flags & SHF_EXECINSTR)
		return true;

	return false;
}

static int
efile_add_map(struct gbpf_elf_file *efile, int idx,
		GElf_Shdr *shp, Elf_Data *data)
{
	/*
	 * We only allow single maps section
	 */
	if (efile->maps->nsecs != 0) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	return sec_info_append(efile->maps, idx, shp, data);
}

static int
efile_add_prog(struct gbpf_elf_file *efile, int idx,
		GElf_Shdr *shp, Elf_Data *data)
{
	return sec_info_append(efile->progs, idx, shp, data);
}

static int
efile_add_reloc(struct gbpf_elf_file *efile, int idx,
		GElf_Shdr *shp, Elf_Data *data)
{
	int sec = shp->sh_info;

	/*
	 * Process only relocation to the section
	 * with exec flag
	 */
	if (!section_have_execinstr(efile, sec)) {
		return 0;
	}

	return sec_info_append(efile->relocs, idx, shp, data);
}

static int
efile_add_symtab(struct gbpf_elf_file *efile, int idx,
		GElf_Shdr *shp, Elf_Data *data)
{
	/*
	 * We only allow single symtab section
	 */
	if (efile->symtabs->nsecs != 0) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	return sec_info_append(efile->symtabs, idx, shp, data);
}

static int
efile_add_strtab(struct gbpf_elf_file *efile, int idx,
		GElf_Shdr *shp, Elf_Data *data)
{
	/*
	 * We only need idx for strtab
	 */
	efile->strtab_idx = idx;
	return 0;
}

static int
efile_collect_sections(struct gbpf_elf_file *efile)
{
	int idx = 0, error = 0;
	Elf *elf = efile->elf;
	GElf_Ehdr *ep = &efile->e;
	Elf_Scn *scn = NULL;

	/* Elf is corrupted/truncated, avoid calling elf_strptr. */
	if (!elf_rawdata(elf_getscn(elf, ep->e_shstrndx), NULL)) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	while ((scn = elf_nextscn(elf, scn)) != NULL) {
		char *name;
		GElf_Shdr sh;
		Elf_Data *data;

		idx++;

		if (gelf_getshdr(scn, &sh) != &sh) {
			gbpf_set_error(GBPF_ERROR_TYPE_LIBELF, elf_errno());
			return -1;
		}

		name = elf_strptr(elf, ep->e_shstrndx, sh.sh_name);
		if (!name) {
			gbpf_set_error(GBPF_ERROR_TYPE_LIBELF, elf_errno());
			return -1;
		}

		data = elf_getdata(scn, 0);
		if (!data) {
			gbpf_set_error(GBPF_ERROR_TYPE_LIBELF, elf_errno());
			return -1;
		}

		printf("Processing section %s\n", name);

		if (strcmp(name, "maps") == 0) {
			printf("Found maps section\n");
			error = efile_add_map(efile, idx, &sh, data);
		} else if (sh.sh_type == SHT_REL) {
			printf("Found reloc section\n");
			error = efile_add_reloc(efile, idx, &sh, data);
		} else if (sh.sh_type == SHT_SYMTAB) {
			printf("Found symtab section\n");
			error = efile_add_symtab(efile, idx, &sh, data);
		} else if (sh.sh_type == SHT_PROGBITS &&
				data->d_size > 0 && (sh.sh_flags & SHF_EXECINSTR)) {
			printf("Found prog section\n");
			error = efile_add_prog(efile, idx, &sh, data);
		} else if (strcmp(name, ".strtab") == 0 &&
				!(sh.sh_flags & SHF_ALLOC)) {
			printf("Found strtab section\n");
			error = efile_add_strtab(efile, idx, &sh, data);
		} else {
			printf("Skipped\n");
			error = 0;
		}

		if (error != 0) {
			return -1;
		}
	}

	/*
	 * Check required sections
	 */
	if (efile->strtab_idx == -1 || efile->symtabs == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return -1;
	}

	return 0;
}

static void
efile_dump(struct gbpf_elf_file *efile)
{
	printf("fd: %d\n", efile->fd);
	printf("elf: %p\n", efile->elf);
	printf("strtab_idx: %d\n", efile->strtab_idx);
	printf("maps\n");
	sec_info_dump(efile->maps);
	printf("progs\n");
	sec_info_dump(efile->progs);
	printf("relocs\n");
	sec_info_dump(efile->progs);
	printf("symtabs\n");
	sec_info_dump(efile->symtabs);
}

struct gbpf_elf_file *
gbpf_elf_open(const char *fname)
{
	int error;
	struct gbpf_elf_file *efile;

	if (fname == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return NULL;
	}

	if (elf_version(EV_CURRENT) == EV_NONE) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, EINVAL);
		return NULL;
	}

	efile = malloc(sizeof(*efile));
	if (efile == NULL) {
		gbpf_set_error(GBPF_ERROR_TYPE_LIBGBPF, ENOMEM);
		return NULL;
	}

	error = efile_open(fname, efile);
	if (error == -1) {
		goto err0;
	}

	error = efile_collect_sections(efile);
	if (error == -1) {
		goto err1;
	}

	return efile;

err1:
	efile_close(efile);
err0:
	free(efile);
	return NULL;
}

void
gbpf_elf_close(struct gbpf_elf_file *efile)
{
	if (efile == NULL) {
		return;
	}
	efile_close(efile);
	free(efile);
}

int
main(void)
{
	int error;
	struct gbpf_object obj;
	struct gbpf_elf_file *efile;

	efile = gbpf_elf_open("test.bpf.o");
	if (efile == NULL) {
		return EXIT_FAILURE;
	}

	// module_instantiate_maps(&mod, efile);

	gbpf_elf_close(efile);

	return EXIT_SUCCESS;
}
