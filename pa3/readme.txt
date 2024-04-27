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
perl check_density_target.pl ./benchmark/ibm01/ibm01.nodes ibm01-cu85.gp.pl ./benchmark/ibm01/ibm01-cu85.scl 