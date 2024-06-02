# 2-Way F-M Circuit Partitioning

## Requirements

- g++

## Specification

`prog1_partitioning.pdf`

## File Descriptions

- `src/*`: All c/c++ source files.
- `fm`: The compiled binary program.
- `Makefile`: A Makefile to generate an executable binary

## Compilation

Type `make` in the current directory to generate the binary file `fm` under `bin/` directory

## Usage

Please use the following command line to execute the program:

```bash
./fm <input_file_name> <output_file_name>
```

For example:

```bash
./bin/fm ./input/input_0.dat ./output/output_0.dat
```

## Score Evaluation

You can get the temporary score from the evaluator by the command below:

```bash
./evaluator/evaluator.sh <input_file_name> <output_file_name>
```

For example:

```bash
./evaluator/evaluator.sh ./input/input_0.dat ./output/output_0.dat
```
