#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cassert>
#include <vector>
#include <cmath>
#include <unordered_map>
#include <map>
#include <algorithm>
#include "module.h"
#include "Bstar.h"
#include "floorplanner.h"
using namespace std;

void Floorplanner::parseInput(fstream &blockInFile, fstream &netInFile)
{
  cout << "Parsing input..." << endl;
  
  // input block file
  blockInFile >> _outlineWidth >> _outlineHeight;
  blockInFile >> _blockNum >> _terminalNum;

  // input block information
  for (int i = 0;i<_blockNum;++i)
  {
    string name;
    size_t width, height;
    blockInFile >> name >> width >> height;
    Block *block = new Block(name, width, height);
    _blockArray.push_back(block);
    _totalArea += block->getArea();
    _blockName2Id[name] = i;
  }

  // input terminal information
  for (int i = 0;i<_terminalNum;++i)
  {
    string name;
    size_t x, y;
    blockInFile >> name >> x >> y;
    Terminal *terminal = new Terminal(name, x, y);
    _terminalArray.push_back(terminal);
    _terminalName2Id[name] = i;
  }

  // input net file
  netInFile >> _netNum;
  for (int i = 0;i<_netNum;++i)
  {
    int netDegree;
    netInFile >> netDegree;
    Net *net = new Net();
    for (int j = 0;j<netDegree;++j)
    {
      string terminalName;
      netInFile >> terminalName;
      if (_terminalName2Id.find(terminalName) != _terminalName2Id.end())
      {
        net->addTerminal(_terminalArray[_terminalName2Id[terminalName]]);
      }
      else if (_blockName2Id.find(terminalName) != _blockName2Id.end())
      {
        net->addTerminal(_blockArray[_blockName2Id[terminalName]]);
      }
      else
      {
        cerr << "Cannot find the terminal or block name \"" << terminalName
             << "\". The program will be terminated..." << endl;
        exit(1);
      }
    }
    _netArray.push_back(net);
  }

  return;
}
