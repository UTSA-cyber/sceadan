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
    FILE *dump_json;
    FILE *dump_nodes;
    int file_type;                    // when dumping
    struct sceadan_vectors *v;
};
typedef struct sceadan_t sceadan;


void sceadan_model_dump(const struct model *); // to stdout

sceadan *sceadan_open(const char *moden_name); // use 0 for default model precompiled
const struct model *sceadan_model_precompiled(void);
const struct model *sceadan_model_default(void); // from a file
void sceadan_update(sceadan *,const uint8_t *buf,size_t bufsize);
void sceadan_vectors_clear(sceadan *s);
int sceadan_classify(sceadan *);
int sceadan_classify_file(const sceadan *,const char *fname);    // classify a file
int sceadan_classify_buf(const sceadan *,const uint8_t *buf,size_t bufsize);
const char *sceadan_name_for_type(int);
void sceadan_close(sceadan *);
void sceadan_dump_json_on_classify(sceadan *,int file_type,FILE *out); // dump JSON vectors instead of classifying
void sceadan_dump_nodes_on_classify(sceadan *,int file_type,FILE *out); // dump  vectors instead of classifying

__END_DECLS


#endif