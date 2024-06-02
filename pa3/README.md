
To compile the program, simply type:
make

Please use the following command line to execute the program: 
./place -aux <inputFile.aux>
For example: 
./bin/place -aux ./benchmark/ibm01/ibm01-cu85.aux
./bin/place -aux ./benchmark/ibm05/ibm05.aux

The “scaled overflow per bin” can be found by using the following script: 
perl check_density_target.pl <input.nodes> <Solution .pl file> <input.scl> 
For example: 
perl check_density_target.pl ibm01.nodes ibm01-cu85.gp.pl ibm01-cu85.scl
perl check_density_target.pl ibm05.nodes ibm05.gp.pl ibm05.scl

To see the placement result, use the following command line
python ./visualizer/plot.py <input.nodes> <Solution .pl file>
For example: 
python ./visualizer/plot.py ./benchmark/ibm01/ibm01.nodes ./ibm01-cu85.gp.pl
python ./visualizer/plot.py ./benchmark/ibm05/ibm05.nodes ./ibm05.gp.pl

You can get your temporary score from the evaluator by the command below: 
bash evaluator/evaluator <inputFile.aux> <HPWL> <Time (s)> 
For example: 
bash evaluator/evaluator.sh ./benchmark/ibm01/ibm01-cu85.aux 57262590 98
