//
// cen64-profile2callgrind: Convert a cen64 profile file into a callgrind one.
//
// CEN64: Cycle-Accurate Nintendo 64 Emulator.
// Copyright (C) 2018 Lauri Kasanen
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//


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

int main(int argc, char **argv) {

	string s;
	map<uint32_t, string> funcs;
	char buf[PATH_MAX];

	if (argc != 3) {
		die("Usage: %s my.z64.profile my.elf\n", argv[0]);
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

		char *ptr = strchr(buf, ' ');
		*ptr = '\0';

		s = "addr2line -s -e ";
		s += argv[2];
		s += " ";
		s += buf;

		char linebuf[PATH_MAX];
		FILE *p = popen(s.c_str(), "r");
		if (!fgets(linebuf, PATH_MAX, p))
			die("Failed getting addr\n");
		pclose(p);

		ptr = strchr(linebuf, ':');
		*ptr = '\0';
		ptr++;

		uint32_t line = 0;
		if (*ptr != '?' && sscanf(ptr, "%u", &line) != 1)
			die("Failed getting addr\n");

		// Okay, we have everything for this sample. Put it out
		fprintf(out, "fl=%s\n", linebuf);
		map<uint32_t, string>::const_iterator it = funcs.lower_bound(addr);
		it--;
		fprintf(out, "fn=%s\n", it->second.c_str());

		fprintf(out, "%u %lu\n\n", line, num);
	}

	fprintf(out, "totals: %lu\n", summary);

	fclose(f);
	fclose(out);

	return 0;
}

