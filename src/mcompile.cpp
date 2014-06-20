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

#include <stdbool.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../liblinear/linear.h"
#include "sceadan.h"

/*
 * Compile a liblinear model to C
 */

int main(int argc,char **argv)
{
    if(argc!=3){
        fprintf(stderr,"usage: mcompile <modelfile> <outputfile>\n");
        exit(1);
    }
    fprintf(stderr,"Loading %s\n",argv[1]);
    const struct model* model = load_model(argv[1]);
    if(!model){
        perror(argv[1]);
        exit(1);
    }
    FILE *f = fopen(argv[2],"w");
    sceadan_model_dump(model,f);
    fclose(f);
    return(0);
}

