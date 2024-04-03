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
  // input block file
  string str;
  blockInFile >> str;
  assert(str == "Outline:");
  blockInFile >> _outlineWidth >> _outlineHeight;
  blockInFile >> str;
  assert(str == "NumBlocks:");
  blockInFile >> _blockNum;
  blockInFile >> str;
  assert(str == "NumTerminals:");
  blockInFile >> _terminalNum;

  // input block information
  for (int i = 0; i < _blockNum; ++i)
  {
    string name;
    size_t width, height;
    blockInFile >> name >> width >> height;
    Block *block = new Block(name, width, height);
    _blockArray.push_back(block);
    _totalArea += block->getArea(); // might be changed later
    _blockName2Id[name] = i;
  }

  // input terminal information
  for (int i = 0; i < _terminalNum; ++i)
  {
    string name, str;
    size_t x, y;
    blockInFile >> name >> str;
    assert(str == "terminal");
    blockInFile >> x >> y;
    Terminal *terminal = new Terminal(name, x, y);
    _terminalArray.push_back(terminal);
    _terminalName2Id[name] = i;
  }

  // input net file
  netInFile >> str;
  assert(str == "NumNets:");
  netInFile >> _netNum;

  for (int i = 0; i < _netNum; ++i)
  {
    netInFile >> str;
    assert(str == "NetDegree:");
    int netDegree;
    netInFile >> netDegree;
    Net *net = new Net();
    for (int j = 0; j < netDegree; ++j)
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

void Floorplanner::compact()
{
  cout << "Compacting..." << endl;

  return;
}

void Floorplanner::floorplan()
{
  cout << "Floorplanning..." << endl;

  return;
}

void Floorplanner::reportModule() const
{
  cout << "Report module information..." << endl;
  cout << "Outline width: " << _outlineWidth << endl;
  cout << "Outline height: " << _outlineHeight << endl;

  // print the block information
  cout << "Number of blocks: " << _blockNum << endl;
  for (int i = 0; i < _blockNum; ++i)
  {
    cout << "Block " << i << ": " << _blockArray[i]->getName() << " " << _blockArray[i]->getWidth() << " " << _blockArray[i]->getHeight() << endl;
  }

  // print the terminal information
  cout << "Number of terminals: " << _terminalNum << endl;
  for (int i = 0; i < _terminalNum; ++i)
  {
    cout << "Terminal " << i << ": " << _terminalArray[i]->getName() << " " << _terminalArray[i]->getX1() << " " << _terminalArray[i]->getX2() << " " << _terminalArray[i]->getY1() << " " << _terminalArray[i]->getY2() << endl;
  }

  // print the net information
  cout << "Number of nets: " << _netNum << endl;
  for (int i = 0; i < _netNum; ++i)
  {
    cout << "Net " << i << ": ";
    vector<Terminal *> terminalList = _netArray[i]->getTermList();
    for (size_t j = 0; j < terminalList.size(); ++j)
    {
      cout << terminalList[j]->getName() << " ";
    }
    cout << endl;
  }

  // print the total area
  cout << "Total area: " << _totalArea << endl;

  return;
}

void Floorplanner::printSummary() const
{
  // print the summary information
  cout << endl;
  cout << "==================== Summary ====================" << endl;
  cout << " Total block number: " << _blockNum << endl;
  cout << " Total terminal number: " << _terminalNum << endl;
  cout << " Total net number:  " << _netNum << endl;
  cout << " Final cost: " << _finalCost << endl;
  cout << " Total wirelength: " << _totalWirelength << endl;
  cout << " Chip area: " << _chipWidth * _chipHeight << endl;
  cout << " Chip width: " << _chipWidth << endl;
  cout << " Chip height: " << _chipHeight << endl;
  cout << " Total runtime: " << _totalRuntime << endl;
  cout << "=================================================" << endl;
  cout << endl;
  return;
}

void Floorplanner::writeResult(fstream &outFile)
{
  stringstream buff;
  // write the summary information
  buff << _finalCost;
  outFile << buff.str() << endl;
  buff.str("");
  buff << _totalWirelength;
  outFile << buff.str() << endl;
  buff.str("");
  buff << _chipWidth * _chipHeight;
  outFile << buff.str() << endl;
  buff.str("");
  buff << _chipWidth << " " << _chipHeight;
  outFile << buff.str() << endl;
  buff.str("");
  buff << _totalRuntime;
  outFile << buff.str() << endl;
  buff.str("");

  // write the block information
  for (int i = 0; i < _blockNum; ++i)
  {
    buff << _blockArray[i]->getName() << " " << _blockArray[i]->getX1() << " " << _blockArray[i]->getY1() << " " << _blockArray[i]->getX2() << " " << _blockArray[i]->getY2() << endl;
    outFile << buff.str();
  }

  return;
}