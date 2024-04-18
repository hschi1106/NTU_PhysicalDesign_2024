./fp [α value] [input.block name] [input.net name] [output file name]

./bin/fp 0.5 ./input/sample.block ./input/sample.nets ./output/sample.output
./bin/fp 0.5 ./input/ami33.block ./input/ami33.nets ./output/ami33.output
./bin/fp 0.5 ./input/ami49.block ./input/ami49.nets ./output/ami49.output
./bin/fp 0.5 ./input/apte.block ./input/apte.nets ./output/apte.output
./bin/fp 0.5 ./input/hp.block ./input/hp.nets ./output/hp.output
./bin/fp 0.5 ./input/xerox.block ./input/xerox.nets ./output/xerox.output

./evaluator/evaluator.sh ./input/sample.block ./input/sample.nets ./output/sample.output 0.5
./evaluator/evaluator.sh ./input/ami33.block ./input/ami33.nets ./output/ami33.output 0.5
./evaluator/evaluator.sh ./input/ami49.block ./input/ami49.nets ./output/ami49.output 0.5
./evaluator/evaluator.sh ./input/apte.block ./input/apte.nets ./output/apte.output 0.5
./evaluator/evaluator.sh ./input/hp.block ./input/hp.nets ./output/hp.output 0.5
./evaluator/evaluator.sh ./input/xerox.block ./input/xerox.nets ./output/xerox.output 0.5

./bin/fp 0.25 ./input/apte.block ./input/apte.nets ./output/apte.output
./evaluator/evaluator.sh ./input/apte.block ./input/apte.nets ./output/apte.output 0.25

./bin/fp 0.5 ./input/apte.block ./input/apte.nets ./output/apte.output
./evaluator/evaluator.sh ./input/apte.block ./input/apte.nets ./output/apte.output 0.5

./bin/fp 0.75 ./input/apte.block ./input/apte.nets ./output/apte.output
./evaluator/evaluator.sh ./input/apte.block ./input/apte.nets ./output/apte.output 0.75


./bin/fp 0.25 ./input/xerox.block ./input/xerox.nets ./output/xerox.output
./evaluator/evaluator.sh ./input/xerox.block ./input/xerox.nets ./output/xerox.output 0.25

./bin/fp 0.5 ./input/xerox.block ./input/xerox.nets ./output/xerox.output
./evaluator/evaluator.sh ./input/xerox.block ./input/xerox.nets ./output/xerox.output 0.5

./bin/fp 0.75 ./input/xerox.block ./input/xerox.nets ./output/xerox.output
./evaluator/evaluator.sh ./input/xerox.block ./input/xerox.nets ./output/xerox.output 0.75


python plot.py ./input/ami33.block ./input/ami33.nets ./output/ami33.output
python plot.py ./input/ami49.block ./input/ami49.nets ./output/ami49.output
python plot.py ./input/apte.block ./input/apte.nets ./output/apte.output
python plot.py ./input/hp.block ./input/hp.nets ./output/hp.output
python plot.py ./input/xerox.block ./input/xerox.nets ./output/xerox.output