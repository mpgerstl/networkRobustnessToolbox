///////////////////////////////////////////////////////////////////////////////
// Author: Matthias P. Gerstl
// Email: matthias.gerstl@acib.at
// Company: Austrian Centre of Industrial Biotechnology (ACIB)
// Web: http://www.acib.at
// Copyright (C) 2015
// Published unter GNU Public License V3
///////////////////////////////////////////////////////////////////////////////
// Basic Permissions.
// 
// All rights granted under this License are granted for the term of copyright
// on the Program, and are irrevocable provided the stated conditions are met.
// This License explicitly affirms your unlimited permission to run the
// unmodified Program. The output from running a covered work is covered by
// this License only if the output, given its content, constitutes a covered
// work. This License acknowledges your rights of fair use or other equivalent,
// as provided by copyright law.
// 
// You may make, run and propagate covered works that you do not convey,
// without conditions so long as your license otherwise remains in force. You
// may convey covered works to others for the sole purpose of having them make
// modifications exclusively for you, or provide you with facilities for
// running those works, provided that you comply with the terms of this License
// in conveying all material for which you do not control copyright. Those thus
// making or running the covered works for you must do so exclusively on your
// behalf, under your direction and control, on terms that prohibit them from
// making any copies of your copyrighted material outside their relationship
// with you.
// 
// Disclaimer of Warranty.
// 
// THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE
// LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR
// OTHER PARTIES PROVIDE THE PROGRAM “AS IS” WITHOUT WARRANTY OF ANY KIND,
// EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE
// ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.
// SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY
// SERVICING, REPAIR OR CORRECTION.
// 
// Limitation of Liability.
// 
// IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL
// ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MODIFIES AND/OR CONVEYS THE
// PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY
// GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE
// OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF DATA
// OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD
// PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS),
// EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGES.
///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

#include "generalFunctions.c"
#include "combinatorics.c"
#include "bitmakros.h"
#include "pfMethods.c"

#define BITSIZE        CHAR_BIT
#define MAX_ARGS       5
#define ERROR_ARGS     1
#define ERROR_THREADS  2
#define ERROR_ZERO_NR  3
#define ERROR_FILE     4
#define ERROR_RAM      5
#define ERROR_INPUT    6
#define ERROR_ALG      7
#define PROGRESS_STEPS 80

static unsigned long progressIndex      = 0;
static unsigned long progressCardLength = 0;
static int           progressLocked     = 0;
static int           actProgressChars   = 0;
static unsigned long steps[PROGRESS_STEPS];
static time_t        starttime; 

// structure needed for multithreading
struct thread_args
{
    int             max_threads;
    int             thread_id;
    int             card;
    int             bitlength;
    int             max_card;
    int             red_rx_count;
    char**          reduced_matrix;
    unsigned long*  start_indices;
    unsigned long*  mcs_card_sum;
    unsigned long** cutsets;
};

void printHeader(FILE *file_out, int line_length, double lambda)
{
    // print output header to stdout
    printLine('=', line_length);
    printf("%4s     %3s    %20s     %10s     %25s     %25s\n", "n", "d",
            "weighted P(f)", "P(f)", "total cutsets(d)", 
            "possible cutsets(d)");
    printf("%19slambda = %1.2e\n"," ", lambda);
    printLine('-', line_length);

    // print output header to output file
    fprintLine(file_out, '=', line_length);
    fprintf(file_out, "%4s     %3s    %20s     %10s     %25s     %25s\n", "n",
            "d", "weighted P(f)", "P(f)", "total cutsets(d)", 
            "possible cutsets(d)");
    fprintf(file_out, "%19slambda = %1.2e\n"," ", lambda);
    fprintLine(file_out, '-', line_length);
    fflush(file_out);
}

void printFooter(FILE *file_out, int line_length, double total_weight_pF, double left_weight)
{
    // print footer to stdout
    printLine('-', line_length);
    printf("total P(f)              %3.10lf\n", total_weight_pF);
    printf("Error                   %3.10f\n", left_weight); 
    printLine('=', line_length);

    // print footer to output file
    fprintLine(file_out, '-', line_length);
    fprintf(file_out, "total P(f)              %3.10lf\n", total_weight_pF);
    fprintf(file_out, "Error                   %3.10f\n", left_weight); 
    fprintLine(file_out, '=', line_length);
    fflush(file_out);
}

