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

// -fprofile-arcs
// -fprofile-generate
// -fprofile-use

//-fbranch-probabilities
//-fvpt
//-funroll-loops
//-fpeel-loops
//-ftracer

//-fprofile-values
//-frename-registers
//-fmove-loop-invariants
//-funswitch-loops
//-ffunction-sections
//-fdata-sections
//-fbranch-target-load-optimize
//-fbranch-target-load-optimize2
//-fsection-anchors


/* STANDARD INCLUDES */                                  /* STANDARD INCLUDES */
#include <stdbool.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
/* END STANDARD INCLUDES */                          /* END STANDARD INCLUDES */

#include "file_type.h"
#include "ngram.h"
#include "sceadan_processblk.h"

/* MACRO CONSTANTS */                                      /* MACRO CONSTANTS */
#define TIME_BUF_SZ (80)

#ifdef VERBOSE
#define VERBOSE_OUTPUT(S) ({ \
		S \
})
#else
#define VERBOSE_OUTPUT(S) { \
	if (__builtin_expect (verbose_flag, false)) { \
		S \
	} \
}
#endif
/* END MACRO CONSTANTS */                              /* END MACRO CONSTANTS */



/* TYPEDEFS */                                                    /* TYPEDEFS */
// http://www.newty.de/fpt/fpt.html
typedef void (*const parse_f) (
	const char         arg[],
	      char **const endptr,
	      void  *const res
);
/* END TYPEDEFS */                                            /* END TYPEDEFS */



/* GLOBAL VARIABLES */                                    /* GLOBAL VARIABLES */

/* Flag set by ‘--verbose’. */
/*static*/          int verbose_flag;

/* Value set by '-c', or '--concurrency-factor'. */
/* Not used, since this version does not support multi-thread */
/*static*/ unsigned int concurrency_factor;

/* END GLOBAL VARIABLES */                            /* END GLOBAL VARIABLES */





/* TYPEDEFS FOR I/O */
typedef int (*input_f) (
	const          char *const input_target,
	const unsigned int         block_factor,
	const output_f             output_vectors,
	      FILE          *const outs[3],
	      file_type_e          file_type
);
/* END TYPEDEFS FOR I/O */



/* FUNCTIONS FOR HANDLING COMMAND LINE PARAMETERS */

static void
print_usage (const char subsection) {
	switch (subsection) {

	case 'h':
		puts ("help mode: help mode");
		puts ("sceadan [-c <#>] [-i <f|d|i|b>] [-o <s|f|b|c>] [-t <0-42>] [--verbose|--brief] <input target> <block factor>");
		puts ("\tstandard usage");
		puts ("sceadan -h <c|i|o|t|v|b|g|f|h>");
		puts ("\tmore help !!!");
		break;

	case 'c':
		puts ("help mode: concurrency factor");
		puts ("[-c <(non-negative integer)>]");
		puts ("concurrency factor");
		puts ("default: 0");
		puts ("\t0 : auto");
		puts ("\t# : number of threads (not supported)");
		break;

	case 'i':
		puts ("help mode: input mode");
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
		puts ("help mode: output mode");
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
		puts ("help mode: target label");
		puts ("[-t <0-42>]");
		puts ("type label");
		puts ("default: 0");
		puts ("\t 0 : unclassified      - predict mode");
		puts ("\t 1 : text              - train   mode");
		puts ("\t 2 : csv               - train   mode");
		puts ("\t 3 : log               - train   mode");
		puts ("\t 4 : html              - train   mode");
		puts ("\t 5 : xml               - train   mode");
		puts ("\t 6 : css               - train   mode");
		puts ("\t 7 : js                - train   mode");
		puts ("\t 8 : json              - train   mode");
		puts ("\t 9 : jpg               - train   mode");
		puts ("\t10 : png               - train   mode");
		puts ("\t11 : gif               - train   mode");
		puts ("\t12 : tif               - train   mode");
		puts ("\t13 : gz                - train   mode");
		puts ("\t14 : zip               - train   mode");
		puts ("\t15 : bz2               - train   mode");
		puts ("\t16 : pdf               - train   mode");
		puts ("\t17 : doc               - train   mode");
		puts ("\t18 : xls               - train   mode");
		puts ("\t19 : ppt               - train   mode");
		puts ("\t20 : docx              - train   mode");
		puts ("\t21 : xlsx              - train   mode");
		puts ("\t22 : pptx              - train   mode");
		puts ("\t23 : mp3               - train   mode");
		puts ("\t24 : m4a               - train   mode");
		puts ("\t25 : mp4               - train   mode");
		puts ("\t26 : avi               - train   mode");
		puts ("\t27 : wmv               - train   mode");
		puts ("\t28 : flv               - train   mode");
		puts ("\t29 : b64               - train   mode");
		puts ("\t30 : a85               - train   mode");
		puts ("\t31 : url               - train   mode");
		puts ("\t32 : fat               - train   mode");
		puts ("\t33 : ntfs              - train   mode");
		puts ("\t34 : ext3              - train   mode");
		puts ("\t35 : aes  (or rand)    - train   mode");
		puts ("\t36 : rand (hi entropy) - train   mode");
		puts ("\t37 : ps                - train   mode");
		puts ("\t38 : pps               - train   mode");
		puts ("\t39 : bmp               - train   mode");
		puts ("\t40 : java              - train   mode");
		puts ("\t41 : ucv const         - train   mode");
		puts ("\t42 : bcv const         - train   mode");
		break;

	case 'v':
	case 'b':
		puts ("help mode: verbose mode or brief mode");
		puts ("[--verbose|--brief]");
		puts ("verbosity");
		puts ("default: brief");
		break;

	case 'g':
		puts ("help mode: input target");
		puts ("<(see input mode)>");
		puts ("input target");
		puts ("default: (none)");
		break;

	case 'f':
		puts ("help mode: block factor");
		puts ("<(unsigned integer)>");
		puts ("block factor");
		puts ("default: (none)");
		puts ("\t0 : container mode");
		puts ("\t# : block     mode - with specified block size");
		break;

	default:
		fprintf (stderr, "invalid help option\n");
		abort ();
	}
}

