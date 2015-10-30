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
#include "efmMethods.c"

#define MAX_ARGS       1
#define ERROR_ARGS     1
#define ERROR_ZERO_NR  2
#define ERROR_FILE     3
#define ERROR_RAM      4

// calculate robustness of EFMs
    double 
getRobustness(unsigned int rx_count, int knockout, int card_step, int*
        efm_card)
{
    int i;
    double num = 0;
    double efm_count = 0;
    for (i = 0; i < card_step; i++)
    {
        num += (double) efm_card[i] * binom(rx_count - (i + 1), knockout); 
        efm_count += (double) efm_card[i];
    }
    double den = binom(rx_count, knockout) * efm_count;
    double robustness = 0;
    if (den > 0)
    {   
        robustness = num/den;
    }
    return robustness;
}

    double 
getOverallRobustness(unsigned int rx_count, int card_step, int* efm_card)
{
    double p = pow(2.0, (1.0/(double)rx_count)) - 1.0;
    double ov_rob = 0;
    int i;
    for (i = 0; i < rx_count; i++)
    {
        int ko = i + 1;
        double weight = pow(p, (double) ko) * binom(rx_count, ko);
        ov_rob += getRobustness(rx_count, ko, card_step, efm_card) * weight;
    }
    return ov_rob;
}

int main (int argc, char *argv[])
{
    // read arguments
    char *optv[MAX_ARGS] = { "-i" };
    char *optd[MAX_ARGS] = {"efm file  (tab separated like:  0.4\t0\t-0.24)"};
    char *optr[MAX_ARGS];
    char *description = "Calculates overall robustness of the network for EFMs\
                         with increasing cardinality";
    char *usg = "overallRobustnessByEfms -i efms.txt";

    readArgs(argc, argv, MAX_ARGS, optv, optr);

    // check if compulsory arguments are given
    if ( !optr[0] )
    {
        usage(description, usg, MAX_ARGS, optv, optd);
        quitError("Missing argument\n", ERROR_ARGS);
    }

    // define reaction count
    FILE *file = fopen(optr[0], "r");
    if (!file)
    {
        quitError("Error in opening file\n", ERROR_FILE);
    }
    int rx_count = getRxCount(file);
    fclose(file);
    if (rx_count < 1)
    {
        quitError("Error in EFM file format; number of reactions < 1\n",
                ERROR_FILE);
    }

    // allocate memory for EFM cardinalities
    int* efm_card = calloc(rx_count, sizeof(int));
    if (NULL == efm_card) 
    {
        quitError("Not enough free memory\n", ERROR_RAM);
    }

    // set counter for each EFM cardinality to 0
    int i;
    for (i = 0; i < rx_count; i++)
    {
        efm_card[i] = 0;
    }

    // read EFM file
    file = fopen(optr[0], "r");
    readEfmFile(rx_count, efm_card, file);
    fclose(file);

    // output header
    int line_length = 47;
    printLine('=', line_length);
    printf("%4s     %11s      %s\n", "n", "cardinality", "overall robustness(d)");
    printLine('-', line_length);

    // output overall robustness results for increasing EFM cardinalities 
    for (i = 1; i <= rx_count; i++)
    {
        double overall_rob = getOverallRobustness(rx_count, i, efm_card);
        printf("%4d     %11d               %.10lf\n", rx_count, i, overall_rob);
    }

    // print overall robustness
    printLine('-', line_length);

    // free memory
    free(efm_card);

    return EXIT_SUCCESS;
}
