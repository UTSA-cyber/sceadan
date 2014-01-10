#ifndef NGRAM_H
#define NGRAM_H





/* STANDARD INCLUDES */                                  /* STANDARD INCLUDES */
#include <stdint.h>
/* END STANDARD INCLUDES */                          /* END STANDARD INCLUDES */





/* number of bits in a unigram
   TODO handle architectures with non-octet chars */
#define nbit_unigram (8)

/* number of bits in a bigram */
#define nbit_bigram  (16)



/* number of possible unigrams
   = 2 ** 8
   (needs at least 9 bits) */
#define n_unigram ((uint_fast16_t) 1 << nbit_unigram)

/* number of possible bigrams
   = 2 ** 16
   (needs at least 17 bits) */
#define n_bigram  ((uint_fast32_t) 1 << nbit_bigram)



/* unigram - an octet of bits
   TODO check whether endian-ness can be an issue */
typedef uint8_t  unigram_t;

/* bigram  - two consecutive unigrams
   TODO check whether endian-ness can be an issue */
typedef uint16_t bigram_t;





#endif
