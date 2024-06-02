To compile the program, simply type:
make

Please use the following command line to execute the program: 
./fm <input_file_name> <output_file_name>
For example: 
./bin/fm ./input/input_0.dat ./output/output_0.dat
./bin/fm ./input/input_1.dat ./output/output_1.dat
./bin/fm ./input/input_2.dat ./output/output_2.dat
./bin/fm ./input/input_3.dat ./output/output_3.dat
./bin/fm ./input/input_4.dat ./output/output_4.dat
./bin/fm ./input/input_5.dat ./output/output_5.dat

You can get the temporary score from the evaluator by the command below: 
bash evaluator/evaluator.sh <input_file_name> <output_file_name>
For example: 
./evaluator/evaluator.sh ./input/input_0.dat ./output/output_0.dat
./evaluator/evaluator.sh ./input/input_1.dat ./output/output_1.dat
./evaluator/evaluator.sh ./input/input_2.dat ./output/output_2.dat
./evaluator/evaluator.sh ./input/input_3.dat ./output/output_3.dat
./evaluator/evaluator.sh ./input/input_4.dat ./output/output_4.dat
./evaluator/evaluator.sh ./input/input_5.dat ./output/output_5.dat
