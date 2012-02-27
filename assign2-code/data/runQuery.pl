die "Usage: <input query file> <output file> <url>" unless @ARGV==3;
my $infile = $ARGV[0];
my $outfile = $ARGV[1];
my $prefix = $ARGV[2];
# my $prefix = "http://ucair.cs.uiuc.edu/cgi-bin/yuelu2/cs410-assign2/LemurCGI";
open(IN, $infile) or die "cannot open $infile\n";
open(OUT, ">$outfile.result") or die "cannot open $outfile.result\n";
my $line;
while($line=<IN>){
    chomp($line);
    if($line=~/<DOC (\d+)>/){
	my $qid = $1;
	my $str = "?q=";
	while($line=<IN>){
		last if ($line=~/<\/DOC>/);
	    chomp($line);
		my @tt = split(/\s+/, $line);
		foreach (@tt){
		$str .= "$_+";
		}
	}
	my $url = "$prefix$str";

	system("wget $url -O $outfile.$qid.html");
	open(HTML, "$outfile.$qid.html") or die "cannot open $outfile.$qid.html\n";
	my $rank =1;
	while(<HTML>){
		if (/<font color="#888888" size="-1">\(([^<]+)\)<\/font>/){
			print OUT "$qid\t$1\t$rank\n";
			$rank++;		
		}
	}
	close(HTML);
    }
}
close(IN);
close(OUT);
