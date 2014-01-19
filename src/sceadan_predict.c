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

#include <errno.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#ifdef HAVE_LINEAR_H
#include <linear.h>
#endif
#ifdef HAVE_LIBLINEAR_LINEAR_H
#include <liblinear/linear.h>
#endif

#include "sceadan_predict.h"
#include "sceadan_processblk.h"

#define MODEL ("model.ucv-bcv.20130509.c256.s2.e005")

#define RANDOMNESS_THRESHOLD (.995)
#define UCV_CONST_THRESHOLD  (.5)
#define BCV_CONST_THRESHOLD  (.5)


static int
do_predict (
	const ucv_t               ucv,
	const bcv_t               bcv,
	const mfv_t        *const mfv,
	      file_type_e  *const file_type,
	      struct feature_node *x, //const x,
	      struct model*      model_
) {

  //int nr_class=get_nr_class(model_);
	//double *prob_estimates=NULL;
	int /*j,*/ n;
	int nr_feature=get_nr_feature(model_);
	if(model_->bias>=0)
		n=nr_feature+1;
	else
		n=nr_feature;

	{
		int i = 0;

		int k, j;
		for (k = 0 ; k < n_unigram; k++, i++) {
			x[i].index = i + 1;
			x[i].value = ucv[k].avg;
		}

		for (k = 0; k < n_unigram; k++)
			for (j = 0; j < n_unigram; j++) {
				x[i].index = i + 1;
				x[i].value = bcv[k][j].avg;
				i++;
			}

		if(model_->bias>=0)
		{
			x[i].index = n;
			x[i].value = model_->bias;
			i++;
		}
		x[i].index = -1;

		{
                  int predict_label = predict(model_,x);
			//fprintf(stdout,"%d\n",predict_label);
                  *file_type = (file_type_e) predict_label;
			// TODO should not be 36 (random)
		}
	}

	return 0;
}

int
predict_liblin (
	const ucv_t        ucv,
	const bcv_t        bcv,
	      mfv_t *const mfv,
	      file_type_e *const file_type
) {
	// TODO floating point comparison
	if (mfv->item_entropy > RANDOMNESS_THRESHOLD) {
		*file_type = RAND;
		return 0;
	}

	int i, j;
	for (i = 0; i < n_unigram; i++) {
		// TODO floating point comparison
		if (ucv[i].avg > UCV_CONST_THRESHOLD) {
				*file_type = UCV_CONST;
				mfv->const_chr[0] = i;
				return 0;
		}
		for (j = 0; j < n_unigram; j++)
			// TODO floating point comparison
			if (bcv[i][j].avg > BCV_CONST_THRESHOLD) {
				*file_type = BCV_CONST;
				mfv->const_chr[0] = i;
				mfv->const_chr[1] = j;
				return 0;
			}
	}
	
	
	// Load model
	struct feature_node *x;
	const int max_nr_attr = n_bigram + n_unigram + 3;//+ /*20*/ 17 /*6 + 2 + 9*/;
	struct model* model_;

	model_=load_model(MODEL);
	if((model_)==0)
	{
		fprintf(stderr,"can't open model file %s\n","");
		return 1;
	}

	x = (struct feature_node *) malloc(max_nr_attr*sizeof(struct feature_node));
	do_predict(ucv, bcv, mfv, file_type,x, model_);
	free_and_destroy_model(&model_);
	free(x);
	return 0;
}
