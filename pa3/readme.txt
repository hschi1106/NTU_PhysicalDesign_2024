GlobalPlacer.cpp
GlobalPlacer.h
main.cpp -> 應該不用動
ObjectiveFunction.cpp -> implement wirelength and density function
ObjectiveFunction.h -> implement wirelength and density function
Optimizer.cpp
Optimizer.h
Point.h -> 應該不用動
gradient descent

./bin/place -aux ./benchmark/ibm01/ibm01-cu85.aux
./bin/place -aux ./benchmark/ibm05/ibm05.aux
./bin/place -aux ./benchmark/ibm02/ibm02-cu90.aux
./bin/place -aux ./benchmark/ibm07/ibm07-cu90.aux
./bin/place -aux ./benchmark/ibm08/ibm08-cu90.aux
./bin/place -aux ./benchmark/ibm09/ibm09-cu90.aux
perl check_density_target.pl ./benchmark/ibm01/ibm01.nodes ibm01-cu85.gp.pl ./benchmark/ibm01/ibm01-cu85.scl
python plot_ibm01.py ./benchmark/ibm01/ibm01.nodes ./ibm01-cu85.gp.pl
python plot_ibm05.py ./benchmark/ibm05/ibm05.nodes ./ibm05.gp.pl
bash evaluator/evaluator.sh ./benchmark/ibm01/ibm01-cu85.aux