/**
 * return 1 if mcs and stored cutset overlaps at least 1 time
 */
int contradictNotAllowedReactions(char* mcs, char* stored, int rx_count)
{
    int i = 0;
    for (i = 0; i < rx_count; i++)
    {
        if (BITTEST(stored, i) && BITTEST(mcs, i))
        {
            return 1;
        }
    }
    return 0;
}

/**
 * count set bits of mcs that are not set on active mcs
 */
int getNotActiveReactionCount(char *mcs, char* active, int rx_count)
{
    int count = 0;
    int i     = 0;
    for (i = 0; i < rx_count; i++)
    {
        if (BITTEST(mcs, i) && !BITTEST(active,i))
        {
            count++;
        }
    }
    return count;
}

/**
 * calculates needed number of chars for bit support of mcs
 */
int getBitsize(int rx_count)
{
    int rx_size = rx_count / BITSIZE;
    int modulo  = rx_count % BITSIZE;
    if (modulo > 0)
    {
        rx_size++;
    }
    int bitsize = rx_size * BITSIZE;
    return bitsize;
}

/**
 * reads first line of MCS file to check if the file is in correct format
 */
int isValidInputFile(char *filename)
{
    int    valid = 0;
    size_t len   = 0;
    char*  line  = NULL;
    FILE*  file  = fopen(filename, "r");
    if (!file)
    {
        quitError("Error in opening file\n", ERROR_FILE);
    }
    if ( getline(&line, &len, file) != -1)
    {
        int rx_count = (int) strlen(line) - 1;
        char allowed[] = "01\n";
        int pos = strspn(line, allowed);
        if (pos < 0 || pos > rx_count)
        {
            valid = 1;
        }
    }
    free(line);
    line = NULL;
    fclose(file);
    return valid;
}

/**
 * read first line mcs file and returns number of reactions
 */
int getReactionCount(char *filename)
{
    int    rx_count = 0;
    char*  line     = NULL;
    size_t len      = 0;
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        quitError("Error in opening file\n", ERROR_FILE);
    }
    if ( getline(&line, &len, file) != -1)
    {
        rx_count = (int) strlen(line) - 1;
    }
    free(line);
    line = NULL;
    fclose(file);
    return(rx_count);
}

/**
 * read mcs file, consisting of 0 and 1 without spaces
 * stores values in a bit matrix
 * define cardinality for each mcs
 * calculates cardinality summary over all mcs
 */
void readInitialMatrix(int rx_count, unsigned long* mcs_count, char***
        m_initial_mat, int** m_mcs_card, unsigned long* mcs_card_sum, int
        bitarray_size, char *filename)
{
    int    alloc_size  = 10000;
    char** initial_mat = NULL;
    int*   mcs_card    = NULL;
    char*  line        = NULL;
    int i;
    for (i = 0; i < rx_count; i++){
        mcs_card_sum[i] = 0;
    }
    size_t        len      = 0;
    unsigned long mat_ix   = 0;
    unsigned long new_size = 0;
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        quitError("Error in opening file\n", ERROR_FILE);
    }
    
    while ( getline(&line, &len, file) != -1)
    {
        // reallocate for new mcs in file
        if (mat_ix % alloc_size == 0)
        {
            new_size = mat_ix + alloc_size;
            initial_mat = (char**) realloc(initial_mat, new_size *
                    sizeof(char*));
            mcs_card = (int*) realloc(mcs_card, new_size * sizeof(int));
            if ( (NULL == initial_mat) || (NULL == mcs_card) )
            {
                quitError("Not enough free memory in readInitialMatrix\n", ERROR_RAM);
            }
        }

        // allocate for actual mcs
        initial_mat[mat_ix] = calloc(1, bitarray_size);
        if (NULL == initial_mat[mat_ix])
        {
            quitError("Not enough free memory\n", ERROR_RAM);
        }

        // store mcs in matrix and define cardinality
        int card = 0;
        for (i = 0; i < rx_count; i++)
        {
            if (line[i] == '1')
            {
                BITSET(initial_mat[mat_ix], i);
                card++;
            } 
        }
        mcs_card[mat_ix] = card - 1;
        mcs_card_sum[card-1]++;
        mat_ix++;
    }

    // set return pointer
    *mcs_count     = mat_ix;
    *m_initial_mat = (char**) realloc(initial_mat, mat_ix * sizeof(char*));
    *m_mcs_card    = (int*) realloc(mcs_card, mat_ix * sizeof(int));
    if ( (NULL == *m_initial_mat) || (NULL == *m_mcs_card) )
    {
        quitError("Not enough free memory\n", ERROR_RAM);
    }
    free(line);
    line = NULL;
    fclose(file);
}

