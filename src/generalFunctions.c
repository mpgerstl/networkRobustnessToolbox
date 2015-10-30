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

#include <string.h>

char* getArg ( int argc, char *argv[], char *opt );
void readArgs ( int argc, char *argv[], int optc, char *optv[], char *optr[] );
void usage ( char* description, char* usage, int optc, char* optv[], char *optd[] );
void quitError( char* message, int rv );
void printLine(char c, int length);
void fprintLine(FILE *file, char c, int length);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  usage
 *  Description:  prints usage of a program
 * =====================================================================================
 */
    void
usage ( char* description, char* usage, int optc, char* optv[], char *optd[] )
{
    printf("\n%s\n", description);
    printf("\nusage: %s\n", usage);
    printf("arguments:\n");
    unsigned int i;
    for (i = 0; i < optc; i++) {
        printf("   %s\t%s\n", optv[i], optd[i]);
    }
    printf("\n");
}		/* -----  end of function usage  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getArg
 *  Description:  returns value of asked argument
 * =====================================================================================
 */
    char*
getArg ( int argc, char *argv[], char *opt )
{
    char *ret = NULL;
    unsigned int i;
    for (i = 0; i < (argc-1); i++) {
        if (!strcmp(argv[i], opt)){
            ret = argv[i+1];
            break;
        }
    }
    return ret;
}		/* -----  end of function getArg  ----- */



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  readArgs
 *  Description:  reads all values of asked arguments
 * =====================================================================================
 */
    void
readArgs ( int argc, char *argv[], int optc, char *optv[], char *optr[] )
{
    unsigned int i;
    for (i = 0; i < optc; i++) {
        optr[i] = getArg(argc, argv, optv[i]);
    }
}		/* -----  end of function readArgs  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  quitError
 *  Description:  exit a program with an error message
 * =====================================================================================
 */
    void
quitError ( char *message, int rv )
{
    fprintf( stderr, "%s", message );
    exit(rv);
}		/* -----  end of function quitError  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  printLine
 *  Description:  print horizontal line of length chars to STDOUT
 * =====================================================================================
 */
    void 
printLine(char c, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        printf("%c", c);
    }
    printf("\n");
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  printLine
 *  Description:  print horizontal line of length chars to file
 * =====================================================================================
 */
    void 
fprintLine(FILE *file, char c, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        fprintf(file, "%c", c);
    }
    fprintf(file, "\n");
}

