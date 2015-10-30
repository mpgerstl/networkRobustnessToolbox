make: src/failureProbabilityByMcs.c src/recalcFailureProbability.c src/robustnessByEfms.c src/overallRobustnessByEfms.c
	gcc -o bin/failureProbabilityByMcs src/failureProbabilityByMcs.c -lm -pthread -Wall -O3
	gcc -o bin/recalcFailureProbability src/recalcFailureProbability.c -lm -Wall -O3
	gcc -o bin/robustnessByEfms src/robustnessByEfms.c -lm -Wall -O3
	gcc -o bin/overallRobustnessByEfms src/overallRobustnessByEfms.c -lm -Wall -O3
