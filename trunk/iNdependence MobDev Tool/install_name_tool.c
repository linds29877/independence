/*
 * Copyright (c) 2001 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include "stuff/errors.h"
#include "stuff/breakout.h"
#include "stuff/round.h"
#include "stuff/allocate.h"
#include "install_name_tool.h"


static void process(
    struct arch *archs,
    unsigned long narchs);

static void write_on_input(
    struct arch *archs,
    unsigned long narchs,
    char *input);

static void update_load_commands(
    struct arch *arch,
    unsigned long *header_size);

/* the argument to the -id option */
static const char *id = NULL;

/* the arguments to the -change options */
struct changes {
    char *old;
    char *new;
};
static struct changes *changes = NULL;
static unsigned long nchanges = 0;

/*
 * This is a pointer to an array of the original header sizes (mach header and
 * load commands) for each architecture which is used when we are writing on the
 * input file.
 */
static unsigned long *arch_header_sizes = NULL;

/* Used to change the install name for a dylib */
int changeInstallName(const char *newname, const char *dylib)
{

	if ( (newname == NULL) || (dylib == NULL) ) {
		return -1;
	}

	id = newname;

    struct arch *archs = NULL;
    unsigned long narchs = 0;

	breakout(dylib, &archs, &narchs, FALSE);

	if (errors) return -1;

	checkout(archs, narchs);

	if (errors) return -1;

	arch_header_sizes = allocate(narchs * sizeof(unsigned long));

	process(archs, narchs);

	if (errors) return -1;

	write_on_input(archs, narchs, dylib);

	if (errors) return -1;

	return 0;
}

static
void
process(
struct arch *archs,
unsigned long narchs)
{
    unsigned long i;
    struct object *object;

	for(i = 0; i < narchs; i++){
	    if(archs[i].type == OFILE_Mach_O){
		object = archs[i].object;
		if(object->mh_filetype == MH_DYLIB_STUB)
		    fatal("input file: %s is Mach-O dynamic shared library stub"
			  " file and can't be changed", archs[i].file_name);
		update_load_commands(archs + i, arch_header_sizes + i);
	    }
	    else{
		error("input file: %s is not a Mach-O file",archs[i].file_name);
		return;
	    }
	}
}

/*
 * write_on_input() takes the modified archs and writes the load commands
 * directly into the input file.
 */
static
void
write_on_input(
struct arch *archs,
unsigned long narchs,
char *input)
{
    int fd;
    unsigned long i, offset, size, headers_size;
    char *headers;
    struct mach_header *mh;
    struct mach_header_64 *mh64;
    struct load_command *lc;
    enum byte_sex host_byte_sex;

	host_byte_sex = get_host_byte_sex();

	fd = open(input, O_WRONLY, 0);
	if(fd == -1)
	    system_error("can't open input file: %s for writing", input);

	for(i = 0; i < narchs; i++){
	    if(archs[i].fat_arch != NULL)
		offset = archs[i].fat_arch->offset;
	    else
		offset = 0;
	    if(lseek(fd, offset, SEEK_SET) == -1)
		system_error("can't lseek to offset: %lu in file: %s for "
			     "writing", offset, input);
	    /*
	     * Since the new headers may be smaller than the old headers and
	     * we want to make sure any old unused bytes are zero in the file
	     * we allocate the size of the original headers into a buffer and
	     * zero it out. Then copy the new headers into the buffer and write
	     * out the size of the original headers to the file.
	     */
	    if(archs[i].object->mh != NULL){
		headers_size = sizeof(struct mach_header) +
			       archs[i].object->mh->sizeofcmds;
	    }
	    else{
		headers_size = sizeof(struct mach_header_64) +
			       archs[i].object->mh64->sizeofcmds;
	    }
	    if(arch_header_sizes[i] > headers_size)
		size = arch_header_sizes[i];
	    else
		size = headers_size;
	    headers = allocate(size);
	    memset(headers, '\0', size);

	    if(archs[i].object->mh != NULL){
		mh = (struct mach_header *)headers;
		lc = (struct load_command *)(headers +
					     sizeof(struct mach_header));
		*mh = *(archs[i].object->mh);
		memcpy(lc, archs[i].object->load_commands, mh->sizeofcmds);
		if(archs[i].object->object_byte_sex != host_byte_sex)
		    if(swap_object_headers(mh, lc) == FALSE)
			fatal("internal error: swap_object_headers() failed");
	    }
	    else{
		mh64 = (struct mach_header_64 *)headers;
		lc = (struct load_command *)(headers +
					     sizeof(struct mach_header_64));
		*mh64 = *(archs[i].object->mh64);
		memcpy(lc, archs[i].object->load_commands, mh64->sizeofcmds);
		if(archs[i].object->object_byte_sex != host_byte_sex)
		    if(swap_object_headers(mh64, lc) == FALSE)
			fatal("internal error: swap_object_headers() failed");
	    }

	    if(write(fd, headers, size) != (int)size)
		system_error("can't write new headers in file: %s", input);

	    free(headers);
	}
	if(close(fd) == -1)
	    system_error("can't close written on input file: %s", input);
}

