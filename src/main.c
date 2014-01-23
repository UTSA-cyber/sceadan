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

#include "config.h"
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

#include "sceadan.h"

// http://www.newty.de/fpt/fpt.html
typedef void (*const parse_f) ( const char         arg[],
                                char **const endptr,
                                void  *const res );

typedef int (*input_f) ( const          char *const input_target,
                         const unsigned int   block_factor,
                         const output_f       output_vectors,
                         file_type_e          file_type );


/* FUNCTIONS FOR PROCESSING FILES */
static int process_file (const          char path[],
                  const unsigned int  block_factor,
                  const output_f      do_output,
                  file_type_e file_type ) 
{
    return block_factor
	?       process_blocks    (path, block_factor, do_output, file_type)
	:       process_container (path,               do_output, file_type);
}


static int ftw_callback_block_factor      = 0;
static output_f ftw_callback_do_output    = 0;
static file_type_e ftw_callback_file_type = UNCLASSIFIED ;

static int ftw_callback ( const char                fpath[],
                          const struct stat *const sb,
                          const int                 typeflag )
{
    if(typeflag==FTW_F){
        process_file (fpath, ftw_callback_block_factor, ftw_callback_do_output, ftw_callback_file_type);
    }
    return 0;
}

/* FUNCTIONS FOR PROCESSING DIRECTORIES */
// TODO full path vs relevant path may matter
#define FTW_MAXOPENFD 8
static int process_dir ( const          char path[],
              const unsigned int  block_factor,
              const output_f      do_output,
              file_type_e file_type )
{
    ftw_callback_block_factor = block_factor;
    ftw_callback_do_output    = do_output;
    ftw_callback_file_type    = file_type;
    return ftw (path, &ftw_callback, FTW_MAXOPENFD);
}


static void print_usage (const char subsection)
{
    switch (subsection) {

    case 'h':
        puts ("help: help mode");
        puts ("sceadan [-c <#>] [-i <f|d|i|b>] [-o <s|f|b|c>] [-t <0-42>] [--verbose|--brief] <input target> <block factor>");
        puts ("\tstandard usage");
        puts ("sceadan -h <c|i|o|t|v|b|g|f|h>");
        puts ("\tmore help !!!");
        break;

    case 'i':
        puts ("help: input mode");
        puts ("[-i <f|d|i|b>]");
        puts ("input mode");
        puts ("default: d");
        puts ("\tf : file");
        puts ("\t\tskips checking whether the target is a directory");
        puts ("\td : directory");
        puts ("\t\twalk the directoy tree");
        puts ("\t\tprocess each file");
        puts ("\ti : forensics image (not supported)");
        puts ("\t\tprocess the image using TSK");
        puts ("\t\t*** for now, just process the target as a file ***");
        puts ("\tb : database        (not supported)");
        break;

    case 'o':
        puts ("help: output mode");
        puts ("[-o <s|f|b|c>]");
        puts ("output mode");
        puts ("default: f");
        puts ("\ts : stdout      (not supported)");
        puts ("\tf : files       (not supported)");
        puts ("\t\t ucv.<date/time stamp> - unigram count vector");
        puts ("\t\t bcv.<date/time stamp> -  bigram count vector");
        puts ("\t\tmain.<date/time stamp> - main feature  vector");
        puts ("\tb : database    (not supported)");
        puts ("\tc : competition (not supported)");
        break;

    case 't':
        puts ("help: target label");
        puts ("[-t <0-42>]");
        puts ("type label");
        puts ("default: 0");
        for(int i=0;;i++){
            const char *name = sceadan_name_for_type(i);
            if(name) printf("\t%2d : %-18s - %s mode\n",i,name,i==0 ? "predict" : "train  ");
            else break;
        }
        break;

    case 'v':
    case 'b':
        puts ("help: verbose mode or brief mode");
        puts ("[--verbose|--brief]");
        puts ("verbosity");
        puts ("default: brief");
        break;

    case 'g':
        puts ("help: input target");
        puts ("<(see input mode)>");
        puts ("input target");
        puts ("default: (none)");
        break;

    case 'f':
        puts ("help: block factor");
        puts ("<(unsigned integer)>");
        puts ("block factor");
        puts ("default: (none)");
        puts ("\t0 : container mode");
        puts ("\t# : block     mode - with specified block size");
        break;

    default:
        fprintf (stderr, "invalid help option\n");
        exit(-1);
    }
}

