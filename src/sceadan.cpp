/*****************************************************************
 *
 *
 *Copyright (c) 2012-2013 The University of Texas at San Antonio
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright Holder:
 * University of Texas at San Antonio
 * One UTSA Circle 
 * San Antonio, Texas 78209
 */

/*
 *
 * Development History:
 *
 *
 * 2014 - Significant refactoring and updating by:
 * Simson L. Garfinkel, Naval Postgraduate School
 *
 * 2013 - Created by:
 * Dr. Nicole Beebe and Lishu Liu, Department of Information Systems and Cyber Security (nicole.beebe@utsa.edu)
 * Laurence Maddox, Department of Computer Science
 * 
 */

#include <config.h>
#include "sceadan.h"

#include <assert.h>
#include <ftw.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#ifndef O_BINARY
#define O_BINARY 0
#endif

#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>

/* We require liblinear.
 * If it is not available, Sceadan will not compile.
 * (previously it would compile but produce run-time errors)
 */

#ifndef HAVE_LIBLINEAR
#error Sceadan requires liblinear
#endif

#ifdef HAVE_LINEAR_H
#include <linear.h>
#endif
#ifdef HAVE_LIBLINEAR_LINEAR_H
#include <liblinear/linear.h>
#endif

/* The definitions of the sceadan structure.  A pointer to the
 * structure is available for the calling C/C++ program, but the
 * contents are private. */

struct sceadan_t {
private:
    // default copy construction and assignment are meaningless
    // and not implemented
    sceadan_t(const sceadan_t &i);
    sceadan_t &operator=(const sceadan_t &i);
public:
    sceadan_t():model(),model_name(),v(),types(),ngram_mode(),mask_file(),mask(),dump_json(),dump_nodes(),file_type(){}
    const struct model *model;          // liblinear model
    std::string model_name;
    struct sceadan_vectors *v;          // internal used by sceadan
    typedef std::map<std::string, int>  typemap_t;
    typedef std::pair<std::string,int>  types_pair;
    typemap_t types;                    // maps type names to liblinear class

    // For disabling individual features:
    int ngram_mode;                     // 
    const char *mask_file;              // feature mask file (not clear why it needs to be here)
    char *mask;                         // feature mask


    // These are set if the feature vectors are dumped:
    FILE *dump_json; // file where the feature vectors should be dumped as a JSON object
    FILE *dump_nodes; // file where the feature vector nodes should be dumped
    int file_type;                    // when dumping
};


/* definitions. Some will be moved out of this file */

typedef uint8_t  unigram_t;  /* unigram  */
typedef uint16_t bigram_t;   /* bigram  two consecutive unigrams; (first<<8)|second */
uint32_t const nbit_unigram=8;                // bits in a unigram
uint32_t const nbit_bigram=16;               /* number of bits in a bigram */

#define NUNIGRAMS 256   /* number of possible unigrams   = 2 ** 8 (needs at least 9 bits) */
#define NBIGRAMS 65536  /* number of possible bigrams = 2 ** 16 (needs at least 17 bits) */
#define bigramcode(f,s) ((f<<8)+s)


/* Liblinear index mapping: */
static const int START_UNIGRAMS=1;         /* 1..256 - unigram counts */
static const int START_BIGRAMS_EVEN = START_UNIGRAMS+NUNIGRAMS;      /* even bigram counts for bigram FS */
static const int START_BIGRAMS_ALL  = START_BIGRAMS_EVEN + NBIGRAMS; /* all bigram counts for bigram FS (characters F and S, where FS=F<<8|S) */
static const int START_BIGRAMS_ODD  = START_BIGRAMS_ALL  + NBIGRAMS; /* odd bigram counts for bigram FS */
static const int START_STATS        = START_BIGRAMS_ODD  + NBIGRAMS;
static const int STATS_IDX_BIGRAM_ENTROPY              = START_STATS + 0;
static const int STATS_IDX_ITEM_ENTROPY                = START_STATS + 1;
static const int STATS_IDX_HAMMING_WEIGHT              = START_STATS + 2;
static const int STATS_IDX_MEAN_BYTE_VALUE             = START_STATS + 3;
static const int STATS_IDX_STDDEV_BYTE_VAL             = START_STATS + 4;
static const int STATS_IDX_ABS_DEV                     = START_STATS + 5;
static const int STATS_IDX_SKEWNESS                    = START_STATS + 6;
static const int STATS_IDX_KURTOSIS                    = START_STATS + 7;
static const int STATS_IDX_CONTIGUITY                  = START_STATS + 8;
static const int STATS_IDX_MAX_BYTE_STREAK             = START_STATS + 9;
static const int STATS_IDX_LO_ASCII_FREQ               = START_STATS + 10;
static const int STATS_IDX_MED_ASCII_FREQ              = START_STATS + 11;
static const int STATS_IDX_HI_ASCII_FREQ               = START_STATS + 12;
static const int STATS_IDX_BYTE_VAL_CORRELATION        = START_STATS + 13;
static const int STATS_IDX_BYTE_VAL_FREQ_CORRELATION   = START_STATS + 14;
static const int STATS_IDX_UNI_CHI_SQ                  = START_STATS + 15;
static const int MAX_NR_ATTR        = START_STATS+16;

