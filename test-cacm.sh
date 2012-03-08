#!/bin/bash

# rsync -r -a -v -e "ssh -l ${user}" assign2-code/ projects.cs.illinois.edu:~/assign2-code/

engine=comp1

cd assign2-code
gmake
cp app/obj/LemurCGI ../public_html/cgi-bin/${engine}/LemurCGI
cd ..

mkdir tests/${engine}
perl assign2-code/data/runQuery.pl assign2-code/data/cacm/query.cacm tests/${engine}/basic-rocchio http://busjaeg2.projects.cs.illinois.edu/cgi-bin/${engine}/LemurCGI

perl assign2-code/data/ireval.pl -j assign2-code/data/cacm/qrel.cacm < tests/${engine}/basic-rocchio.result  > tests/${engine}/basic-rocchio.eval

tail -n 27 tests/${engine}/basic-rocchio.eval