static void parse_uint (    const char         arg[],
                            char **const endptr,
                            void  *const res )
{
    *((unsigned int *) res) = strtoul (arg, endptr, 0);
}

static void parse_input_arg ( const char            arg[],
                              const char    **const endptr,
                              input_f  *const res )
{
    switch (arg[0]) {
    case 'f':
        *res = &process_file;
        printf ("\tinput-mode        : file\n");
        break;

    case 'd':
        *res = &process_dir;
        printf ("\tinput-mode        : directory\n");
        break;

    default:
        puts ("invalid input mode");
        puts ("try sceadan -h i");
        exit(-1);
    } 

    *endptr = arg + 1;
}

static void parse_output_arg ( const char             arg[],
                               const char     **const endptr,
                               output_f  *const res    )
{
    switch (arg[0]) {

    case 's':
        printf ("\toutput-mode       : stdout\n");
        break;

    case 'f':
        printf ("\toutput-mode       : files\n");
        break;

    case 'b':
        printf ("\toutput-mode       : database\n");
        break;

    default:
        printf ("invalid output mode\n");
        puts ("try sceadan -h o");
        exit(-1);
    } 

    *endptr = arg + 1;
}

static void parse_help_arg ( const char            arg[],
                             const char    **const endptr,
                             char     *const res )
{
    *res = arg[0];
    *endptr = arg + 1;
}

static void parse_arg ( const char arg[],
                        void    *const res,
                        const parse_f parse )
{
    // don't parse empty arg
    if ((*arg == '\0')) {
        printf ("empty parameter\n");
        puts ("try sceadan -h h");
        exit(-1);
    }

    char *endptr;
    parse (arg, &endptr, res);

    // parse entire arg
    if ( (*endptr != '\0')) {
        printf ("could not complete parsing parameter\n");
        puts ("try sceadan -h h");
        exit(-1);
    }
}

// http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
// http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
static void
parse_args (const int argc,
            char *const argv[],
            const char **const input_target,
            unsigned int     *const block_factor,
            output_f         *const do_output,
            input_f          *const do_input,
            file_type_e      *const file_type )
{
    char help_chr;

    while (true) {
        static struct option long_options[] =	{
            // TODO help
            
            /* These options don't set a flag. We distinguish them by their indices. */
            {"input-type",         required_argument, 0, 'i'},
            {"output-type",        required_argument, 0, 'o'},
            {"file-type",          required_argument, 0, 't'},
            {"help",               required_argument, 0, 'h'},
            {0,                    0,                 0, 0}
        };
        
        /* getopt_long stores the option index here. */
        int option_index = 0;
        
        const int c = getopt_long (argc, argv, "i:o:t:h:", long_options, &option_index);
        
        if (c == -1) break;        /* End of the options. */
        switch (c) {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if ( (long_options[option_index].flag != 0))
                break;
            break;

        case 'i': parse_arg (optarg, do_input, (parse_f) &parse_input_arg); break;
        case 'o': parse_arg (optarg, do_output, (parse_f) &parse_output_arg); break;
        case 't': parse_arg (optarg, file_type, (parse_f) &parse_uint); break;
        case 'h': parse_arg (optarg, &help_chr, (parse_f) &parse_help_arg);
            print_usage (help_chr);
            exit (EXIT_SUCCESS);

        case '?':
            /* getopt_long already printed an error message. */
            break;

        default:
            printf ("some kind of parse fail\n");
            puts ("try sceadan -h h");
            exit(-1);
        } // end switch
    } // end (apparently) infinite while-loop

    /* Instead of reporting ‘--verbose’
     * and ‘--brief’ as they are encountered,
     * we report the final status resulting from them.
     */

    if ( optind != argc - 2) {
        printf("incorrect number of positional parameters\n");
        puts("try sceadan -h h");
        exit(-1);
    }

    *input_target = argv[optind++];
    parse_arg (argv[optind], block_factor, parse_uint);
}

int main (const int argc, char *const argv[])
{
    const char     *input_target=0;
    unsigned int  block_factor;
    output_f      do_output = output_competition;
    input_f       do_input = process_dir;
    file_type_e   file_type = UNCLASSIFIED;

    parse_args (argc, argv,
		&input_target, &block_factor,
		&do_output, &do_input, &file_type );
    
    do_input (input_target, block_factor, do_output, file_type);
    exit(0);
}
