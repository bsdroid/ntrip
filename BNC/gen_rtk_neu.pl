#!/usr/bin/perl -w

use strict;

use lib "$ENV{HOME}/gpss_src/perl";
use geotools;

my($inFile, $outFile) = @ARGV;
if (!defined($outFile)) {
  die "Usage: gen_rtk_neu.pl inFile outFile\n";
}

open(inFile , $inFile);
open(outFile, ">$outFile");

###my $xApr = 0.0;
###my $yApr = 0.0;
###my $zApr = 0.0;

my $xApr = 4331299.9;
my $yApr =  567537.2;
my $zApr = 4633133.7;

while ( my $line=<inFile> ) {

  if ($line =~ /PPP/) {

    my @p = split(/\s+/, $line);
    
    my $time = $p[4];
    my $x    = $p[6];
    my $y    = $p[7];
    my $z    = $p[8];
    
    if ($xApr == 0.0 && $yApr == 0.0 && $zApr == 0.0) {
        $xApr = $x;
        $yApr = $y;
        $zApr = $z;
    }
    
    my $dx   = $x - $xApr;
    my $dy   = $y - $yApr;
    my $dz   = $z - $zApr;

    my ($n, $e, $u) = get_neu($dx, $dy, $dz, $xApr, $yApr, $zApr);

    printf(outFile "%s %8.4f %8.4f %8.4f\n", $time, $n, $e, $u);
  }
}

close inFile;
close outFile;
