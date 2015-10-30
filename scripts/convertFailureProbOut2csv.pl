#!/usr/bin/env perl 

###############################################################################
## Author: Matthias P. Gerstl
## Email: matthias.gerstl@acib.at
## Company: Austrian Centre of Industrial Biotechnology (ACIB)
## Web: http:##www.acib.at
## Copyright (C) 2015
## Published unter GNU Public License V3
###############################################################################
## Basic Permissions.
## 
## All rights granted under this License are granted for the term of copyright
## on the Program, and are irrevocable provided the stated conditions are met.
## This License explicitly affirms your unlimited permission to run the
## unmodified Program. The output from running a covered work is covered by
## this License only if the output, given its content, constitutes a covered
## work. This License acknowledges your rights of fair use or other equivalent,
## as provided by copyright law.
## 
## You may make, run and propagate covered works that you do not convey,
## without conditions so long as your license otherwise remains in force. You
## may convey covered works to others for the sole purpose of having them make
## modifications exclusively for you, or provide you with facilities for
## running those works, provided that you comply with the terms of this License
## in conveying all material for which you do not control copyright. Those thus
## making or running the covered works for you must do so exclusively on your
## behalf, under your direction and control, on terms that prohibit them from
## making any copies of your copyrighted material outside their relationship
## with you.
## 
## Disclaimer of Warranty.
## 
## THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE
## LAW. EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND#OR
## OTHER PARTIES PROVIDE THE PROGRAM “AS IS” WITHOUT WARRANTY OF ANY KIND,
## EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
## WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE
## ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.
## SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY
## SERVICING, REPAIR OR CORRECTION.
## 
## Limitation of Liability.
## 
## IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING WILL
## ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MODIFIES AND#OR CONVEYS THE
## PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY
## GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE
## OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO LOSS OF DATA
## OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD
## PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS),
## EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF
## SUCH DAMAGES.
###############################################################################

use strict;
use warnings;
use utf8;
use Getopt::Std;

my $opt_string = 'hi:';
my %opt;
getopts( "$opt_string", \%opt ) or usage();
if ( $opt{h} or !$opt{i} ){
    usage();
}

my $in = readFile($opt{i});
my $inData = 0;

print "n,d,Pf.weighted,Pf,cutsets.total,cutsets.possible\n";

for (my $i = 0; $i < @{$in}; $i++) {
	if ($inData)
	{
		if ($in->[$i] =~ /---/)
		{
			last;
		}
		else
		{
			chomp($in->[$i]);
            $in->[$i] =~ s/^\s+//;
			my @spl = split(/\s+/, $in->[$i]);
			print join(',', @spl);
			print "\n";
		}
	}
	elsif ($in->[$i] =~ /---/)
	{
		$inData = 1;
	}
}

sub readFile {
    my ($file) = @_;
    open(IN, "<$file") or die ("Could not open $file for reading: $!\n");
    my @in = <IN>;
    close(IN);
    return \@in;
}

# HELP
#################################
sub usage {
    print <<"END_TEXT";

Usage: $0 [OPTIONS]


   -h    show this help
   -i    calcFailureProbability outputfile

   Examples:
   $0 -i input.file

END_TEXT

   exit( 0 );
}




