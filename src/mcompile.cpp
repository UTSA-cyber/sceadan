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


#include "config.h"

#include <stdbool.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_LINEAR_H
#include <linear.h>
#endif
#ifdef HAVE_LIBLINEAR_LINEAR_H
#include <liblinear/linear.h>
#endif

/*
 * Compile a liblinear model to C
 */

int weights[] = {1,2,3};
struct model m = {.param =
                  {.solver_type=0,
                   .weight_label = weights
                  },
                  .nr_class = 1};

int main(int argc,char **argv)
{
    struct model* model = 0;
    model = load_model("model.ucv-bcv.20130509.c256.s2.e005");

    puts("#include \"config.h\"");
    puts("#ifdef HAVE_LINEAR_H");
    puts("#include <linear.h>");
    puts("#endif");
    puts("#ifdef HAVE_LIBLINEAR_LINEAR_H");
    puts("#include <liblinear/linear.h>");
    puts("#endif");

    if(model->param.nr_weight){
        printf("static int weight_label[]={");
        for(int i=0;i<model->param.nr_weight;i++){
            if(i>0) putchar(',');
            printf("%d",model->param.weight_label[i]);
            if(i%10==9) printf("\n\t");
        }
        printf("};\n\n");
    }
    if(model->param.nr_weight){
        printf("static double weight[] = {");
        for(int i=0;i<model->param.nr_weight;i++){
            if(i>0) putchar(',');
            printf("%g",model->param.weight[i]);
            if(i%10==9) printf("\n\t");
        }
        printf("};\n\n");
    }

    printf("static int label[] = {");
    for(int i=0;i<model->nr_class;i++){
        printf("%d",model->label[i]);
        if(i<model->nr_class-1) putchar(',');
        if(i%20==19) printf("\n\t");
    }
    printf("};\n");

    printf("static double w[] = {");
    for(int i=0;i<model->nr_feature;i++){
        printf("%g",model->w[i]);
        if(i<model->nr_feature-1) putchar(',');
        if(i%10==9) printf("\n\t");
    }
    printf("};\n");
        
    printf("struct model m = {\n");
    printf("\t.param = {\n");
    printf("\t\t.solver_type=%d,\n",model->param.solver_type);
    printf("\t\t.eps = %g,\n",model->param.eps);
    printf("\t\t.C = %g,\n",model->param.C);
    printf("\t\t.nr_weight = %d,\n",model->param.nr_weight);
    printf("\t\t.weight_label = %s,\n",model->param.nr_weight ? "weight_label" : "0");
    printf("\t\t.weight = %s,\n",model->param.nr_weight ? "weight" : "0");
    printf("\t\t.p = %g},\n",model->param.p);

    printf("\t.nr_class=%d,\n",model->nr_class);
    printf("\t.nr_feature=%d,\n",model->nr_feature);
    printf("\t.w=w,\n");
    printf("\t.label=label,\n");
    printf("\t.bias=%g};\n",model->bias);
    return(0);
}

