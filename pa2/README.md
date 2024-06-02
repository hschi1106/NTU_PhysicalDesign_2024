# Fixed-outline Floorplanning

## Requirements

- g++
- matplotlib for visualization

## Specification

`prog2_floorplanning.pdf`

## File Descriptions

- `src/*`: All c/c++ source files.
- `fp`: The compiled binary program.
- `Makefile`: A Makefile to generate an executable binary

## Compilation

Type `make` in the current directory to generate the binary file `fp` under `bin/` directory

## Usage

Please use the following command line to execute the program:

```bash
./fp [α value] [input.block name] [input.net name] [output file name]
```

For example:

```bash
./bin/fp 0.5 ./input/ami33.block ./input/ami33.nets ./output/ami33.output
```

## Visualization

To see the floorplanning result, use the following command line

```bash
python ./visualizer/plot.py <input.block> <input.nets> <outputFile>
```

For example:

```bash
python ./visualizer/plot.py ./input/ami33.block ./input/ami33.nets ./output/ami33.output
```

## Score Evaluation

You can get the temporary score from the evaluator by the command below:

```bash
./evaluator/evaluator.sh <input.block> <input.net> <outputFile> <alpha>
```

For example:

```bash
./evaluator/evaluator.sh ./input/ami33.block ./input/ami33.nets ./output/ami33.output 0.5
```

![img](https://github.com/hschi1106/NTU_PhysicalDesign_2024/blob/main/pa2/visualizer/png/ami33.png)

*Floorplanning result for case "ami33"*