/**
 * remove single knockouts from matrix
 *   - remove rows with single knockouts
 *   - remove columns where single knockouts == 1
 * remove knockouts from matrix with higher cardinality than max_card
 */
void processMatrix(int rx_count, unsigned long mcs_count, char** initial_mat,
        int* mcs_card, unsigned long* mcs_card_sum, int bitarray_size, int
        red_rx_count, unsigned long red_mcs_count, char** red_mat, int
        red_bitarray_size, int max_card)
{
    // allocate memory for columnts of reduced matrix
    int* left_cols = calloc(red_rx_count, sizeof(int));

    // remove single knockouts
    if (mcs_card_sum[0] > 0)
    {
        // define bitset for single knockouts
        char* single_ko_cols = calloc(1, bitarray_size);
        if (NULL == single_ko_cols)
        {
            quitError("Not enough free memory\n", ERROR_RAM);
        }

        // find reactions of single knockouts
        unsigned long li = 0;
        for (li = 0; li < mcs_count; li++)
        {
            if (mcs_card[li] == 0)
            {
                int j = 0;
                for (j = 0; j < rx_count; j++)
                {
                    if (BITTEST(initial_mat[li],j))
                    {
                        BITSET(single_ko_cols, j);
                        j = rx_count;
                    }
                }
            }
        }

        // define columns that are not used by single knockouts
        int i = 0;
        int j = 0;
        for (i = 0; i < rx_count; i++){
            if (!(BITTEST(single_ko_cols, i))){
                left_cols[j] = i;
                j++;
            }
        }
        free(single_ko_cols);
        single_ko_cols = NULL;
    } 
    else 
    {
        // no single knockout, set left columns to all columns
        int i = 0;
        for (i = 0; i < red_rx_count; i++)
        {
            left_cols[i] = i;
        }
    }

    // reduce matrix
    int           i  = 0;
    unsigned long li = 0;
    for (i = 1; i < max_card; i++)
    {
        // if exist mcs with cardinality i
        if (mcs_card_sum[i] > 0)
        {
            // compare with all mcs and sort mcs by cardinality
            unsigned long mcs_ix = 0;
            for (mcs_ix = 0; mcs_ix < mcs_count; mcs_ix++)
            {
                // add all mcs with cardinality i to matrix
                if (mcs_card[mcs_ix] == i)
                {
                    red_mat[li] = calloc(1, red_bitarray_size);
                    int k = 0;
                    for (k = 0; k < red_rx_count; k++)
                    {
                        if (BITTEST(initial_mat[mcs_ix],left_cols[k]))
                        {
                            BITSET(red_mat[li],k);
                        }
                    }
                    li++;
                }
            }
        }
    }
    free(left_cols);
    left_cols = NULL;
}

/*
 * main part of the algorithm
 * searches cutsets out of mcs without considering a cutset more than once
 */
