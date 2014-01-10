#ifndef SCEADAN_PREDICT_H
#define SCEADAN_PREDICT_H

#include "file_type.h"
#include "sceadan_processblk.h"

int
predict_liblin (
	const ucv_t        ucv,
	const bcv_t        bcv,
	      mfv_t *const mfv,
	      file_type_e *const file_type
) ;

#endif
