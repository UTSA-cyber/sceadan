#ifndef SCEADAN_H
#define SCEADAN_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

// Forward definitions 
struct model;                           // in liblinear

typedef struct sceadan_t sceadan;

/* struct model is defined in liblinear. If you don't have it, it
 * won't generate an error unless it's used (and it won't be).
 */

void sceadan_model_dump(const struct model *,FILE *outfile); // to stdout
sceadan *sceadan_open(const char *model_file,const char *map_file,const char *feature_mask_file); // use 0 for default model precompiled
const struct model *sceadan_model_precompiled(void);
const struct model *sceadan_model_default(void); // from a file
void sceadan_update(sceadan *,const uint8_t *buf,size_t bufsize);
void sceadan_clear(sceadan *s);         // like a close and open
int sceadan_classify(sceadan *);
int sceadan_classify_file(const sceadan *,const char *fname);    // classify a file
int sceadan_classify_buf(const sceadan *,const uint8_t *buf,size_t bufsize);
const char *sceadan_name_for_type(const sceadan *,int type);
int sceadan_type_for_name(const sceadan *,const char *name);
void sceadan_close(sceadan *);
void sceadan_dump_json_on_classify(sceadan *,int file_type,FILE *out); // dump JSON vectors instead of classifying
void sceadan_dump_nodes_on_classify(sceadan *,int file_type,FILE *out); // dump  vectors instead of classifying
void sceadan_set_ngram_mode(sceadan *s,int mode);
void sceadan_build_feature_mask(sceadan *s);
int sceadan_load_feature_mask(sceadan *s,const char *file_name);
int sceadan_dump_feature_mask(sceadan *s,const char *file_name);
int sceadan_reduce_feature(sceadan *s,const char *file_name,int n); // select top n features for each type, and dump resulted feature mask to a file

#define SCEADAN_NGRAM_MODE_DEFAULT 2

__END_DECLS


#endif
