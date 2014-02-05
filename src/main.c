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
 *        - Works with precompiled model (avoids model load times)
 *        - Smaller, cleaner code base.
 *        - sceadan now has a clear API that is separated from file-reading.
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
int    opt_json = 0;
int    opt_train = 0;
int    opt_omit = 0;
int    opt_each = 0;
int    opt_help = 0;
const char *opt_model = 0;

static void do_output(const char *path,uint64_t offset,int file_type )
{
    printf("%-10" PRId64 " %s # %s\n", offset,sceadan_name_for_type(file_type),path);
}



/**
 * ftw() callback to process a file. In this implementation it handles the file whole or block-by-block, prints
 * results with do_output (above), and then returns.
 */
static int process_file(const char path[],
                        const struct stat *const sb, /* ignored */
                        const int typeflag ) 
{
    if(typeflag==FTW_F){
        sceadan *s = sceadan_open(opt_model);
        
        if(opt_json){
            sceadan_dump_json_on_classify(s,opt_json,stdout);
        }

        if(opt_train){
            sceadan_dump_nodes_on_classify(s,opt_train,stdout);
        }

        /* Test the incremental classifier */
        const int fd = (strcmp(path,"-")==0) ? STDIN_FILENO : open(path, O_RDONLY|O_BINARY);
        if (fd<0){perror("open");exit(0);}
        uint8_t   *buf = malloc(block_size);
        if(buf==0){ perror("malloc"); exit(1); }
        
        /* Read the file one block at a time */
        uint64_t offset = 0;
        
        /* See if we are supposed to skip the first block */
        if(opt_omit){
            lseek(fd,block_size,SEEK_SET);
            offset += block_size;
        }
        while(true){
            const ssize_t rd = read(fd, buf, block_size);
            //fprintf(stderr,"fd=%d rd=%d each=%d\n",fd,rd,opt_each);
            if(rd==-1){ perror("read"); exit(0);}
            /* if we read data, update the vectors */
            if(rd>0){
                sceadan_update(s,buf,rd); 
            }
            /* Print the results if we are classifying each block and we read a complete block,
             * or if we are not classifying each block and we read nothing (meaning we are at the end)
             *
             * sceadan_classify() clears the vectors.
             */
            if((opt_each && rd==block_size) ||
               (!opt_each && rd==0)){
                int t = sceadan_classify(s);
                /* print results if not dumping */
                if(!opt_json && !opt_train) do_output(path,offset,t); 
            }
            /* If we read nothing, break out of the loop */
            if(rd==0) break;
            offset += rd;
        }
        free(buf);
        sceadan_close(s);
        if(fd) close(fd);
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
    puts("usage: sceadan_app [options] inputfile");
    puts("where [options] are:");
    printf("infile - file to analyze. Specify '-' to input from stdin\n");
    printf("  -b <size>   - specifies blocksize (default %zd)\n",block_size);
    printf("  -e          - print classification of each block\n");
    printf("  -m <modelfile>   - use modelfile instead of build-in model\n");
    printf("  -x          - omit file headers (the first block)\n");
    printf("  -j <class>  - generate features for <class> and output in JSON format\n");
    printf("  -t <class>  - generate a liblinear training for class <class>\n");
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

static int get_type(const char *name)
{
    int ival = atoi(name);
    if(ival) return ival;
    ival = sceadan_type_for_name(name);
    if(ival>0) return ival;
    fprintf(stderr,"%s: not a valid type name\n",name);
    exit(1);
}

int main (int argc, char *const argv[])
{
    int ch;
    while((ch = getopt(argc,argv,"b:ej:m:t:xh")) != -1){
        switch(ch){
        case 'b': block_size = atoi(optarg); break;
        case 'e': opt_each = 1;break;
        case 'j': opt_json  = get_type(optarg); break;
        case 'm': opt_model = optarg; break;
        case 't': opt_train = get_type(optarg); break;
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
    if(strcmp(argv[0],"-")==0){         /* process stdin */
        process_file("-",0,FTW_F);      /* FTW_F is not correct, but it works with process_file */
    }

    const char *input_target = argv[0];
    process_dir(input_target); /* if input_target is a file, it will be handled as a file */
    exit(0);
}
