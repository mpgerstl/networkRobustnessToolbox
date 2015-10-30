#!/bin/bash

echo "===================================================================="
 
echo "failureProbabilityByMcs:"
echo "  calculate failure probability using max deletions = 4, threads = 8"
echo "  and lambda = 0.15 and save output to exp.fp.out"
read -n 1 -p "(press any key)"

../bin/failureProbabilityByMcs -i example.cutsets -l 0.15 -m 4 -t 8 -o exp.fp.out

echo "--------------------------------------------------------------------"

echo "recalcFailureProbability:"
echo "  recalculate failure probability with lambda = 0.5 and save output"
echo "  to exp.refp.out"
read -n 1 -p "(press any key)"

../bin/recalcFailureProbability -i exp.fp.out -l 0.5 > exp.refp.out

echo "--------------------------------------------------------------------"

echo "convertFailureProbOut2csv.pl:"
echo "  convert failure probability output of exp.fp.out to csv format"
read -n 1 -p "(press any key)"

../scripts/convertFailureProbOut2csv.pl -i exp.fp.out

echo "--------------------------------------------------------------------"

echo "robustnessByEfms:"
echo "  calculate robustness and save output to robustness.out"
read -n 1 -p "(press any key)"

../bin/robustnessByEfms -i rob.efms > robustness.out

echo "--------------------------------------------------------------------"

echo "overallRobustnessByEfms:"
echo "  calculate overall robustness and save output to overall.rob.out"
read -n 1 -p "(press any key)"

../bin/overallRobustnessByEfms -i rob.efms > overall.rob.out

echo "===================================================================="
