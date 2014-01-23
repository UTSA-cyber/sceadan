#ifndef SCEADAN_H
#define SCEADAN_H

#include <stdint.h>
#include <sys/types.h>

#ifdef HAVE_LINEAR_H
#include <linear.h>
#endif
#ifdef HAVE_LIBLINEAR_LINEAR_H
#include <liblinear/linear.h>
#endif

#include "file_type.h"


/* definitions. Some will be moved out of this file */

#define nbit_unigram (8)                // bits in a unigram
#define nbit_bigram  (16)               /* number of bits in a bigram */
#define n_unigram ((uint32_t) 1 << nbit_unigram) /* number of possible unigrams   = 2 ** 8 (needs at least 9 bits) */
#define n_bigram  ((uint32_t) 1 << nbit_bigram)  /* number of possible bigrams = 2 ** 16 (needs at least 17 bits) */

typedef uint8_t  unigram_t;             // unigram 
typedef uint16_t bigram_t; /* bigram  - two consecutive unigrams;  TODO check whether endian-ness can be an issue */

struct model *get_sceadan_model_precompiled();

/* low  ascii range is
   0x00 <= char < 0x20 */
#define ASCII_LO_VAL (0x20)

/* mid  ascii range is
   0x20 <= char < 0x80 */

/* high ascii range is
   0x80 <= char */
#define ASCII_HI_VAL (0x80)

/* type for accumulators */
typedef unsigned long long sum_t;

/* type for summation/counting
   followed by floating point ops */
typedef union {
    sum_t  tot;
    double avg;
} cv_e;

/* unigram count vector
   map unigram to count,
   then to frequency
   implements the probability distribution function for unigrams
   TODO beware of overflow
   more implementations */
typedef cv_e ucv_t[n_unigram];

/* bigram count vector
   map bigram to count,
   then to frequency
   implements the probability distribution function for bigrams
   TODO beware of overflow
   more implementations */
typedef cv_e bcv_t[n_unigram][n_unigram];

typedef enum {
    ID_UNDEF = 0,
    ID_CONTAINER,
    ID_BLOCK
} id_e;

/* main feature vector */
typedef struct {

    id_e id_type;

    const char *id_container;
    size_t      id_block;

    unigram_t const_chr[2];

    /* size of item */
    sum_t  uni_sz;
    //sum_t  bi_sz;

    /*  Feature Name: Bi-gram Entropy
        Description : Shannon's entropy
        Formula     : Sum of -P (v) lb (P (v)),
        over all bigram values (v)
        where P (v) = frequency of bigram value (v) */
    double bigram_entropy;

    /* Feature Name: Item Identifier
       Description : Shannon's entropy
       Formula     : Sum of -P (v) lb (P (v)),
       over all byte values (v)
       where P (v) = frequency of byte value (v) */
    double item_entropy;

    /* Feature Name: Hamming Weight
       Description : Total number of 1s divided by total bits in item */
    cv_e hamming_weight;

    /* Feature Name: Mean Byte Value
       Description : Sum of all byte values in item divided by total size in bytes
       Note        : Only go to EOF if item is an allocated file */
    cv_e byte_value;

    /* Feature Name: Standard Deviation of Byte Values
       Description : Typical standard deviation formula */
    cv_e stddev_byte_val;

    /* (from wikipedia:)
       the (average) absolute deviation of an element of a data set is the
       absolute difference between that element and ... a measure of central
       tendency
       TODO use median instead of mean */
    double abs_dev;

    /* (from wikipedia:)
       skewness is a measure of the asymmetry of the probability distribution of
       a real-valued random variable */
    double skewness;

    /* Feature Name: Kurtosis
       Description : Shows peakedness in the byte value distribution graph */
    double kurtosis;

    /* Feature Name: Compressed item length - bzip2
       Description : Uses Burrows-Wheeler algorithm; effective, slow */
    cv_e bzip2_len;

    /* Feature Name: Compressed item length - LZW
       Description : Lempel-Ziv-Welch algorithm; fast */
    cv_e lzw_len;

    /* Feature Name: Average contiguity between bytes
       Description : Average distance between consecutive byte values */
    cv_e contiguity;

    /* Feature Name: Max Byte Streak
       Description : Length of longest streak of repeating bytes in item
       TODO normalize ? */
    cv_e  max_byte_streak;

    // TODO longest common subsequence

    /* Feature Name: Low ASCII frequency
       Description : Frequency of bytes 0x00 .. 0x1F,
       normalized by item size */
    cv_e lo_ascii_freq;

    /* Feature Name: ASCII frequency
       Description : Frequency of bytes 0x20 .. 0x7F,
       normalized by item size */
    cv_e med_ascii_freq;

    /* Feature Name: High ASCII frequency
       Description : Frequency of bytes 0x80 .. 0xFF,
       normalized by item size */
    cv_e hi_ascii_freq;

    /* Feature Name: Byte Value Correlation
       Description : Correlation of the values of the n and n+1 bytes */
    double byte_val_correlation;

    /* Feature Name: Byte Value Frequency Correlation
       Description : Correlation of the frequencies of values of the m and m+1 bytes */
    double byte_val_freq_correlation;

    /* Feature Name: Unigram chi square
       Description : Test goodness-of-fit of unigram count vector relative to
       random distribution of byte values to see if distribution
       varies statistically significantly from a random
       distribution */
    double uni_chi_sq;
} mfv_t;

/* TYPEDEFS FOR I/O */
typedef int (*output_f) (
                         const ucv_t        ucv,
                         const bcv_t        bcv,
                         const mfv_t *const mfv,
                         file_type_e  file_type
                         );
/* END TYPEDEFS FOR I/O */
struct sceadan_type_t {
    int code;
    const char *name;
};

extern struct sceadan_type_t sceadan_types[];
const char *sceadan_name_for_type(int i);

/* FUNCTIONS */
// TODO full path vs relevant path may matter
int
process_dir (
             const          char path[],
             const unsigned int  block_factor,
             const output_f      do_output,
             file_type_e file_type
             ) ;

int
process_file (
              const          char path[],
              const unsigned int  block_factor,
              const output_f      do_output,
	      file_type_e file_type
              ) ;

int
output_competition  (
                     const ucv_t        ucv,
                     const bcv_t        bcv,
                     const mfv_t *const mfv,
                     file_type_e file_type
                     ) ;


#endif
