#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include "floorplanner.h"
using namespace std;

int main(int argc, char **argv)
{
  double alpha;
  fstream inputBlock, inputNet, output;

  if (argc == 5)
  {
    alpha = atof(argv[1]);
    inputBlock.open(argv[2], ios::in);
    inputNet.open(argv[3], ios::in);
    output.open(argv[4], ios::out);
    if (!argv[0]) {
      cerr << "Lost alpha constant \"" << argv[0]
           << "\". The program will be terminated..." << endl;
      exit(1);
    }
    if (!inputBlock)
    {
      cerr << "Cannot open the input block file \"" << argv[1]
           << "\". The program will be terminated..." << endl;
      exit(1);
    }
    if (!inputNet)
    {
      cerr << "Cannot open the input net file \"" << argv[1]
           << "\". The program will be terminated..." << endl;
      exit(1);
    }
    if (!output)
    {
      cerr << "Cannot open the output file \"" << argv[2]
           << "\". The program will be terminated..." << endl;
      exit(1);
    }
  }
  else
  {
    cerr << "Usage: ./fp [alpha value] [input.block name] [input.net name] [output file name]" << endl;
    exit(1);
  }

  Floorplanner *fp = new Floorplanner(alpha, inputBlock, inputNet);
  // fp->reportModule();
  fp->floorplan();
  // fp->reportBStarTree(fp->getBStarTreeRoot());
  fp->printSummary();
  fp->writeResult(output);
  cout << "total runtime: " << (double)clock() / CLOCKS_PER_SEC << " seconds" << endl;

  return 0;
}