/*
 * update_load_commands() changes the install names the LC_LOAD_DYLIB,
 * LC_LOAD_WEAK_DYLIB and LC_PREBOUND_DYLIB commands for the specified arch.
 */
static
void
update_load_commands(
struct arch *arch,
unsigned long *header_size)
{
    unsigned long i, j, new_sizeofcmds, new_size,
		  linked_modules_size, ncmds, sizeof_mach_header, cmd_round;
    uint64_t low_fileoff;
    struct load_command *lc1, *lc2, *new_load_commands;
    struct dylib_command *dl_load1, *dl_load2, *dl_id1, *dl_id2;
    struct prebound_dylib_command *pbdylib1, *pbdylib2;
    char *dylib_name1, *dylib_name2, *arch_name, *linked_modules1,
	 *linked_modules2;
    struct segment_command *sg;
    struct segment_command_64 *sg64;
    struct section *s;
    struct section_64 *s64;
    struct arch_flag arch_flag;

	/*
	 * Make a pass through the load commands and figure out what the new
	 * size of the the commands needs to be and how much room there is for
	 * them.
	 */
	if(arch->object->mh != NULL){
	    new_sizeofcmds = arch->object->mh->sizeofcmds;
	    ncmds = arch->object->mh->ncmds;
	    sizeof_mach_header = sizeof(struct mach_header);
	    cmd_round = 4;
	}
	else{
	    new_sizeofcmds = arch->object->mh64->sizeofcmds;
	    ncmds = arch->object->mh64->ncmds;
	    sizeof_mach_header = sizeof(struct mach_header_64);
	    cmd_round = 8;
	}
	low_fileoff = ULLONG_MAX;
	lc1 = arch->object->load_commands;
	for(i = 0; i < ncmds; i++){
	    switch(lc1->cmd){
	    case LC_ID_DYLIB:
		dl_id1 = (struct dylib_command *)lc1;
		dylib_name1 = (char *)dl_id1 + dl_id1->dylib.name.offset;
		if(id != NULL){
		    new_size = sizeof(struct dylib_command) +
			       round(strlen(id) + 1, cmd_round);
		    new_sizeofcmds += (new_size - dl_id1->cmdsize);
		}
		break;

	    case LC_LOAD_DYLIB:
	    case LC_LOAD_WEAK_DYLIB:
		dl_load1 = (struct dylib_command *)lc1;
		dylib_name1 = (char *)dl_load1 + dl_load1->dylib.name.offset;
		for(j = 0; j < nchanges; j++){
		    if(strcmp(changes[j].old, dylib_name1) == 0){
			new_size = sizeof(struct dylib_command) +
				   round(strlen(changes[j].new) + 1,
					 cmd_round);
			new_sizeofcmds += (new_size - dl_load1->cmdsize);
			break;
		    }
		}
		break;

	    case LC_PREBOUND_DYLIB:
		pbdylib1 = (struct prebound_dylib_command *)lc1;
		dylib_name1 = (char *)pbdylib1 + pbdylib1->name.offset;
		for(j = 0; j < nchanges; j++){
		    if(strcmp(changes[j].old, dylib_name1) == 0){
			linked_modules_size = pbdylib1->cmdsize - (
				sizeof(struct prebound_dylib_command) +
				round(strlen(dylib_name1) + 1, cmd_round));
			new_size = sizeof(struct prebound_dylib_command) +
				   round(strlen(changes[j].new) + 1,
					 cmd_round) +
				   linked_modules_size;
			new_sizeofcmds += (new_size - pbdylib1->cmdsize);
			break;
		    }
		}
		break;

	    case LC_SEGMENT:
		sg = (struct segment_command *)lc1;
		s = (struct section *)
		    ((char *)sg + sizeof(struct segment_command));
		if(sg->nsects != 0){
		    for(j = 0; j < sg->nsects; j++){
			if(s->size != 0 &&
			   (s->flags & S_ZEROFILL) != S_ZEROFILL &&
			   s->offset < low_fileoff)
			    low_fileoff = s->offset;
			s++;
		    }
		}
		else{
		    if(sg->filesize != 0 && sg->fileoff < low_fileoff)
			low_fileoff = sg->fileoff;
		}
		break;

	    case LC_SEGMENT_64:
		sg64 = (struct segment_command_64 *)lc1;
		s64 = (struct section_64 *)
		    ((char *)sg64 + sizeof(struct segment_command_64));
		if(sg64->nsects != 0){
		    for(j = 0; j < sg64->nsects; j++){
			if(s64->size != 0 &&
			   (s64->flags & S_ZEROFILL) != S_ZEROFILL &&
			   s64->offset < low_fileoff)
			    low_fileoff = s64->offset;
			s64++;
		    }
		}
		else{
		    if(sg64->filesize != 0 && sg64->fileoff < low_fileoff)
			low_fileoff = sg64->fileoff;
		}
		break;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}

	if(new_sizeofcmds + sizeof_mach_header > low_fileoff){
	    arch_flag.cputype = arch->object->mh->cputype;
	    arch_flag.cpusubtype = arch->object->mh->cpusubtype;
	    set_arch_flag_name(&arch_flag);
	    arch_name = arch_flag.name;
	    error("changing install names can't be redone for: %s (for "
		  "architecture %s) because larger updated load commands do "
		  "not fit (the program must be relinked)", arch->file_name,
		  arch_name);
	    return;
	}

	/*
	 * Allocate space for the new load commands and zero it out so any holes
	 * will be zero bytes.  Note this may be smaller than the original size
	 * of the load commands.
	 */
	new_load_commands = allocate(new_sizeofcmds);
	memset(new_load_commands, '\0', new_sizeofcmds);

	/*
	 * Fill in the new load commands by copying in the non-modified
	 * commands and updating ones with install name changes.
	 */
	lc1 = arch->object->load_commands;
	lc2 = new_load_commands;
	for(i = 0; i < ncmds; i++){
	    switch(lc1->cmd){
	    case LC_ID_DYLIB:
		if(id != NULL){
		    memcpy(lc2, lc1, sizeof(struct dylib_command));
		    dl_id2 = (struct dylib_command *)lc2;
		    dl_id2->cmdsize = sizeof(struct dylib_command) +
				     round(strlen(id) + 1, cmd_round);
		    dl_id2->dylib.name.offset = sizeof(struct dylib_command);
		    dylib_name2 = (char *)dl_id2 + dl_id2->dylib.name.offset;
		    strcpy(dylib_name2, id);
		}
		else{
		    memcpy(lc2, lc1, lc1->cmdsize);
		}
		break;

	    case LC_LOAD_DYLIB:
	    case LC_LOAD_WEAK_DYLIB:
		dl_load1 = (struct dylib_command *)lc1;
		dylib_name1 = (char *)dl_load1 + dl_load1->dylib.name.offset;
		for(j = 0; j < nchanges; j++){
		    if(strcmp(changes[j].old, dylib_name1) == 0){
			memcpy(lc2, lc1, sizeof(struct dylib_command));
			dl_load2 = (struct dylib_command *)lc2;
			dl_load2->cmdsize = sizeof(struct dylib_command) +
					    round(strlen(changes[j].new) + 1,
						  cmd_round);
			dl_load2->dylib.name.offset =
			    sizeof(struct dylib_command);
			dylib_name2 = (char *)dl_load2 +
				      dl_load2->dylib.name.offset;
			strcpy(dylib_name2, changes[j].new);
			break;
		    }
		}
		if(j >= nchanges){
		    memcpy(lc2, lc1, lc1->cmdsize);
		}
		break;

	    case LC_PREBOUND_DYLIB:
		pbdylib1 = (struct prebound_dylib_command *)lc1;
		dylib_name1 = (char *)pbdylib1 + pbdylib1->name.offset;
		for(j = 0; j < nchanges; j++){
		    if(strcmp(changes[j].old, dylib_name1) == 0){
			memcpy(lc2, lc1, sizeof(struct prebound_dylib_command));
			pbdylib2 = (struct prebound_dylib_command *)lc2;
			linked_modules_size = pbdylib1->cmdsize - (
			    sizeof(struct prebound_dylib_command) +
			    round(strlen(dylib_name1) + 1, cmd_round));
			pbdylib2->cmdsize =
			    sizeof(struct prebound_dylib_command) +
			    round(strlen(changes[j].new) + 1, cmd_round) +
			    linked_modules_size;

			pbdylib2->name.offset =
			    sizeof(struct prebound_dylib_command);
			dylib_name2 = (char *)pbdylib2 +
				      pbdylib2->name.offset;
			strcpy(dylib_name2, changes[j].new);
			
			pbdylib2->linked_modules.offset = 
			    sizeof(struct prebound_dylib_command) +
			    round(strlen(changes[j].new) + 1, cmd_round);
			linked_modules1 = (char *)pbdylib1 +
					  pbdylib1->linked_modules.offset;
			linked_modules2 = (char *)pbdylib2 +
					  pbdylib2->linked_modules.offset;
			memcpy(linked_modules2, linked_modules1,
			       linked_modules_size);
			break;
		    }
		}
		if(j >= nchanges){
		    memcpy(lc2, lc1, lc1->cmdsize);
		}
		break;

	    default:
		memcpy(lc2, lc1, lc1->cmdsize);
		break;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	    lc2 = (struct load_command *)((char *)lc2 + lc2->cmdsize);
	}

	/*
	 * Finally copy the updated load commands over the existing load
	 * commands. Since the headers could be smaller we save away the old
	 * header_size (for use when writing on the input) and also put zero
	 * bytes on the part that is no longer used for headers.
	 */
	if(arch->object->mh != NULL){
	    *header_size = sizeof(struct mach_header) +
			   arch->object->mh->sizeofcmds;
	    if(new_sizeofcmds < arch->object->mh->sizeofcmds){
		memset(((char *)arch->object->load_commands) + new_sizeofcmds,
		       '\0', arch->object->mh->sizeofcmds - new_sizeofcmds);
	    }
	    memcpy(arch->object->load_commands, new_load_commands,
		   new_sizeofcmds);
	    arch->object->mh->sizeofcmds = new_sizeofcmds;
	}
	else{
	    *header_size = sizeof(struct mach_header_64) +
			   arch->object->mh64->sizeofcmds;
	    if(new_sizeofcmds < arch->object->mh64->sizeofcmds){
		memset(((char *)arch->object->load_commands) + new_sizeofcmds,
		       '\0', arch->object->mh64->sizeofcmds - new_sizeofcmds);
	    }
	    memcpy(arch->object->load_commands, new_load_commands,
		   new_sizeofcmds);
	    arch->object->mh64->sizeofcmds = new_sizeofcmds;
	}

	free(new_load_commands);

	/* reset the pointers into the load commands */
	lc1 = arch->object->load_commands;
	for(i = 0; i < ncmds; i++){
	    switch(lc1->cmd){
	    case LC_SYMTAB:
		arch->object->st = (struct symtab_command *)lc1;
	        break;
	    case LC_DYSYMTAB:
		arch->object->dyst = (struct dysymtab_command *)lc1;
		break;
	    case LC_TWOLEVEL_HINTS:
		arch->object->hints_cmd = (struct twolevel_hints_command *)lc1;
		break;
	    case LC_PREBIND_CKSUM:
		arch->object->cs = (struct prebind_cksum_command *)lc1;
		break;
	    case LC_SEGMENT:
		sg = (struct segment_command *)lc1;
		if(strcmp(sg->segname, SEG_LINKEDIT) == 0)
		    arch->object->seg_linkedit = sg;
	    case LC_SEGMENT_64:
		sg64 = (struct segment_command_64 *)lc1;
		if(strcmp(sg64->segname, SEG_LINKEDIT) == 0)
		    arch->object->seg_linkedit64 = sg64;
	    }
	    lc1 = (struct load_command *)((char *)lc1 + lc1->cmdsize);
	}
}