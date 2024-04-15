echo "alpha = 0.25" > out.log

./bin/fp 0.25 ./input/ami33.block ./input/ami33.nets ./output/ami33.output >> out.log
./bin/fp 0.25 ./input/ami49.block ./input/ami49.nets ./output/ami49.output >> out.log
./bin/fp 0.25 ./input/apte.block ./input/apte.nets ./output/apte.output >> out.log
./bin/fp 0.25 ./input/hp.block ./input/hp.nets ./output/hp.output >> out.log
./bin/fp 0.25 ./input/xerox.block ./input/xerox.nets ./output/xerox.output >> out.log

./evaluator/evaluator.sh ./input/ami33.block ./input/ami33.nets ./output/ami33.output 0.25 >> out.log
./evaluator/evaluator.sh ./input/ami49.block ./input/ami49.nets ./output/ami49.output 0.25 >> out.log
./evaluator/evaluator.sh ./input/apte.block ./input/apte.nets ./output/apte.output 0.25 >> out.log
./evaluator/evaluator.sh ./input/hp.block ./input/hp.nets ./output/hp.output 0.25 >> out.log
./evaluator/evaluator.sh ./input/xerox.block ./input/xerox.nets ./output/xerox.output 0.25 >> out.log


echo "alpha = 0.5" >> out.log

./bin/fp 0.5 ./input/ami33.block ./input/ami33.nets ./output/ami33.output >> out.log
./bin/fp 0.5 ./input/ami49.block ./input/ami49.nets ./output/ami49.output >> out.log
./bin/fp 0.5 ./input/apte.block ./input/apte.nets ./output/apte.output >> out.log
./bin/fp 0.5 ./input/hp.block ./input/hp.nets ./output/hp.output >> out.log
./bin/fp 0.5 ./input/xerox.block ./input/xerox.nets ./output/xerox.output >> out.log

./evaluator/evaluator.sh ./input/ami33.block ./input/ami33.nets ./output/ami33.output 0.5 >> out.log
./evaluator/evaluator.sh ./input/ami49.block ./input/ami49.nets ./output/ami49.output 0.5 >> out.log
./evaluator/evaluator.sh ./input/apte.block ./input/apte.nets ./output/apte.output 0.5 >> out.log
./evaluator/evaluator.sh ./input/hp.block ./input/hp.nets ./output/hp.output 0.5 >> out.log
./evaluator/evaluator.sh ./input/xerox.block ./input/xerox.nets ./output/xerox.output 0.5 >> out.log


echo "alpha = 0.75" >> out.log

./bin/fp 0.75 ./input/ami33.block ./input/ami33.nets ./output/ami33.output >> out.log
./bin/fp 0.75 ./input/ami49.block ./input/ami49.nets ./output/ami49.output >> out.log
./bin/fp 0.75 ./input/apte.block ./input/apte.nets ./output/apte.output >> out.log
./bin/fp 0.75 ./input/hp.block ./input/hp.nets ./output/hp.output >> out.log
./bin/fp 0.75 ./input/xerox.block ./input/xerox.nets ./output/xerox.output >> out.log

./evaluator/evaluator.sh ./input/ami33.block ./input/ami33.nets ./output/ami33.output 0.75 >> out.log
./evaluator/evaluator.sh ./input/ami49.block ./input/ami49.nets ./output/ami49.output 0.75 >> out.log
./evaluator/evaluator.sh ./input/apte.block ./input/apte.nets ./output/apte.output 0.75 >> out.log
./evaluator/evaluator.sh ./input/hp.block ./input/hp.nets ./output/hp.output 0.75 >> out.log
./evaluator/evaluator.sh ./input/xerox.block ./input/xerox.nets ./output/xerox.output 0.75 >> out.log