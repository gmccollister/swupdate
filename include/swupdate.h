/*
 * (C) Copyright 2012-2014
 * Stefano Babic, DENX Software Engineering, sbabic@denx.de.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc.
 */

#ifndef _SWUPDATE_H
#define _SWUPDATE_H

#include <sys/types.h>
#include "bsdqueue.h"
#include "globals.h"

/*
 * swupdate uses SHA256 hashes
 */
#define SHA256_HASH_LENGTH	32

typedef enum {
	FLASH,
	UBI,
	FILEDEV,
	PARTITION,
	SCRIPT
} imagetype_t;

/*
 * this is used to indicate if a file
 * in the .swu image is required for the
 * device, or can be skipped
 */
enum {
	COPY_FILE,
	SKIP_FILE,
	INSTALL_FROM_STREAM
};


struct sw_version {
	char name[SWUPDATE_GENERAL_STRING_SIZE];
	char version[SWUPDATE_GENERAL_STRING_SIZE];
	int install_if_different;
	LIST_ENTRY(sw_version) next;
};

LIST_HEAD(swver, sw_version);


struct img_type {
	struct sw_version id;
	char type[SWUPDATE_GENERAL_STRING_SIZE];
	char fname[MAX_IMAGE_FNAME];
	char volname[MAX_VOLNAME];
	char device[MAX_VOLNAME];
	char path[MAX_IMAGE_FNAME];
	char extract_file[MAX_IMAGE_FNAME];
	char filesystem[MAX_IMAGE_FNAME];
	int required;
	int provided;
	int compressed;
	int install_directly;
	int is_script;
	int is_partitioner;
	long long partsize;
	int fdin;	/* Used for streaming file */
	off_t offset;	/* offset in cpio file */
	long long size;
	unsigned int checksum;
	unsigned char sha256[SHA256_HASH_LENGTH];	/* SHA-256 is 32 byte */
	LIST_ENTRY(img_type) next;
};

LIST_HEAD(imglist, img_type);

struct hw_type {
	char boardname[SWUPDATE_GENERAL_STRING_SIZE];
	char revision[SWUPDATE_GENERAL_STRING_SIZE];
	LIST_ENTRY(hw_type) next;
};

LIST_HEAD(hwlist, hw_type);

enum {
	SCRIPT_NONE,
	SCRIPT_PREINSTALL,
	SCRIPT_POSTINSTALL
};

struct uboot_var {
	char varname[UBOOT_VAR_LENGTH];
	char value[255];
	LIST_ENTRY(uboot_var) next;
};

LIST_HEAD(ubootvarlist, uboot_var);

struct swupdate_cfg {
	char name[SWUPDATE_GENERAL_STRING_SIZE];
	int valid;
	char version[SWUPDATE_GENERAL_STRING_SIZE];
	char software_set[SWUPDATE_GENERAL_STRING_SIZE];
	char running_mode[SWUPDATE_GENERAL_STRING_SIZE];
	struct hw_type hw;
	struct hwlist hardware;
	struct swver installed_sw_list;
	struct imglist images;
	struct imglist partitions;
	struct imglist scripts;
	struct ubootvarlist uboot;
	void *dgst;	/* Structure for signed images */
};

#define SEARCH_FILE(type, list, found, offs) do { \
	if (!found) { \
		type *p; \
		for (p = list.lh_first; p != NULL; \
			p = p->next.le_next) { \
			if (strcmp(p->fname, fdh.filename) == 0) { \
				found = 1; \
				p->offset = offs; \
				p->provided = 1; \
				p->size = fdh.size; \
				break; \
			} \
		} \
	} \
} while(0)

off_t extract_sw_description(int fd, const char *descfile, off_t start);
int cpio_scan(int fd, struct swupdate_cfg *cfg, off_t start);

#endif