static void
parse_uint (
	const char         arg[],
	      char **const endptr,
	      void  *const res
) {
	const unsigned long tmp = strtoul (arg, endptr, 0);
	if ( (tmp > UINT_MAX))
		abort ();

	*((unsigned int *) res) = tmp;
}

static void
parse_input_arg (
	const char            arg[],
	const char    **const endptr,
	      input_f  *const res
) {
	switch (arg[0]) {

	case 'f':
		*res = &process_file;
		printf ("\tinput-mode        : file\n");
		break;

	case 'd':
		*res = &process_dir;
		printf ("\tinput-mode        : directory\n");
		break;

	case 'i':
//		*res = &process_img;
		printf ("\tinput-mode        : image\n");
		break;

	case 'b':
//		*res = &process_db;
		printf ("\tinput-mode        : database\n");
		break;

	default:
		puts ("invalid input mode");
		puts ("try sceadan -h i");
		abort ();
	} // end switch input type

	*endptr = arg + 1;
}

static void
parse_output_arg (
	const char             arg[],
	const char     **const endptr,
	      output_f  *const res
) {
	switch (arg[0]) {

	case 's':
//		*res = &output_vectors_to_stdout;
		printf ("\toutput-mode       : stdout\n");
		break;

	case 'f':
//		*res = &output_vectors_to_files;
		printf ("\toutput-mode       : files\n");
		break;

	case 'b':
//		*res = &output_vectors_to_db;
		printf ("\toutput-mode       : database\n");
		break;

	default:
		printf ("invalid output mode\n");
		puts ("try sceadan -h o");
		abort ();
	} // end switch output type

	*endptr = arg + 1;
}

static void
parse_help_arg (
	const char            arg[],
	const char    **const endptr,
	      char     *const res
) {
	*res = arg[0];
	*endptr = arg + 1;
}

static void
parse_arg (
	const char           arg[],
	      void    *const res,
	const parse_f        parse
) {
	// don't parse empty arg
	if ((*arg == '\0')) {
		printf ("empty parameter\n");
		puts ("try sceadan -h h");
		abort ();
	}

	char *endptr;
	parse (arg, &endptr, res);

	// parse entire arg
	if ( (*endptr != '\0')) {
		printf ("could not complete parsing parameter\n");
		puts ("try sceadan -h h");
		abort ();
	}
}

static void
init_defaults (
	output_f    *const do_output,
	input_f     *const do_input,
	file_type_e *const file_type
) {
	//*do_output = &output_vectors_to_files;
	*do_output = &output_competition;
	*do_input  = &process_dir;
	*file_type = UNCLASSIFIED;
}