/* Tunable parameters */

const uint32_t ASCII_LO_VAL=0x20;   /* low  ascii range is  0x00 <= char < ASCII_LOW_VALUE0 */
const uint32_t ASCII_HI_VAL=0x80;   /* high ascii range is   ASCII_HI_VAL <= char */

/* type for summation/counting followed by floating point ops.
 * The original programmer had this as a union, which is just asking for trouble.
 */
typedef struct {
    uint64_t  tot;
    double    avg;
} cv_e;

/* unigram count vector map unigram to count, then to frequency
   implements the probability distribution function for unigrams TODO
   beware of overflow more implementations */
typedef cv_e ucv_t[NUNIGRAMS];

/* bigram count vector map bigram to count, then to frequency
   implements the probability distribution function for bigrams TODO
   beware of overflow more implementations */
typedef cv_e bcv_t[NUNIGRAMS][NUNIGRAMS];

/* main feature vector */
typedef struct {
    /* size of item */
    uint64_t  unigram_count;                      /* number of unigrams */

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
    cv_e mean_byte_value;

    /* Feature Name: Standard Deviation of Byte Values
       Description : Typical standard deviation formula */
    cv_e stddev_byte_val;

    /* (from wikipedia:)
       the (average) absolute deviation of an element of a data set is the
       absolute difference between that element and ... a measure of central
       tendency
       TODO use median instead of mean */
    double abs_dev;

    /* SKEWNESS is a measure of the asymmetry of the probability distribution of
       a real-valued random variable */
    double skewness;

    /* KURTOSIS  Shows peakedness in the byte value distribution graph */
    double kurtosis;

    /* Feature Name: Average contiguity between bytes
       Description : Average distance between consecutive byte values */
    cv_e  contiguity;

    /* Feature Name: Max Byte Streak
       Description : Length of longest streak of repeating bytes in item
       TODO normalize ? */
    cv_e  max_byte_streak;

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

/* END TYPEDEFS FOR I/O */

/**
 * implementaiton hidden from sceadan users.
 */
struct sceadan_vectors {
    ucv_t ucv;                          /* unigram statistics */
    bcv_t bcv_all;                      /* all bigram statistics */
    bcv_t bcv_even;                     /* even bigram statistics */
    bcv_t bcv_odd;                      /* odd bigram statistics */
    mfv_t mfv;                          /* other statistics; # of unigrams processes is mfv.unigram_count */
    uint8_t prev_value;                 /* last value from previous loop iteration */
    uint64_t prev_count;                /* number of prev_vals in a row*/
    const char *file_name;              /* if the vectors came from a file, indicate it here */
};
typedef struct sceadan_vectors sceadan_vectors_t;

#define MODEL_DEFAULT_FILENAME ("model") /* default model file */

#define RANDOMNESS_THRESHOLD (.995)     /* ignore things more random than this */
#define UCV_CONST_THRESHOLD  (.5)       /* ignore UCV more than this */
#define BCV_CONST_THRESHOLD  (.5)       /* ignore BCV more than this */

static inline double square(double v) { return v*v;}
static inline double cube(double v)   { return v*v*v;}
static inline uint64_t max( const uint64_t a, const uint64_t b ) { return a > b ? a : b; }

/*********************************
 *** Map file types to numbers ***
 *********************************/

static const char *sceadan_map_precompiled[] =
{"UNCLASSIFIED", "TEXT", "CSV", "LOG", "HTML", "XML", "ASPX", "JSON", "JS", "JAVA", 
 "CSS", "B64", "B85", "B16", "URL", "PS", "RTF", "TBIRD", "PST", "PNG",
 "GIF", "TIF", "JB2", "GZ", "ZIP", "JAR", "RPM", "BZ2", "PDF", "DOCX", 
 "XLSX", "PPTX", "JPG", "MP3", "M4A", "MP4", "AVI", "WMV", "FLV", "SWF", 
 "WAV", "WMA", "MOV", "DOC",  "XLS", "PPT", "FS-FAT", "FS-NTFS", "FS-EXT", "EXE",
 "DLL", "ELF", "BMP", "AES", "RAND",  "PPS", "RAR", "3GP", "7Z", 
 0};

const char *sceadan_name_for_type(const sceadan *s,int code)
{
    for (sceadan_t::typemap_t::const_iterator it = s->types.begin(); it!=s->types.end(); it++){
        if( (*it).second == code) return (*it).first.c_str();
    }
    return(0);
}

int sceadan_type_for_name(const sceadan *s,const char *name)
{
    sceadan_t::typemap_t::const_iterator it = s->types.find(name);

    if (it == s->types.end()) return -1;
    return (*it).second;
}


/****************************************************************
 *** liblinear node/vector interface
 ****************************************************************/

/** Create the sparse liblinear fature_node structure from the vectors used by sceadan. **/

#define assert_and_set(i)    {assert(set[i]==0);set[i]=1;} // make sure it hasn't been set before
#define set_index_value(k,v) {assert(idx<MAX_NR_ATTR);x[idx].index = k; x[idx].value = v; idx++;}
#define feature_enabled(k)   s->mask[k]=='1'

static void build_nodes_from_vectors(const sceadan *s, const sceadan_vectors_t *v, struct feature_node *x )
{
    int idx = 0;                        /* cannot exceed MAX_NR_ATTR */
    int key = 0;
    int set[MAX_NR_ATTR];
    memset(set,0,sizeof(set));
    
    /* Add the unigrams to the vector */
    for (int i = 0 ; i < NUNIGRAMS; i++) {
        key = START_UNIGRAMS + i;
        if(v->ucv[i].avg > 0.0 && feature_enabled(key)){
            set_index_value(key, v->ucv[i].avg);
        }
    }
    
    /* Add the bigrams to the vector */
    if (s->ngram_mode & 1) {
        for (int i = 0; i < NUNIGRAMS; i++) {
            for (int j = 0; j < NUNIGRAMS; j++) {
                key = START_BIGRAMS_ALL+bigramcode(i,j);
                if (v->bcv_all[i][j].avg > 0.0 && feature_enabled(key)) {
                    set_index_value(key, v->bcv_all[i][j].avg);
                }
            }
        }
    }
    if (s->ngram_mode & 2) {
        for (int i = 0; i < NUNIGRAMS; i++) {
            for (int j = 0; j < NUNIGRAMS; j++) {
                key = START_BIGRAMS_EVEN+bigramcode(i,j);
                if (v->bcv_even[i][j].avg > 0.0 && feature_enabled(key)) {
                    set_index_value(key, v->bcv_even[i][j].avg);
                }
            }
        }
    }
    if (s->ngram_mode & 4) {
        for (int i = 0; i < NUNIGRAMS; i++) {
            for (int j = 0; j < NUNIGRAMS; j++) {
                key = START_BIGRAMS_ODD+bigramcode(i,j);
                if (v->bcv_odd[i][j].avg > 0.0 && feature_enabled(key)) {
                    set_index_value(key, v->bcv_odd[i][j].avg);
                }
            }
        }
    }
    
    key = STATS_IDX_BIGRAM_ENTROPY;
    if (s->ngram_mode & 0x00008 && feature_enabled(key)) { set_index_value(key, v->mfv.bigram_entropy); }
    key = STATS_IDX_ITEM_ENTROPY;
    if (s->ngram_mode & 0x00010 && feature_enabled(key)) { set_index_value(key, v->mfv.item_entropy); }
    key = STATS_IDX_HAMMING_WEIGHT; 
    if (s->ngram_mode & 0x00020 && feature_enabled(key)) { set_index_value(key, v->mfv.hamming_weight.avg); }
    key = STATS_IDX_MEAN_BYTE_VALUE;
    if (s->ngram_mode & 0x00040 && feature_enabled(key)) { set_index_value(key, v->mfv.mean_byte_value.avg); }
    key = STATS_IDX_STDDEV_BYTE_VAL;
    if (s->ngram_mode & 0x00080 && feature_enabled(key)) { set_index_value(key, v->mfv.stddev_byte_val.avg); }
    key = STATS_IDX_ABS_DEV;
    if (s->ngram_mode & 0x00100 && feature_enabled(key)) { set_index_value(key, v->mfv.abs_dev); }
    key = STATS_IDX_SKEWNESS;
    if (s->ngram_mode & 0x00200 && feature_enabled(key)) { set_index_value(key, v->mfv.skewness); }
    key = STATS_IDX_KURTOSIS;
    if (s->ngram_mode & 0x00400 && feature_enabled(key)) { set_index_value(key, v->mfv.kurtosis); }
    key = STATS_IDX_CONTIGUITY;
    if (s->ngram_mode & 0x00800 && feature_enabled(key)) { set_index_value(key, v->mfv.max_byte_streak.avg); }
    key = STATS_IDX_MAX_BYTE_STREAK;
    if (s->ngram_mode & 0x01000 && feature_enabled(key)) { set_index_value(key, v->mfv.max_byte_streak.tot); }
    key = STATS_IDX_LO_ASCII_FREQ;
    if (s->ngram_mode & 0x02000 && feature_enabled(key)) { set_index_value(key, v->mfv.lo_ascii_freq.avg); }
    key = STATS_IDX_MED_ASCII_FREQ;
    if (s->ngram_mode & 0x04000 && feature_enabled(key)) { set_index_value(key, v->mfv.med_ascii_freq.avg); }
    key = STATS_IDX_HI_ASCII_FREQ;
    if (s->ngram_mode & 0x08000 && feature_enabled(key)) { set_index_value(key, v->mfv.hi_ascii_freq.avg); }
    key = STATS_IDX_BYTE_VAL_CORRELATION;
    if (s->ngram_mode & 0x10000 && feature_enabled(key)) { set_index_value(key, v->mfv.byte_val_correlation); }
    key = STATS_IDX_BYTE_VAL_FREQ_CORRELATION;
    if (s->ngram_mode & 0x20000 && feature_enabled(key)) { set_index_value(key, v->mfv.byte_val_freq_correlation); }
    key = STATS_IDX_UNI_CHI_SQ;
    if (s->ngram_mode & 0x40000 && feature_enabled(key)) { set_index_value(key, v->mfv.uni_chi_sq); }

    /* Add the Bias if we are using Bias. It goes last, apparently */
    if (s->model && s->model->bias >= 0 ) {
        set_index_value(get_nr_feature( s->model ) + 1, s->model->bias);
    }
    /* And note that we are at the end of the vectors */
    assert (idx < MAX_NR_ATTR) ;
    x[ idx++ ].index = -1; /* end of vectors */
}


static void dump_vectors_as_json(const sceadan *s,const sceadan_vectors_t *v)
{
    printf("{ \"file_type\": %d,\n",s->file_type);
    if(v->file_name) printf("  \"file_name\": \"%s\",\n",v->file_name);
    printf("  \"unigrams\": { \n");
    int first = 1;
    for(int i=0;i<NUNIGRAMS;i++){
        if(v->ucv[i].avg>0){
            if(first) {
                first = 0;
            } else {
                printf(",\n");
            }
            printf("    \"%d\" : %.16lg",i,v->ucv[i].avg);
        }
    }
    printf("  },\n");
    printf("  \"bigrams:\": { \n");
    first = 1;
    for(int i=0;i<NUNIGRAMS;i++){
        for(int j=0;j<NUNIGRAMS;j++){
            if(v->bcv_all[i][j].avg>0){
                if(first){
                    first = 0;
                } else {
                    printf(",\n");
                    first = 0;
                }
                printf("    \"%d\" : %.16lg",i<<8|j,v->bcv_all[i][j].avg);
            }
        }
    }
    printf("  }\n");
#define OUTPUT(XXX) printf("  \"%s\": %.16lg,\n",#XXX,v->mfv.XXX)
    OUTPUT(bigram_entropy);
    OUTPUT(item_entropy);
    OUTPUT(hamming_weight.avg);
    OUTPUT(mean_byte_value.avg);
    OUTPUT(stddev_byte_val.avg);
    OUTPUT(abs_dev);
    OUTPUT(skewness);
    OUTPUT(kurtosis);
    OUTPUT(contiguity.avg);
    OUTPUT( max_byte_streak.avg);
    OUTPUT(lo_ascii_freq.avg);
    OUTPUT(med_ascii_freq.avg);
    OUTPUT(hi_ascii_freq.avg);
    OUTPUT(byte_val_correlation);
    OUTPUT(byte_val_freq_correlation);
    OUTPUT(uni_chi_sq);
    printf("  \"version\":1.0\n");
    printf("}\n");
}

static void dump_nodes(FILE *out,const sceadan *s,const struct feature_node *x)
{
    fprintf(out,"%d ",s->file_type);
    for(int i=0;i<MAX_NR_ATTR;i++){
        if (x[i].index && x[i].value>0) fprintf(out,"%d:%g ",x[i].index,x[i].value);
        if (x[i].index == -1) break;
    }
    fputc('\n',out);
}

/****************************************************************
 *** VECTOR GENERATION FUNCTIONS
 ****************************************************************/

static void vectors_update (const sceadan *s,const uint8_t buf[], const size_t sz, sceadan_vectors_t *v)
{
    for (size_t ndx = 0; ndx < sz; ndx++) { /* ndx is index within the buffer */

        /* First update single-byte statistics */

        const unigram_t unigram = buf[ndx];
        v->ucv[unigram].tot++;          /* unigram counter */

        /* Histogram for ASCII value ranges */
        if (unigram < ASCII_LO_VAL) v->mfv.lo_ascii_freq.tot++;
        else if (unigram < ASCII_HI_VAL) v->mfv.med_ascii_freq.tot++;
        else v->mfv.hi_ascii_freq.tot++;

        // total count of set bits (for hamming weight)
        v->mfv.hamming_weight.tot += __builtin_popcount (unigram);
        v->mfv.mean_byte_value.tot += unigram;              /* sum of byte values */
        v->mfv.stddev_byte_val.tot += unigram*unigram; /* sum of squares */

        /* Compute the bigram values if this is not the first character seen */
        if (v->mfv.unigram_count>0){                 /* only process bigrams on characters >=1 */
            int parity = v->mfv.unigram_count % 2;

            if (s->ngram_mode & 1) v->bcv_all[v->prev_value][unigram].tot++;
            if (parity == 0) {
                if (s->ngram_mode & 2) v->bcv_even[v->prev_value][unigram].tot++;
            } else {
                if (s->ngram_mode & 4) v->bcv_odd[v->prev_value][unigram].tot++;
            }

            v->mfv.contiguity.tot += abs (unigram - v->prev_value);
            if (v->prev_value==unigram) {
                v->prev_count++;
                v->mfv.max_byte_streak.tot = max(v->prev_count, v->mfv.max_byte_streak.tot);
            } 
        }
        /* If this is the first, or if this is not the next in a streak, reset the counters */
        if (v->mfv.unigram_count==0 || v->prev_value!=unigram){
            v->prev_value = unigram;
            v->prev_count = 1;
        }

        v->mfv.unigram_count ++;
    }
}

static void vectors_finalize ( sceadan_vectors_t *v)
{
    // hamming weight
    v->mfv.hamming_weight.avg = (double) v->mfv.hamming_weight.tot / (v->mfv.unigram_count * nbit_unigram);

    // mean byte value
    v->mfv.mean_byte_value.avg = (double) v->mfv.mean_byte_value.tot / v->mfv.unigram_count;

    // average contiguity between bytes
    v->mfv.contiguity.avg = (double) v->mfv.contiguity.tot / v->mfv.unigram_count;

    // max byte streak
    //v->mfv.max_byte_streak = max_cnt;
    v->mfv.max_byte_streak.avg = (double) v->mfv.max_byte_streak.tot / v->mfv.unigram_count;

    double expectancy_x3 = 0;  // for skewness
    double expectancy_x4 = 0;  // for kurtosis

    const double central_tendency = v->mfv.mean_byte_value.avg;
    for (int i = 0; i < NUNIGRAMS; i++) {

        // unigram frequency
        v->ucv[i].avg = (double) v->ucv[i].tot / v->mfv.unigram_count;
        v->mfv.abs_dev += v->ucv[i].tot * fabs (i - central_tendency);

        // item entropy
        // Currently calculated but not used 
        double pv = v->ucv[i].avg;
        if (fabs(pv)>0) {
            v->mfv.item_entropy += pv * log2 (1 / pv) / nbit_unigram; // more divisions for accuracy
        } 
            
        // Normalize the bigram counts
        for (int j = 0; j < NUNIGRAMS; j++) {
            v->bcv_all[i][j].avg  = (double) v->bcv_all[i][j].tot / (v->mfv.unigram_count / 2); // rounds down
            v->bcv_even[i][j].avg = (double) v->bcv_even[i][j].tot / (v->mfv.unigram_count / 4); // rounds down
            v->bcv_odd[i][j].avg = (double) v->bcv_odd[i][j].tot / (v->mfv.unigram_count / 4); // rounds down

            // bigram entropy
            // Currently calculated but not used 
            pv = v->bcv_all[i][j].avg;
            if (fabs(pv)>0) {
                v->mfv.bigram_entropy  += pv * log2 (1 / pv) / nbit_bigram;
            }
        }

        const double extmp = cube(i) * v->ucv[i].avg; 

        expectancy_x3 += extmp;        // for skewness
        expectancy_x4 += extmp * i;    // for kurtosis
    }

    const double variance  = (double) v->mfv.stddev_byte_val.tot / v->mfv.unigram_count - square(v->mfv.mean_byte_value.avg);

    v->mfv.stddev_byte_val.avg = sqrt (variance);

    const double sigma3    = variance * v->mfv.stddev_byte_val.avg;
    const double variance2 = square(variance);

    // average absolute deviation
    v->mfv.abs_dev /= v->mfv.unigram_count;
    v->mfv.abs_dev /= NUNIGRAMS;

    // skewness
    v->mfv.skewness = (expectancy_x3 - v->mfv.mean_byte_value.avg * (3 * variance + square (v->mfv.mean_byte_value.avg))) / sigma3;

    // kurtosis
    assert(isinf(expectancy_x4)==0);
    assert(isinf(variance2)==0);

    v->mfv.kurtosis = (expectancy_x4 / variance2);
    v->mfv.mean_byte_value.avg      /= NUNIGRAMS;
    v->mfv.stddev_byte_val.avg /= NUNIGRAMS;
    v->mfv.kurtosis            /= NUNIGRAMS;
    v->mfv.contiguity.avg      /= NUNIGRAMS;
    v->mfv.lo_ascii_freq.avg  = (double) v->mfv.lo_ascii_freq.tot  / v->mfv.unigram_count;
    v->mfv.med_ascii_freq.avg = (double) v->mfv.med_ascii_freq.tot / v->mfv.unigram_count;
    v->mfv.hi_ascii_freq.avg  = (double) v->mfv.hi_ascii_freq.tot  / v->mfv.unigram_count;
}

/* predict the vectors with a model and return the predicted type.
 * 
 * That is to handle vectors of too little or too much
 * variance/entropy, e.g. a file of all 0x00s will be considered
 * CONSTANT, or a file of all random unigrams will be considered
 * RANDOM. We consider those vectors abnormal and taken special care
 * of, instead of predicting. 
 */
static int sceadan_predict(const sceadan *s,sceadan_vectors_t *v)
{
    int ret = 0;

    vectors_finalize(v);
    if(s->dump_json){                        /* dumping, not predicting */
        dump_vectors_as_json(s,v);
        return 0;
    }

    struct feature_node *x = (struct feature_node *) calloc(MAX_NR_ATTR,sizeof(struct feature_node));
    build_nodes_from_vectors(s,v, x);
    
    if(s->dump_nodes){
        dump_nodes(s->dump_nodes,s,x);
    } else {
        ret = predict(s->model,x);           /* run the liblinear predictor */
    }
    free(x);
    return ret;
}


const struct model *sceadan_model_default()
{
    static struct model *default_model = 0;               /* this assures that the model will only be loaded once */
    if(default_model==0){
        default_model=load_model(MODEL_DEFAULT_FILENAME);
        if(default_model==0){
            fprintf(stderr,"can't open model file %s\n","");
            return 0;
        }
    }
    return default_model;
}

void sceadan_model_dump(const struct model *model,FILE *f)
{
    if(model->param.nr_weight){
        fprintf(f,"static int weight_label[]={");
        for(int i=0;i<model->param.nr_weight;i++){
            if(i>0) fprintf(f,",");
            fprintf(f,"%d",model->param.weight_label[i]);
            if(i%10==9) fprintf(f,"\n\t");
        }
        fprintf(f,"};\n\n");
    }
    if(model->param.nr_weight){
        fprintf(f,"static double weight[] = {");
        for(int i=0;i<model->param.nr_weight;i++){
            if(i>0) fprintf(f,",");
            fprintf(f,"%g",model->param.weight[i]);
            if(i%10==9) fprintf(f,"\n\t");
        }
        fprintf(f,"};\n\n");
    }

    fprintf(f,"static int label[] = {");
    for(int i=0;i<model->nr_class;i++){
        fprintf(f,"%d",model->label[i]);
        if(i<model->nr_class-1) fprintf(f,",");
        if(i%20==19) fprintf(f,"\n\t");
    }
    fprintf(f,"};\n");

    fprintf(f,"static double w[] = {");
    int n;
    if(model->bias>=0){
        n = model->nr_feature+1;
    } else {
        n = model->nr_feature;
    }
    int w_size = n;
    int nr_w;
    if(model->nr_class==2 && model->param.solver_type != 4){
        nr_w = 1;
    } else {
        nr_w = model->nr_class;
    }

    for(int i=0;i<w_size;i++){
        for(int j=0;j<nr_w;j++){
            fprintf(f,"%.16lg",model->w[i*nr_w+j]);
            if(i!=w_size-1 || j!=nr_w-1) fprintf(f,",");
            if(j%10==9) fprintf(f,"\n\t");
        }
        fprintf(f,"\n\t");
    }
    fprintf(f,"};\n");
        
    fprintf(f,"static struct model m = {\n");
    fprintf(f,"\t .param = {\n");
    fprintf(f,"\t\t .solver_type=%d,\n",model->param.solver_type);
    fprintf(f,"\t\t .eps = %g,\n",model->param.eps);
    fprintf(f,"\t\t .C = %g,\n",model->param.C);
    fprintf(f,"\t\t .nr_weight = %d,\n",model->param.nr_weight);
    fprintf(f,"\t\t .weight_label = %s,\n",model->param.nr_weight ? "weight_label" : "0");
    fprintf(f,"\t\t .weight = %s\n",model->param.nr_weight ? "weight" : "0");

    /* Liblinear 1.9 added model->param.p. The code below fakes it to be 0 if we are running
     * on Liblinear 1.8 or before, and allows the resulting code to be compiled on Liblinear 1.8 or 1.9
     */
    fprintf(f,"#ifdef LIBLINEAR_19\n");
#ifdef LIBLINEAR_19
    fprintf(f,"\t\t ,t.p = %g",model->param.p);
#else
    fprintf(f,"\t\t ,t.p = %g",0.0);
#endif
    fprintf(f,"#endif\n");
    fprintf(f,"},\n");

    fprintf(f,"\t .nr_class=%d,\n",model->nr_class);
    fprintf(f,"\t .nr_feature=%d,\n",model->nr_feature);
    fprintf(f,"\t .w=w,\n");
    fprintf(f,"\t .label=label,\n");
    fprintf(f,"\t .bias=%g};\n",model->bias);

    fprintf(f,"const struct model *sceadan_model_precompiled(){return &m;}\n");
}


const char *sceadan_model_name(sceadan *s)
{
    return s->model_name.c_str();
}

/****************************************************************
 *** Feature mask
 ****************************************************************/


void sceadan_initialize_feature_mask(sceadan *s)     // initialize feature_mask based on ngram_mode
{
    int count = 0;
    memset(s->mask, '0', MAX_NR_ATTR);
    /* unigrams */
    memset(s->mask+START_UNIGRAMS, '1', NUNIGRAMS);
    count += NUNIGRAMS;
    /* bigrams */
    if(s->ngram_mode & 1){
        memset(s->mask+START_BIGRAMS_ALL, '1', NBIGRAMS);
        count += NBIGRAMS;
    }
    if(s->ngram_mode & 2){
        memset(s->mask+START_BIGRAMS_EVEN, '1', NBIGRAMS);
        count += NBIGRAMS;
    }
    if(s->ngram_mode & 4){
        memset(s->mask+START_BIGRAMS_ODD, '1', NBIGRAMS);
        count += NBIGRAMS;
    }
    /* stats */
    if (s->ngram_mode & 0x00008) { s->mask[STATS_IDX_BIGRAM_ENTROPY]            = '1'; count ++; }
    if (s->ngram_mode & 0x00010) { s->mask[STATS_IDX_ITEM_ENTROPY]              = '1'; count ++; }
    if (s->ngram_mode & 0x00020) { s->mask[STATS_IDX_HAMMING_WEIGHT]            = '1'; count ++; }
    if (s->ngram_mode & 0x00040) { s->mask[STATS_IDX_MEAN_BYTE_VALUE]           = '1'; count ++; }
    if (s->ngram_mode & 0x00080) { s->mask[STATS_IDX_STDDEV_BYTE_VAL]           = '1'; count ++; }
    if (s->ngram_mode & 0x00100) { s->mask[STATS_IDX_ABS_DEV]                   = '1'; count ++; }
    if (s->ngram_mode & 0x00200) { s->mask[STATS_IDX_SKEWNESS]                  = '1'; count ++; }
    if (s->ngram_mode & 0x00400) { s->mask[STATS_IDX_KURTOSIS]                  = '1'; count ++; }
    if (s->ngram_mode & 0x00800) { s->mask[STATS_IDX_CONTIGUITY]                = '1'; count ++; }
    if (s->ngram_mode & 0x01000) { s->mask[STATS_IDX_MAX_BYTE_STREAK]           = '1'; count ++; }
    if (s->ngram_mode & 0x02000) { s->mask[STATS_IDX_LO_ASCII_FREQ]             = '1'; count ++; }
    if (s->ngram_mode & 0x04000) { s->mask[STATS_IDX_MED_ASCII_FREQ]            = '1'; count ++; }
    if (s->ngram_mode & 0x08000) { s->mask[STATS_IDX_HI_ASCII_FREQ]             = '1'; count ++; }
    if (s->ngram_mode & 0x10000) { s->mask[STATS_IDX_BYTE_VAL_CORRELATION]      = '1'; count ++; }
    if (s->ngram_mode & 0x20000) { s->mask[STATS_IDX_BYTE_VAL_FREQ_CORRELATION] = '1'; count ++; }
    if (s->ngram_mode & 0x40000) { s->mask[STATS_IDX_UNI_CHI_SQ]                = '1'; count ++; }

}

int sceadan_load_feature_mask(sceadan *s,const char *file_name)
{
    int count=0;
    FILE *fp = fopen(file_name, "r");
    if(fp == NULL){
        printf("sceadan: error opening mask file %s\n", file_name);
        return -1;
    }
    memset(s->mask, '0', MAX_NR_ATTR);
    for(int i = 0; i < MAX_NR_ATTR; i++){
        int ch = fgetc(fp);
        assert(ch=='0' || ch=='1');
        if(ch=='1'){
            s->mask[i] = char(ch);
            count ++;
        }
    }
    if(fclose(fp) < 0){
        printf("sceadan: error closing mask file %s\n", file_name);
        return -1;
    }
    // make sure feature_mask match the model
    assert(count==get_nr_feature( s->model ));
    return 0;
}

int sceadan_dump_feature_mask(sceadan *s,const char *file_name)
{
    FILE *fp = fopen(file_name, "w");
    if(fp == NULL){
        printf("Sceadan: Error opening mask dump file %s\n", file_name);
        return -1;
    }
    if(fwrite(s->mask, sizeof(char), MAX_NR_ATTR, fp) != (size_t) MAX_NR_ATTR){
        printf("Error writing file %s\n", file_name);
        fclose(fp);
        return -1;
    }
    if(fclose(fp) < 0){
        printf("Error closing file %s\n", file_name);
        return -1;
    }
    return 0;
}

/****************************************************************
 *** Feature mask
 ****************************************************************/


/*
 * Open another classifier, reading both a model, a map and a feature_mask.
 */
sceadan *sceadan_open(const char *model_file,const char *class_file,const char *feature_mask_file) // use 0 for default model
{
    sceadan *s    = new sceadan();
    if (model_file && model_file[0]) {
        s->model_name = model_file;
        s->model = load_model(model_file);
    } else {
        s->model_name = "<precompiled>";
        s->model      = sceadan_model_precompiled();
    }

    if (!s->model){
        delete s;
        return 0;           // cannot load
    }

    s->v          = new sceadan_vectors_t();
    s->ngram_mode = SCEADAN_NGRAM_MODE_DEFAULT;
    /* Load up the default types */
    int type_counter = 0;
    while(sceadan_map_precompiled[type_counter]){
        s->types[ sceadan_map_precompiled[type_counter] ] = type_counter;
        type_counter++;
    }
    if (class_file && class_file[0]){
        std::ifstream i(class_file);
        if (i.is_open()) {
            std::string str;
            while(getline(i,str)) {
                size_t endpos = str.find_last_not_of(" \n\r\t");
                if (std::string::npos != endpos ) str = str.substr( 0, endpos+1 );

                /* Don't add unless it's not present */
                
                if (s->types.find(str) == s->types.end()) {
                    s->types[ str ] = type_counter++;
                }
            }
        }
    }
    s->mask = (char *)calloc(MAX_NR_ATTR, sizeof(char));
    s->mask_file = feature_mask_file;
    if(feature_mask_file && feature_mask_file[0]){
        if(sceadan_load_feature_mask(s, feature_mask_file) < 0){
            goto fail;
        }
    }
    else{
        /* if we are not loading feature_mask from file,
         * defer the initialization of feature_mask in 
         * sceadan_set_ngram_mode(), so that we can do 
         * some assertion to make sure the ngram_mode
         * matches the loaded mode.

         * ngram_mode ---> full feature_mask count ---> nr_feature in loaded model
         *
         * TODO: 
         *      should full feature_mask count == nr_feature?
         *      What if model->bias != -1?
         */
        
        sceadan_initialize_feature_mask(s);
    }
    return s;

fail:
    sceadan_close(s);                           // will do cleanup
    return 0;  
}

void sceadan_close(sceadan *s)
{
    if (s->v) {
        delete s->v;
        s->v = 0;
    }
    if(s->mask) { free(s->mask);}
    delete s;
}

void sceadan_clear(sceadan *s)
{
    memset(s->v,0,sizeof(sceadan_vectors_t));
}

void sceadan_update(sceadan *s,const uint8_t *buf,size_t bufsize)
{
    vectors_update(s,buf, bufsize, s->v);
}

int sceadan_classify(sceadan *s)
{
    int r = sceadan_predict(s,s->v);
    sceadan_clear(s);
    return r;
}


int sceadan_classify_file(const sceadan *s,const char *file_name)
{
    sceadan_vectors_t v;
    memset(&v,0,sizeof(v));
    v.file_name = file_name;
    const int fd = open(file_name, O_RDONLY|O_BINARY);
    if (fd<0) return -1;                /* error condition */
    while (true) {
        uint8_t    buf[BUFSIZ];
        const ssize_t rd = read (fd, buf, sizeof (buf));
        if(rd<=0) break;
        vectors_update(s,buf,rd,&v);
    }
    if(close(fd)<0) return -1;
    return sceadan_predict(s,&v);
}

int  sceadan_classify_buf(const sceadan *s,const u_char *buf,size_t buflen)
{
    sceadan_vectors_t v;
    memset(&v,0,sizeof(v));
    vectors_update(s,buf,buflen,&v);
    return sceadan_predict(s,&v);
}

void sceadan_dump_json_on_classify(sceadan *s,int file_type,FILE *out)
{
    s->dump_json = out;
    s->file_type = file_type;
}

void sceadan_dump_nodes_on_classify(sceadan *s,int file_type,FILE *out)
{
    s->dump_nodes = out;
    s->file_type = file_type;
}

void sceadan_set_ngram_mode(sceadan *s,int ngram_mode)
{
    s->ngram_mode = ngram_mode;
    // build feature mask if it is not loaded from a file
    if(s->mask_file==0 || s->mask_file[0]==0){
        sceadan_initialize_feature_mask(s);
    }
}

/* Structure to track the weight of each feature */
struct fweight {
    fweight(int _idx, double _w):idx(_idx),w(_w){}
    static bool comp(const fweight &w1, const fweight &w2) { return (w1.w > w2.w); }

    int    idx;
    double w;
};

int sceadan_reduce_feature(sceadan *s,const char *file_name,int n)
{
    int n_class = get_nr_class(s->model);
    int n_feature = get_nr_feature(s->model); 
    assert(n > 0 && n < n_feature);

    // generate feature mask using local index range [0, n_feature)
    int *mask_l = (int *) calloc(n_feature, sizeof(int));
    std::vector<fweight> ws;
    for(int i=0; i<n_class; i++){                           // select feature for each class
        for(int j=0; j<n_feature; j++){                     
            // s->model->w = [f1_c1, f1_c2, ... f1_cn, f2_c1 ... f2_cn, ... ] 
            ws.push_back(fweight(j, fabs(s->model->w[ i+j*n_class ])));
        }
        std::sort(ws.begin(), ws.end(), fweight::comp);
        // union selected features for different classes
        for(int k=0; k<n; k++){
            mask_l[ ws[k].idx ] = 1;
            //printf("%6d:%f   ", ws[k].idx, ws[k].w);
        }
        //printf("\n");
        ws.clear();
    }
    // convert index from local range [0, n_feature) to global range [1, MAX_NR_ATTR)
    char *mask_g = (char *) malloc(MAX_NR_ATTR*sizeof(char));
    memset(mask_g, '0', MAX_NR_ATTR);
    int count = 0;  // counting selected features
    int idx_l=0, idx_g=1;
    for(; idx_g<MAX_NR_ATTR; idx_g++){
        if(s->mask[idx_g]=='1'){
            if(mask_l[idx_l]){
                mask_g[idx_g] = '1';
                count++;
            }
            idx_l++;
        }
    }
    assert(idx_l == n_feature);
    
    free(mask_l);
    mask_l = NULL;

    printf("**********************************************************\n");
    printf("Feature reduction:\n");
    printf("    n = %d\n", n );
    printf("    nr_feature(before) = %d\n", n_feature );
    printf("    nr_feature(after)  = %d\n", count );
    printf("    reduction rate     = %f\n", float(count)/n_feature );
    printf("**********************************************************\n");

    assert(count <= n_feature);
    if(count==n_feature){
        // reduce no feature
        // caller of sceadan_app need to know this error code and decrease n
        free(mask_g);
        return -1;
    }
    // successfully reduce feature     
    free(s->mask);
    s->mask = mask_g;
    if(sceadan_dump_feature_mask(s, file_name) < 0){
        return -2;
    }
    return 0;
}
