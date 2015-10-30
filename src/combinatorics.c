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

unsigned long choose(int n, int k);
double binom(int n, int k);
unsigned long getHcf(unsigned long a, unsigned long b);
int min(int a, int b);

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  choose
 *  Description:  returns binomial of n over k
 * =====================================================================================
 */
    unsigned long 
choose(int n, int k)
{
    unsigned long num = 1;
    unsigned long den = 1;
    int i;
    for(i = 0; i < k; i++)
    {
        num *= (n-i);
        den *= (i+1);
        unsigned long div = getHcf(num, den);
        if (div > 0)
        {
            num /= div;
            den /= div;
        }
    }
    unsigned long sum = num/den;
    return sum;
}		/* -----  end of function choose  ----- */

/* 
* ===  FUNCTION  ======================================================================
*         Name:  binom
*  Description:  returns binomial of n over k
*                alternative method to choose
* =====================================================================================
*/
    double 
binom(int n, int k)
{
    if (k < 0)
    {
        return 0;
    }
    double* C = (double*)calloc(k+1, sizeof(double));
    int i, j;
    double res;
    C[0] = 1;
    for(i = 1; i <= n; i++)
    {
        for(j = min(i, k); j > 0; j--)
        {
            C[j] += C[j-1];
        }
    }
    res = C[k];  // Store the result before freeing memory
    free(C);  // free dynamically allocated memory to avoid memory leak
    return res;
}       /* -----  end of function binom  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  getHcf
 *  Description:  returns highest common factor
 * =====================================================================================
 */
    unsigned long
getHcf(unsigned long a, unsigned long b)
{
    unsigned long min = ( a > b ) ? b : a;
    unsigned long i;
    for (i = min; i >= 1; i--)
    {
        if ( a%i == 0 && b%i == 0 )
        {
            return i;
        }
    }
    return 0;
}		/* -----  end of function getHcf  ----- */

/* 
* ===  FUNCTION  ======================================================================
*         Name:  min
*  Description:  returns minimum of two values
* =====================================================================================
*/
    int 
min(int a, int b)
{
    return (a < b) ? a : b;
}       /* -----  end of function min  ----- */