// http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
// http://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
static void
parse_args (
	const int                     argc,
	      char             *const argv[],
	const char            **const input_target,
	      unsigned int     *const block_factor,
	      output_f         *const do_output,
	      input_f          *const do_input,
	      file_type_e      *const file_type
) {
	char help_chr;

	init_defaults (do_output, do_input, file_type);

//	printf ("optional ARGV-elements:\n");

	while (true) {
		static struct option long_options[] =	{
			// TODO help

			/* These options set a flag. */
			{"verbose",            no_argument,       &verbose_flag, 1},
			{"brief",              no_argument,       &verbose_flag, 0},

			/* These options don't set a flag.
			   We distinguish them by their indices. */
			{"concurrency-factor", required_argument, 0,             'c'},
			{"input-type",         required_argument, 0,             'i'},
			{"output-type",        required_argument, 0,             'o'},
			{"file-type",          required_argument, 0,             't'},
			{"help",               required_argument, 0,             'h'},
			{0,                    0,                 0,             0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		const int c = getopt_long (argc, argv, "c:i:o:t:h:",
		                           long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c) {
		case 0:
			/* If this option set a flag, do nothing else now. */
			if ( (long_options[option_index].flag != 0))
				break;
			break;

		case 'c':
			parse_arg (optarg, &concurrency_factor, &parse_uint);
			break;

		case 'i':
			parse_arg (optarg, do_input, (parse_f) &parse_input_arg);
			break;

		case 'o':
			parse_arg (optarg, do_output, (parse_f) &parse_output_arg);
			break;

		case 't':
			parse_arg (optarg, file_type, (parse_f) &parse_uint);
			break;

		case 'h':
			parse_arg (optarg, &help_chr, (parse_f) &parse_help_arg);
			print_usage (help_chr);
			exit (EXIT_SUCCESS);
			//break;

		case '?':
			/* getopt_long already printed an error message. */
			break;

		default:
			printf ("some kind of parse fail\n");
			puts ("try sceadan -h h");
			abort ();
		} // end switch
	} // end (apparently) infinite while-loop

	/* Instead of reporting ‘--verbose’
	   and ‘--brief’ as they are encountered,
	   we report the final status resulting from them. */
	#ifdef VERBOSE
	if ( (verbose_flag))
		puts ("RUNNING IN PERMANENT VERBOSE MODE");
	else {
		puts ("This version was compile in 'permanent verbose' mode,");
		puts ("so the verbose flag is repetitively redundant...");
		puts ("and a little verbose !!!");
	}
	#else
	VERBOSE_OUTPUT(
		puts ("verbose flag is set");
	)
	#endif


	if ( (optind != argc - 2)) {
		printf ("incorrect number of positional parameters\n");
		puts ("try sceadan -h h");
		abort ();
	}

	*input_target = argv[optind++];

	parse_arg (argv[optind], block_factor, parse_uint);
}

int
main (
	const int         argc,
	      char *const argv[]
) {
	const char         *input_target;
	      unsigned int  block_factor;
	      output_f      do_output;
	      input_f       do_input;
	      file_type_e   file_type;
	parse_args (argc, argv,
		&input_target, &block_factor,
		&do_output, &do_input, &file_type
	);

	time_t rawtime;
	struct tm * timeinfo;
	char buf [TIME_BUF_SZ];

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	{
		const size_t strsz = strftime (buf + 9, TIME_BUF_SZ - 9,".%Y%m%d.%H%M%S",timeinfo);
		if ( (strsz == 0 || strsz == TIME_BUF_SZ - 9)) {
			// TODO
			return 1;
		}
	}

	FILE *outs[3];
	switch (file_type) {
	case UNCLASSIFIED:
		// TODO
		break;

	default:
		memcpy (buf + 9 - (sizeof ("./ucv") - 1), "./ucv", sizeof ("./ucv") - 1);

		outs[0] = fopen (buf + 9 - (sizeof ("./ucv") - 1), "w");
		if ( outs[0] == NULL) {
			// TODO
			fprintf (stderr, "fopen fail\n");
			return 1;
		}

		memcpy (buf + 9 - (sizeof ("./bcv") - 1), "./bcv", sizeof ("./bcv") - 1);

		outs[1] = fopen (buf + 9 - (sizeof ("./bcv") - 1), "w");
		if (outs[1] == NULL) {
			// TODO
			return 1;
		}

		memcpy (buf + 9 - (sizeof ("./main") - 1), "./main", sizeof ("./main") - 1);

		outs[2] = fopen (buf + 9 - (sizeof ("./main") - 1), "w");
		if (outs[2] == NULL) {
			// TODO
			return 1;
		}
	}

	const int ret =  (
		do_input (input_target, block_factor, do_output, outs, file_type) == 0
	)
	?       EXIT_SUCCESS
	:       EXIT_FAILURE;

	
	switch (file_type) {
	case UNCLASSIFIED: break;
	default:
		if ((fclose (outs[2]) != 0)) {
			// TODO
			return 1;
		}

		if ( (fclose (outs[1]) != 0)) {
			// TODO
			return 1;
		}

		if ((fclose (outs[0]) != 0)) {
			// TODO
			return 1;
		}
	}

	return ret;
}

/* END FUNCTIONS FOR HANDLING COMMAND LINE PARAMETERS */

