/*============================================================
 *
 * Copyright (c) 2012-2013 The University of Texas at San Antonio
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Revision History:
 *
 * 2014 - substantially rewritten by Simson L. Garfinkel
 * 
 * 2013 - cleaned by Lishu Liu, 
 *
 * 2013 - Original version by:
 *        Dr. Nicole Beebe Department of Information Systems
 *        and Cyber Security (nicole.beebe@utsa.edu)
 *        University of Texas at San Antonio,
 *        One UTSA Circle 
 *        San Antonio, Texas 78209
 *    
 *        Laurence Maddox, Department of Computer Science
 *
 *
 ************************************************************/

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

size_t block_size = 512;
int    opt_train = 0;
int    opt_omit = 0;
int    opt_each = 0;
int    opt_help = 0;

static void do_output(const char *path,uint64_t offset,int file_type )
{
    printf("%-10" PRId64 " %s # %s\n", offset,sceadan_name_for_type(file_type),path);
}



static int process_file(const char path[],
                        const struct stat *const sb,
                        const int typeflag )
{
    if(typeflag==FTW_F){
        sceadan *s = sceadan_open(0);
        
        if(opt_train){
            sceadan_dump_vectors_on_classify(s,opt_train,stdout);
        }

        /* Test the incremental classifier */
        const int fd = open(path, O_RDONLY|O_BINARY);
        if (fd<0){perror("open");exit(0);}
        uint8_t   *buf = malloc(block_size);
        if(buf==0){ perror("malloc"); exit(1); }
        
        /* Read the file one block at a time */
        uint64_t offset = 0;
        while(true){
            const ssize_t rd = read (fd, buf, block_size);
            if(rd==-1){ perror("read"); exit(0);}
            if(rd==0) break;
            if(opt_omit==0 || (offset>0 && rd==block_size)){
                sceadan_update(s,buf,rd);
                if(opt_each) do_output(path,offset,sceadan_classify(s));
            }
            offset += rd;
        }
        if(!opt_each) do_output(path,offset,sceadan_classify(s));
        free(buf);
        sceadan_close(s);
    }
    return 0;
}

#define FTW_MAXOPENFD 8
static void process_dir( const          char path[])
{
    ftw (path, &process_file, FTW_MAXOPENFD);
}


void usage(void) __attribute__((noreturn));
void usage()
{
    puts("usage: sceadan_app [options] inputfile [block factor]");
    puts("where [options] are:");
    printf("  -b <size>   - specifies blocksize (default %zd)\n",block_size);
    printf("  -e          - print classification of each block\n");
    printf("  -x          - omit file headers and footers\n");
    printf("  -t <class>  - generate features for <class> and output to stdout\n");
    printf("  -h          - generate help (-hh for more)\n");
    puts("");
    if(opt_help>1){
        puts("Classes");
        for(int i=0;sceadan_name_for_type(i);i++){
            printf("\t%2d : %s\n",i,sceadan_name_for_type(i));
        }
    }
    exit(0);
}

int main (int argc, char *const argv[])
{
    int ch;
    while((ch = getopt(argc,argv,"b:et:xh")) != -1){
        switch(ch){
        case 'b': block_size = atoi(optarg); break;
        case 'e': opt_each = 1;break;
        case 't': opt_train = atoi(optarg); break;
        case 'x': opt_omit = 1; break;
        case 'h': opt_help++; break;
        }
    }
    if (opt_help) usage();

    argc -= optind;
    argv += optind;

    if(block_size<1){
        fprintf(stderr,"Invalid block size\n");
        usage();
    }

    if(argc != 1) usage();
    const char *input_target = argv[0];
    process_dir(input_target); /* if input_target is a file, it will be handled as a file */
    exit(0);
}