void recursiveCutsetSearch(char** reduced_matrix, unsigned long mcs_index,
        char* active, char* stored, int red_rx_count, int bitlength, int
        max_card, unsigned long* cutsets, int inRecursion)
{
    // prepare memory
    int            comb_card = 0;
    unsigned long  still_tocheck_count = 0;
    unsigned long* still_tocheck_ix = calloc(mcs_index, sizeof(unsigned long));
    unsigned long* comb_cutsets = calloc(max_card, sizeof(unsigned long));
    char*          comb_active = calloc(1, bitlength);
    char*          comb_stored = calloc(1, bitlength);
    if ( (NULL == still_tocheck_ix) || (NULL == comb_active) || 
         (NULL == comb_stored) || (NULL == comb_cutsets) )
    {
        quitError("Not enough free memory\n", ERROR_RAM);
    }

    // copy stored and actualize active reactions
    int i;
    for (i = 0; i < red_rx_count; i++)
    {
        if ( (BITTEST(reduced_matrix[mcs_index], i)) || (BITTEST(active, i)) )
        {
            BITSET(comb_active, i);
            comb_card++;
        }
        if (BITTEST(stored, i))
        {
            BITSET(comb_stored, i);
        }
    }
    comb_card--;

    // prepare cutset search
    int found_subset = 0;
    unsigned long li;
    for (li = 0; li < mcs_index; li++)
    {
        if (!contradictNotAllowedReactions(reduced_matrix[li], comb_stored,
                    red_rx_count))
        {
            int left_rx = getNotActiveReactionCount(reduced_matrix[li],
                    comb_active, red_rx_count);
            if (left_rx == 1)
            {
                int j;
                for (j = 0; j < red_rx_count; j++)
                {
                    if (BITTEST(reduced_matrix[li], j) &&
                            (!BITTEST(comb_active, j)))
                    {
                        BITSET(comb_stored, j);
                        break;
                    }
                }
            }
            else if (left_rx > 1)
            {
                still_tocheck_ix[still_tocheck_count] = li;
                still_tocheck_count++;
            }
            else
            {
                // is a subset
                found_subset = 1;
                break;
            }
        }
    }
    if (!found_subset)
    {
        // define degree of freedom for combined mcs
        int dof = 0;
        int i;
        for (i = 0; i < red_rx_count; i++)
        {
            if ( (!BITTEST(comb_stored,i)) && (!BITTEST(comb_active,i)) )
            {
                dof++;
            }
        }
        for (i = comb_card; i < max_card; i++)
        {
            comb_cutsets[i] = choose(dof, i - comb_card);
        }
        if ( (comb_card + 1) < max_card)
        {
            if ( comb_cutsets[comb_card + 1] > 0 )
            {
                unsigned long li;
                for (li = 0; li < still_tocheck_count; li++)
                {
                    if (!contradictNotAllowedReactions(
                           reduced_matrix[still_tocheck_ix[li]], 
                           comb_stored, red_rx_count))
                    {
                        int left_rx = getNotActiveReactionCount(
                                reduced_matrix[still_tocheck_ix[li]],
                                comb_active, red_rx_count);
                        if (left_rx > 1)
                        {
                            recursiveCutsetSearch(reduced_matrix,
                                    still_tocheck_ix[li], comb_active,
                                    comb_stored, red_rx_count, bitlength,
                                    max_card, comb_cutsets, 1);
                        }
                        else
                        {
                            quitError("recursiveCutsetSearch: You should not \
end up here: left_rx <= 1\n", ERROR_ALG);
                        }
                    } 
                }
            }
        }
        if (inRecursion)
        {
            for (i = comb_card; i < max_card; i++)
            {
                cutsets[i] = cutsets[i] - comb_cutsets[i];
                if (cutsets[i] < 0)
                {
                    cutsets[i] = 0;
                }
            }
        }
        else
        {
            for (i = comb_card; i < max_card; i++)
            {
                cutsets[i] = comb_cutsets[i];
            }
        }
    }
    // free memory
    free(comb_cutsets);
    free(comb_active);
    free(comb_stored);
    free(still_tocheck_ix);
    comb_cutsets = NULL;
    comb_active = NULL;
    comb_stored = NULL;
    still_tocheck_ix = NULL;
}

/**
 * resets variables needed to show progress of calculation
 */
void resetStaticProgressVariables(unsigned long card_length)
{
    progressIndex = 0;
    progressCardLength = card_length; 
    unsigned long div = (unsigned long) progressCardLength/PROGRESS_STEPS;
    steps[0] = 0;
    int i;
    for (i = 1; i < PROGRESS_STEPS; i++)
    {
        steps[i] = i * div;
    }
    starttime = time(NULL);
    actProgressChars = 0;
}

/**
 * deletes progress bar from stdout
 */
void clearProgress()
{
    int ri;
    int add = 40;
    for (ri = 0; ri < add; ri++)
    {
        printf(" ");
    }
    int all = PROGRESS_STEPS + 2*add;
    for (ri = 0; ri < all; ri++)
    {
        printf("\b");
    }
    fflush(stdout);
}

