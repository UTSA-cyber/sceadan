#ifndef SCEADAN_H
#define SCEADAN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <sys/types.h>

struct sceadan_t {
    const struct model *model;
};
typedef struct sceadan_t sceadan;


sceadan *sceadan_open(const char *moden_name); // use 0 for default model precompiled
const struct model *sceadan_model_precompiled(void);
const struct model *sceadan_model_default(void); // from a file
void sceadan_model_dump(const struct model *); // to stdout
int sceadan_classify_file(sceadan *,const char *fname);    // classify a file
int sceadan_classify_buf(sceadan *,const uint8_t *buf,size_t bufsize);
const char *sceadan_name_for_type(int);
void sceadan_close(sceadan *);

#ifdef __cplusplus
}
#endif



#endif
