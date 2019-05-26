//
// cen64-profile2callgrind: Convert a cen64 profile file into a callgrind one.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2018 Lauri Kasanen
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#define PACKAGE "" // work around bfd.h requirement
#include <bfd.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <string>

using namespace std;

static void die(const char fmt[], ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	exit(1);
}

static asymbol **syms = NULL;
static uint32_t num_syms = 0;

static bool found;
static const char *filename, *funcname;
static unsigned lineno, discriminator;

static void findsym(bfd *bin, asection *section, void *addrv) {
	bfd_vma vma;
	bfd_size_type size;

	if (found)
		return;
	if ((bfd_get_section_flags(bin, section) & SEC_ALLOC) == 0)
		return;
	vma = bfd_get_section_vma(bin, section);
	const uint32_t addr = *(uint32_t *) addrv;

	if (addr < vma)
		return;
	size = bfd_get_section_size(section);
	if (addr >= vma + size)
		return;

	found = bfd_find_nearest_line_discriminator(bin, section, syms, addr - vma,
							&filename, &funcname,
							&lineno, &discriminator);
}

int main(int argc, char **argv) {

	string s;
	map<uint32_t, string> funcs;
	char buf[PATH_MAX];

	if (argc != 3) {
		die("Usage: %s my.z64.profile my.elf\n", argv[0]);
	}

	bfd_init();
	//bfd_set_default_target("elf32-big");
	bfd *bin = bfd_openr(argv[2], NULL);
	if (!bin) die("Failed to open elf file\n");

	bin->flags |= BFD_DECOMPRESS;

	if (bfd_check_format(bin, bfd_archive) ||
		!bfd_check_format_matches(bin, bfd_object, NULL))
		die("Unable to get addresses from elf file\n");

	const uint32_t storage = bfd_get_symtab_upper_bound(bin);
	if (storage) {
		syms = (asymbol **) malloc(storage);
		num_syms = bfd_canonicalize_symtab(bin, syms);
	}

	s = "nm ";
	s += argv[2];
	s += " | grep -i \" t \"";

	FILE *f = popen(s.c_str(), "r");
	if (!f) die("Can't get functions\n");

	// Add the bootloader
	funcs[0x80000000] = "PIF bootloader";
	while (fgets(buf, PATH_MAX, f)) {
		uint32_t tmp;
		if (sscanf(buf, "%x", &tmp) != 1)
			die("Failed getting functions\n");

		char *ptr = strrchr(buf, ' ');
		if (!ptr)
			die("Failed getting functions\n");
		ptr++;

		uint32_t len = strlen(ptr);
		if (ptr[len - 1] == '\n')
			ptr[len - 1] = '\0';

		// Skip internal/libc magic parts
		if (ptr[0] == '_' && ptr[1] == '_')
			continue;

		funcs[tmp] = ptr;
	}

	pclose(f);

	strcpy(buf, argv[1]);
	char *ptr = strstr(buf, ".profile");
	if (ptr) {
		*ptr = '\0';
	}
	strcat(buf, ".callgrind");

	FILE *out = fopen(buf, "w");
	if (!out) die("Can't open '%s' for output\n", buf);

	fprintf(out, "# callgrind format\n");
	fprintf(out, "cmd: %s\n", argv[2]);
	fprintf(out, "events: instructions\n\n");

	fprintf(out, "ob=%s\n\n", argv[2]);

	uint64_t summary = 0;

	// We have a list of functions, now turn the samples into C line info
	f = fopen(argv[1], "r");
	if (!f) die("Can't open profile file\n");

	while (fgets(buf, PATH_MAX, f)) {
		uint32_t addr;
		uint64_t num;
		if (sscanf(buf, "%x %lu", &addr, &num) != 2)
			die("Malformed profile file\n");

		summary += num;

		found = false;
		bfd_map_over_sections(bin, findsym, &addr);

		if (found) {
			const char *ptr = filename ? strrchr(filename, '/') : NULL;
			if (ptr) {
				ptr++;
				fprintf(out, "fl=%s\n", ptr);
			} else {
				fprintf(out, "fl=??\n");
			}

			fprintf(out, "fn=%s\n", funcname);
			fprintf(out, "%u %lu\n\n", lineno, num);
		} else {
			fprintf(out, "fl=??\n");
			map<uint32_t, string>::const_iterator it = funcs.lower_bound(addr);
			it--;
			fprintf(out, "fn=%s\n", it->second.c_str());

			fprintf(out, "0 %lu\n\n", num);
		}
	}

	bfd_close(bin);

	fprintf(out, "totals: %lu\n", summary);

	fclose(f);
	fclose(out);
	free(syms);

	return 0;
}