/**
 * print progress bar and expected left seconds for the calculation
 */
void printProgress()
{
    progressIndex++;
    // only update progress if it is not done by other thread
    if (!progressLocked)
    {
        // lock progress for other threads
        progressLocked = 1;

        // define progress bar length
        int i;
        for (i = actProgressChars; i < PROGRESS_STEPS; i++)
        {
            if (progressIndex >= steps[i])
            {
                actProgressChars = i;
            }
            else
            {
                i = PROGRESS_STEPS;
            }
        }

        // clear progress bar and print new one
        clearProgress();
        printf("[");
        for (i = 0; i < actProgressChars; i++)
        {
            printf("=");
        }
        printf(">");
        for (i = actProgressChars; i < PROGRESS_STEPS; i++)
        {
            printf(" ");
        }
        printf("] ");

        // print progress percentage
        double prg = 100 * (double)progressIndex / (double)progressCardLength;
        if (prg < 10)
        {
            printf(" ");
        }
        printf("%.2f%%", prg);

        // print expected left runtime for this step
        time_t acttime = time(NULL);
        if (acttime > starttime)
        {
            int diff = (int)acttime - (int)starttime;
            double expected = diff * (100-prg) / prg;
            printf(" (%d sec) ", (int)expected);
        }
        else 
        {
            printf(" (??? sec) ");
        }
        fflush(stdout);

        // unlock progress for other threads
        progressLocked = 0;
    }
}

/**
 * multithread function to define cutsets
 * for each mcs
 */
void *analyseMcs(void *pointer_thread_args)
{
    // unpack given arguments
    struct thread_args* thread_args    = (struct thread_args*)
        pointer_thread_args;
    int                 thread_id      = thread_args->thread_id;
    int                 max_threads    = thread_args->max_threads;
    int                 bitlength      = thread_args->bitlength;
    int                 card           = thread_args->card;
    int                 max_card       = thread_args->max_card;
    int                 red_rx_count   = thread_args->red_rx_count;
    unsigned long*      start_indices  = thread_args->start_indices;
    unsigned long*      mcs_card_sum   = thread_args->mcs_card_sum;
    char**              reduced_matrix = thread_args->reduced_matrix;
    unsigned long**     cutsets        = thread_args->cutsets;

    // check every mcs of cardinality card
    unsigned long index;
    unsigned long last = start_indices[card] + mcs_card_sum[card];
    for (index = start_indices[card]; index < last; index++)
    {
        // check if mcs is calculated by this thread
        if ( (index % max_threads) == thread_id)
        {
            printProgress();
            char* stored = calloc(1, bitlength);
            if ( NULL == stored )
            {
                quitError("Not enough free memory\n", ERROR_RAM);
            }
            recursiveCutsetSearch(reduced_matrix, index, reduced_matrix[index],
                    stored, red_rx_count, bitlength, max_card, cutsets[index],
                    0);
            free(stored);
            stored = NULL;
        }
    }
    return((void *)NULL);
}

/**
 * prepare analyses of each mcs with given cardinality
 * calls multithreaded analyseMcs
 *   - card = cardinality - 1!
 *       if cardinality of mcs = 2 then card = 1
 */
void analyseCard(char** reduced_matrix, unsigned long red_mcs_count, int
        red_rx_count, unsigned long* mcs_card_sum, unsigned long*
        start_indices, int card, int max_threads, int max_card, unsigned long**
        cutsets)
{
    if (mcs_card_sum[card] > 0)
    {
        // define bitlength
        int bitlength = getBitsize(red_rx_count);

        // prepare threads
        pthread_t thread[max_threads];
        struct thread_args thread_args[max_threads];
        int i;
        for (i = 0; i < max_threads; i++)
        {
            thread_args[i].thread_id      = i;
            thread_args[i].max_threads    = max_threads;
            thread_args[i].bitlength      = bitlength;
            thread_args[i].card           = card;
            thread_args[i].max_card       = max_card;
            thread_args[i].red_rx_count   = red_rx_count;
            thread_args[i].start_indices  = start_indices;
            thread_args[i].mcs_card_sum   = mcs_card_sum;
            thread_args[i].reduced_matrix = reduced_matrix;
            thread_args[i].cutsets        = cutsets;
        }

        resetStaticProgressVariables(mcs_card_sum[card]);

        // start threads
        for (i = 0; i < max_threads; i++)
        {
            pthread_create(&thread[i], NULL, analyseMcs, 
                    (void *)&thread_args[i]);
        }

        // join threads
        for (i = 0; i < max_threads; i++)
        {
            pthread_join(thread[i], NULL);
        }
    }
}

