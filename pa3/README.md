# Global Placement

## Requirements

- g++
- matplotlib for visualization

## Specification

`prog3_placement.pdf`

## File Descriptions

- `src/*`: All c/c++ source files.
- `place`: The compiled binary program.
- `Makefile`: A Makefile to generate an executable binary

## Compilation

Type `make` in the current directory to generate the binary file `place` under `bin/` directory

## Usage

Please use the following command line to execute the program:

```bash
./place -aux <inputFile.aux>
```

For example:

```bash
./bin/place -aux ./benchmark/ibm01/ibm01-cu85.aux
```

## Visualization

To see the placement result, use the following command line

```bash
python ./visualizer/plot.py <input.nodes> <Solution .pl file>
```

For example:

```bash
python ./visualizer/plot.py ./benchmark/ibm01/ibm01.nodes ./output/ibm01/ibm01-cu85.gp.pl
```

## Overflow Evaluation

The “scaled overflow per bin” can be found by using the following script:

```bash
perl check_density_target.pl <input.nodes> <Solution .pl file> <input.scl>
```

For example:

```bash
perl check_density_target.pl ibm01.nodes ibm01-cu85.gp.pl ibm01-cu85.scl
```

perl check_density_target.pl ibm05.nodes ibm05.gp.pl ibm05.scl

## Score Evaluation

You can get the temporary score from the evaluator by the command below:

```bash
./evaluator/evaluator.sh <inputFile.aux> <HPWL> <Time (s)>
```

For example:

```bash
./evaluator/evaluator.sh ./benchmark/ibm01/ibm01-cu85.aux 57262590 98
```

## Result

*Global placement result for case "ibm01"*
![img](https://github.com/hschi1106/NTU_PhysicalDesign_2024/blob/main/pa3/visualizer/png/ibm01.gp.png)
