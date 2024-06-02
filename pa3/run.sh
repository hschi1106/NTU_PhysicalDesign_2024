#!/bin/bash

# Run commands and save output to out.log
./bin/place -aux ./benchmark/ibm01/ibm01-cu85.aux > out.log
./bin/place -aux ./benchmark/ibm05/ibm05.aux >> out.log
./bin/place -aux ./benchmark/ibm02/ibm02-cu90.aux >> out.log
./bin/place -aux ./benchmark/ibm07/ibm07-cu90.aux >> out.log
./bin/place -aux ./benchmark/ibm08/ibm08-cu90.aux >> out.log
./bin/place -aux ./benchmark/ibm09/ibm09-cu90.aux >> out.log