/**
 * calculate number of all cutsets for given cardinality
 */
unsigned long getCutsets(int card, unsigned long** cutsets, unsigned long*
        start_indices, unsigned long* mcs_card_sum, int total_rx_count)
{
    // number of mcs with cardinality card
    unsigned long card_cutsets = mcs_card_sum[card];
    if (card > 0)
    {
        int i = 0;
        // consider single knockouts
        for (i = 0; i < mcs_card_sum[0]; i++)
        {
            card_cutsets += choose( (total_rx_count - i - 1), card );
        }
        unsigned long li = 0;
        for (li = start_indices[1]; li < start_indices[card]; li++)
        {
            card_cutsets += cutsets[li][card];
        }
    }
    return card_cutsets;
}

int main (int argc, char *argv[])
{
    // read arguments
    char *optv[MAX_ARGS] = { "-i", "-m", "-l", "-t", "-o" };
    char *optd[MAX_ARGS] = { "mcs file in form of 000110", 
        "maximum number of knockouts [default=number of reactions]", 
        "lambda = weighting factor ( > 0 ) [default=0.5]",
        "number of threads [default=1]",
        "output file"};
    char *optr[MAX_ARGS];
    char *description = "Calculate failure probability of the network for \
                         increasing number of knockouts";
    char *usg = "failureProbabilityByMcs -i mcs.csv -m 6 -l 0.1 -t 6 -o failure.out";

    readArgs(argc, argv, MAX_ARGS, optv, optr);

    // check if compulsory arguments are given
    if ( (!optr[0]) || (!optr[4]) )
    {
        usage(description, usg, MAX_ARGS, optv, optd);
        quitError("Missing argument\n", ERROR_ARGS);
    }

    if (!isValidInputFile(optr[0]))
    {
        quitError("MCS file is not valid\n", ERROR_FILE);
    }

    // open output file
    FILE *file_out = fopen(optr[4], "w");
    if (!file_out)
    {
        quitError("Error in opening outputfile\n", ERROR_FILE);
    }

    // define number of reactions
    int rx_count = getReactionCount(optr[0]);
    if (rx_count < 1)
    {
        quitError("\nNumber of reactions = 0. That is not possible\n\n",
                ERROR_ZERO_NR);
    }

    // define maximum number of deletions
    int max_card = rx_count;
    if (optr[1])
    {
        max_card = atoi(optr[1]);
    }
    if (max_card < 1)
    {
        quitError("Number of maximum knockouts < 1\n\n", ERROR_ARGS);
    }
    if (max_card > rx_count)
    {
        max_card = rx_count;
    }

    // define lambda for weighting function
    double lambda = 0.5;
    if (optr[2])
    {
        lambda = atof(optr[2]);
    }
    if (lambda <= 0)
    {
        quitError("lambda needs to be greater than zero\n\n", ERROR_ARGS);
    }

    // define number of threads to use
    int max_threads = 1;
    if (optr[3])
    {
        max_threads = atoi(optr[3]);
    }
    if (max_threads < 1)
    {
        quitError("Number of threads < 1\n\n", ERROR_THREADS);
    }

    // allocate memory for bit matrix and cardinalities
    int            bitarray_size = getBitsize(rx_count);
    char**         initial_mat   = NULL;
    int*           mcs_card      = NULL;
    unsigned long  mcs_count     = 0;
    unsigned long* mcs_card_sum  = calloc(1, rx_count * sizeof(unsigned long));
    if ( (NULL == mcs_card_sum) )
    {
        quitError("Not enough free memory for mcs_card_sum\n", ERROR_RAM);
    }

    // read mcs matrix
    readInitialMatrix(rx_count, &mcs_count, &initial_mat, &mcs_card,
            mcs_card_sum, bitarray_size, optr[0]);

    // prepare matrix reduction
    int           red_rx_count      = rx_count - mcs_card_sum[0];
    int           red_bitarray_size = getBitsize(red_rx_count);
    unsigned long red_mcs_count     = mcs_count - mcs_card_sum[0];
    int i;
    for (i = max_card; i < rx_count; i++)
    {
        red_mcs_count -= mcs_card_sum[i];
    }

    // reduce matrix
    char** reduced_mat = malloc(red_mcs_count * sizeof(char *));
    processMatrix(rx_count, mcs_count, initial_mat, mcs_card, mcs_card_sum,
            bitarray_size, red_rx_count, red_mcs_count, reduced_mat,
            red_bitarray_size, max_card);

    // free memory of initial matrix
    unsigned long li;
    for (li=0; li<mcs_count; li++)
    {
        free(initial_mat[li]);
    }
    free(initial_mat);
    initial_mat = NULL;

    // define start indices for cardinalities in reduced matrix
    // cardinalities start with 0: card 1 = 0; card 2 = 1; ...
    // in reduced matrix cardinality 2 starts in row 0 !!!
    unsigned long* start_indices = malloc(rx_count * sizeof(unsigned long));
    for (i = 0; i < red_rx_count; i++)
    {
        if (i < 2)
        {
            start_indices[i] = 0;
        }
        else
        {
            start_indices[i] = start_indices[i-1] + mcs_card_sum[i-1];
        }
    }
    for (i = red_rx_count; i < rx_count; i++)
    {
        start_indices[i] = start_indices[red_rx_count - 1] +
            mcs_card_sum[red_rx_count - 1];
    }

    // allocate memory for cutsets
    unsigned long** cutsets = calloc(red_mcs_count, sizeof(double*));
    if (red_mcs_count > 0)
    {
        for (li = 0; li < red_mcs_count; li++)
        {
            cutsets[li] = calloc(rx_count, sizeof(double));
        }
        if (NULL == cutsets[red_mcs_count - 1])
        {
            quitError("Not enough free memory for cutsets\n", ERROR_RAM);
        }
    }

    int line_length = 111;

    printHeader(file_out, line_length, lambda);

    // calculate failure probability
    int    card;
    double total_weight_pF = 0;
    double left_weight = 1 - exp(-lambda);
    for (card = 0; card < max_card; card++)
    {
        if (card > 0)
        {
            analyseCard(reduced_mat, red_mcs_count, red_rx_count, mcs_card_sum,
                    start_indices, card, max_threads, max_card, cutsets);
        }
        unsigned long card_cutsets = getCutsets(card, cutsets, start_indices,
                mcs_card_sum, rx_count);
        unsigned long all_possible = choose(rx_count, (card + 1));
        double failure = (double)card_cutsets/(double)all_possible;
        double weight = getWeight(lambda, card+1);
        double weight_pF = weight * failure;

        total_weight_pF += weight_pF;
        left_weight -= weight;

        // print result of failure probability of actual number of deletions to
        // stdout
        clearProgress();
        printf("%4d     %3d            %.10lf     %.8f     %25.0lu     %25.0lu",
                rx_count, card + 1, weight_pF, failure, card_cutsets,
                all_possible);

        // clear left overs from progress bars
        int oi;
        for (oi = 0; oi < PROGRESS_STEPS; oi++)
        {
            printf(" ");
        }
        for (oi = 0; oi < PROGRESS_STEPS; oi++)
        {
            printf("\b");
        }
        printf("\n");

        // print result of failure probability of actual number of deletions to
        // output file
        fprintf(file_out, 
                "%4d     %3d            %.10lf     %.8f     %25.0lu     %25.0lu\n",
                rx_count, card + 1, weight_pF, failure, card_cutsets,
                all_possible);
        fflush(file_out);
    }

    // weight cannot be less than 0; values < 0 are results of numerical
    // instability of double values
    if (left_weight < 0)
    {
        left_weight = 0;
    }

    printFooter(file_out, line_length, total_weight_pF, left_weight);

    // prepare exit
    fclose(file_out);
    for (i = 0; i < red_mcs_count; i++)
    {
        free(cutsets[i]);
        free(reduced_mat[i]);
    }
    free(cutsets);
    free(reduced_mat);
    free(mcs_card);
    free(mcs_card_sum);
    free(start_indices);

    return EXIT_SUCCESS;
}
