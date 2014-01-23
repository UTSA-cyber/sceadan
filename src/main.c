//===============================================================================================================//

//Copyright (c) 2012-2013 The University of Texas at San Antonio

//This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.

//This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Public License for more details.

//You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

//Written by: 
//Dr. Nicole Beebe and Lishu Liu, Department of Information Systems and Cyber Security (nicole.beebe@utsa.edu)
//Laurence Maddox, Department of Computer Science
//University of Texas at San Antonio
//One UTSA Circle 
//San Antonio, Texas 78209

//===============================================================================================================//

#include "config.h"
#include <inttypes.h>
#include <assert.h>
#include <ftw.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <limits.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif


#include "sceadan.h"

/* Globals for the stand-alone program */

size_t block_factor = 0;

static void do_output(const char *path,uint64_t offset,file_type_e file_type )
{
    printf("%-10" PRId64 " %s # %s\n", offset,sceadan_name_for_type(file_type),path);
}



/* FUNCTIONS FOR PROCESSING FILES */
static void process_file (const char path[]) 
{
    sceadan *s = sceadan_open(0);

    /* Test the single-file classifier */
    if(block_factor==0){
        do_output(path,0,sceadan_classify_file(s,path));
        sceadan_close(s);
        return;
    }

    /* Test the incremental classifier */
    const int fd = open(path, O_RDONLY|O_BINARY);
    if (fd<0){perror("open");exit(0);}
    uint8_t   *buf = malloc(block_factor);
    if(buf==0){ perror("malloc"); exit(1); }

    /* Read the file one block at a time */
    uint64_t offset = 0;
    while(true){
        const ssize_t rd = read (fd, buf, block_factor);
        if(rd==-1){ perror("read"); exit(0);}
        if(rd==0) break;
        do_output(path,offset,sceadan_classify_buf(s,buf,rd));
        offset += rd;
    }
    free(buf);
    sceadan_close(s);
}


static int ftw_callback( const char                fpath[],
                          const struct stat *const sb,
                          const int                 typeflag )
{
    if(typeflag==FTW_F) process_file (fpath);
    return 0;
}

#define FTW_MAXOPENFD 8
static void process_dir( const          char path[])
{
    ftw (path, &ftw_callback, FTW_MAXOPENFD);
}


void usage(void) __attribute__((noreturn));
void usage()
{
    printf("usage: sceadan_app [options] inputfile [block factor]\n");
    exit(0);
}

int main (int argc, char *const argv[])
{
    int ch;
    while((ch = getopt(argc,argv,"h")) != -1){
        switch(ch){
        case 'h':
            usage();
            exit(0);
        }
    }
    argc -= optind;
    argv += optind;

    if(argc < 1) usage();
    const char *input_target = argv[0];

    argc--;
    argv++;
    if(argc >= 1){
        block_factor = atoi(argv[0]);
        argc--;
        argv++;
    }

    if(argc!=0) usage();
    process_dir(input_target); /* if input_target is a file, it will be handled as a file */
    exit(0);
}
