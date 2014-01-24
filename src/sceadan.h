#ifndef SCEADAN_H
#define SCEADAN_H

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef __BEGIN_DECLS
#include <winsock.h>
#if defined(__cplusplus)
#define __BEGIN_DECLS   extern "C" {
#define __END_DECLS     }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif
#endif


__BEGIN_DECLS

struct sceadan_t {
    const struct model *model;
};
typedef struct sceadan_t sceadan;


void sceadan_model_dump(const struct model *); // to stdout

sceadan *sceadan_open(const char *moden_name); // use 0 for default model precompiled
const struct model *sceadan_model_precompiled(void);
const struct model *sceadan_model_default(void); // from a file
int sceadan_classify_file(sceadan *,const char *fname);    // classify a file
int sceadan_classify_buf(sceadan *,const uint8_t *buf,size_t bufsize);
const char *sceadan_name_for_type(int);
void sceadan_close(sceadan *);
void sceadan_vectors_dump(sceadan *,FILE *out);

__END_DECLS


#endif
