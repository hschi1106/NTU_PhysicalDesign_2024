To compile the program, simply type:
make

Please use the following command line to execute the program: 
./fp [α value] [input.block name] [input.net name] [output file name]
For example: 
./bin/fp 0.5 ./input/ami33.block ./input/ami33.nets ./output/ami33.output
./bin/fp 0.5 ./input/ami49.block ./input/ami49.nets ./output/ami49.output
./bin/fp 0.5 ./input/apte.block ./input/apte.nets ./output/apte.output
./bin/fp 0.5 ./input/hp.block ./input/hp.nets ./output/hp.output
./bin/fp 0.5 ./input/xerox.block ./input/xerox.nets ./output/xerox.output

You can get the temporary score from the evaluator by the command below: 
bash evaluator/evaluator.sh <input.block> <input.net> <outputFile> <alpha>
For example: 
./evaluator/evaluator.sh ./input/ami33.block ./input/ami33.nets ./output/ami33.output 0.5
./evaluator/evaluator.sh ./input/ami49.block ./input/ami49.nets ./output/ami49.output 0.5
./evaluator/evaluator.sh ./input/apte.block ./input/apte.nets ./output/apte.output 0.5
./evaluator/evaluator.sh ./input/hp.block ./input/hp.nets ./output/hp.output 0.5
./evaluator/evaluator.sh ./input/xerox.block ./input/xerox.nets ./output/xerox.output 0.5

To see the floorplanning result, use the following command line
python ./visualizer/plot.py <input.block> <input.nets> <outputFile>
For example: 
python ./visualizer/plot.py ./input/ami33.block ./input/ami33.nets ./output/ami33.output
python ./visualizer/plot.py ./input/ami49.block ./input/ami49.nets ./output/ami49.output
python ./visualizer/plot.py ./input/apte.block ./input/apte.nets ./output/apte.output
python ./visualizer/plot.py ./input/hp.block ./input/hp.nets ./output/hp.output
python ./visualizer/plot.py ./input/xerox.block ./input/xerox.nets ./output/xerox.output