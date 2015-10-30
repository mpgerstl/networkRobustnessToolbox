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
#include <math.h>

#include "generalFunctions.c"
#include "combinatorics.c"
#include "pfMethods.c"

#define MAX_ARGS       2
#define ERROR_ARGS     1
#define ERROR_FILE     4
#define ERROR_RAM      5
#define ERROR_INPUT    6

struct dataset
{
    int rx_number;
    int deletions;
    long long total_cs;
    long long possible_cutsets;
};

/**
 * read input file and parse number of cutsets for each number of deletions
 */
int readCutsets(FILE *file, struct dataset** ds)
{
    int        alloc_size = 100;
    size_t     len        = 0;
    int        in_dataset = 0;
    int        counter    = 0;
    char*      line       = NULL;
    struct dataset* m_ds  = malloc(alloc_size * sizeof(struct dataset));

    if ( NULL == m_ds )
    {
        quitError("Not enough free memory\n", ERROR_RAM);
    }

    while (getline(&line, &len, file) != -1)
    {
        if (in_dataset)
        {
            if (line[0] == '-')
            {
                in_dataset = 0;
            }
            else
            {
                counter++;
                if (counter % alloc_size == 0)
                {
                    int new_size = counter + alloc_size;
                    m_ds = (struct dataset*) realloc(m_ds, new_size * sizeof(struct dataset));
                    if ( NULL == m_ds )
                    {
                        quitError("Not enough free memory\n", ERROR_RAM);
                    }
                }
                int i = 0;
                char* str;
                str = strtok(line, "\n ");
                int ix = counter - 1;
                while (str != NULL)
                {
                    i++;
                    if (i == 1)
                    {
                        m_ds[ix].rx_number = atoi(str);
                    }
                    else if (i == 2)
                    {
                        m_ds[ix].deletions = atoi(str);
                    }
                    else if (i == 5)
                    {
                        m_ds[ix].total_cs = atoll(str);
                    }
                    else if (i == 6)
                    {
                        m_ds[ix].possible_cutsets = atoll(str);
                    }
                    str = strtok(NULL, "\n ");
                }
                if (i != 6)
                {
                    quitError("Not a valid input file\n", ERROR_FILE);
                }
            }
        }
        else if (line[0] == '-')
        {
            in_dataset = 1;
        }
    }
    free(line);
    int new_size = counter;
    *ds = (struct dataset*) realloc(m_ds, new_size * sizeof(struct dataset));
    if ( NULL == ds )
    {
        quitError("Not enough free memory\n", ERROR_RAM);
    }
    if (new_size < 1)
    {
        quitError("The input file seems not to be valid as the dataset is 0\n", ERROR_FILE);
    }
    return counter;
}

int main (int argc, char *argv[])
{
    // read arguments
    char *optv[MAX_ARGS] = { "-i", "-l" };
    char *optd[MAX_ARGS] = { 
        "output of calcFailureProbability", 
        "lambda = weighting factor ( > 0 )" 
    };
    char *optr[MAX_ARGS];
    char *description = "Recalculate failure probability with new lambda value";
    char *usg = "recalcFailureProbability -i calcFailureProbability.output -l 0.1";

    readArgs(argc, argv, MAX_ARGS, optv, optr);

    // check if compulsory arguments are given
    if ( (!optr[0]) || (!optr[1]) )
    {
        usage(description, usg, MAX_ARGS, optv, optd);
        quitError("Missing argument\n", ERROR_ARGS);
    }
    
    // define lambda for weighting function
    double lambda = atof(optr[1]);
    if (lambda <= 0)
    {
        quitError("lambda needs to be greater than zero\n\n", ERROR_ARGS);
    }

    // read calculated number of cutsets from old file
    struct dataset* dataset = NULL;
    FILE *file = fopen(optr[0], "r");
    if (!file)
    {
        quitError("Error in opening file\n", ERROR_FILE);
    }
    int max_card = readCutsets(file, &dataset);
    fclose(file);

    int line_length = 111;

    // print output header to stdout
    printLine('=', line_length);
    printf("%4s     %3s    %20s     %10s     %25s     %25s\n", "n", "d",
            "weighted P(f)", "P(f)", "total cutsets(d)", 
            "possible cutsets(d)");
    printf("%19slambda = %1.2e\n"," ", lambda);
    printLine('-', line_length);

    // calculate failure probability
    int    card;
    double total_weight_pF = 0;
    double left_weight = 1 - exp(-lambda);
    for (card = 0; card < max_card; card++)
    {
        double failure = (double)dataset[card].total_cs/(double)dataset[card].possible_cutsets;
        double weight = getWeight(lambda, card+1);
        double weight_pF = weight * failure;

        total_weight_pF += weight_pF;
        left_weight -= weight;

        printf("%4d     %3d            %.10lf     %.8f     %25.0lld     %25.0lld", 
                dataset[card].rx_number, card + 1, weight_pF, failure,
                dataset[card].total_cs, dataset[card].possible_cutsets);

        printf("\n");
    }

    // weight cannot be less than 0; values < 0 are results of numerical
    // instability of double values
    if (left_weight < 0)
    {
        left_weight = 0;
    }

    // print footer to stdout
    printLine('-', line_length);
    printf("total P(f)              %3.10lf\n", total_weight_pF);
    printf("Error                   %3.10f\n", left_weight); 
    printLine('=', line_length);

    free(dataset);

    return EXIT_SUCCESS;
}
