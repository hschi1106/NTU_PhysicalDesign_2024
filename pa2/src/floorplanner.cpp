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
  }

  // sort _blockArray by area
  sort(_blockArray.begin(), _blockArray.end(), [](Block *a, Block *b)
       { return a->getArea() > b->getArea(); });

  for (int i = 0; i < _blockNum; ++i)
  {
    _blockName2Id[_blockArray[i]->getName()] = i;
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

void Floorplanner::createBStarTree()
{
  for (int i = 0; i < _blockNum; ++i)
  {
    TreeNode *node = new TreeNode(_blockArray[i]);
    _blockName2TreeNode[_blockArray[i]->getName()] = node;
  }

  _bStarTreeRoot = _blockName2TreeNode[_blockArray[0]->getName()];

  for (int i = 0; i < _blockNum; ++i)
  {
    TreeNode *currNode = _blockName2TreeNode[_blockArray[i]->getName()];
    if (2 * i + 1 < _blockNum)
    {
      string leftName = _blockArray[2 * i + 1]->getName();
      TreeNode *leftNode = _blockName2TreeNode[leftName];
      currNode->setLeft(leftNode);
      leftNode->setParent(currNode);
    }
    if (2 * i + 2 < _blockNum)
    {
      string rightName = _blockArray[2 * i + 2]->getName();
      TreeNode *rightNode = _blockName2TreeNode[rightName];
      currNode->setRight(rightNode);
      rightNode->setParent(currNode);
    }
  }

  return;
}

size_t Floorplanner::calculateTreeY(TreeNode *currNode)
{
  // basic information of the current node
  size_t blockWidth = currNode->getBlock()->getWidth(), blockHeight = currNode->getBlock()->getHeight();
  size_t placeX = currNode->getX();
  map<size_t, size_t>::iterator it = _heightMap.begin();
  while (it->first < placeX)
  {
    it++;
  }
  // now: it->first == placeX

  // find the y coordinate
  size_t prevY = it->second; // the previous y coordinate before placeX
  size_t highestY = prevY;   // the highest y coordinate between placeX and placeX + blockWidth
  while (it != _heightMap.end() && it->first < placeX + blockWidth)
  {
    // update the highest y coordinate
    if (it->second > highestY)
    {
      highestY = it->second;
    }

    // update the previous y coordinate before iterator moves to the next position
    prevY = it->second;

    // iterator moves to the next position
    it++;

    // remove the redundant height map between placeX and placeX + blockWidth
    if (it != _heightMap.begin() && prev(it)->first > placeX && prev(it)->first < placeX + blockWidth)
    {
      _heightMap.erase(prev(it));
    }
  }

  // update the height map
  _heightMap[placeX] = highestY + blockHeight;
  _heightMap[placeX + blockWidth] = _heightMap.find(placeX + blockWidth) == _heightMap.end() ? prevY : _heightMap[placeX + blockWidth];

  return highestY;
}

void Floorplanner::calculateTreePosition(TreeNode *currNode)
{
  // if the current node is nullptr, return
  if (currNode == nullptr)
  {
    return;
  }

  // if the current node is the root of the B*-tree, clear the height map
  if (currNode == _bStarTreeRoot)
  {
    _heightMap.clear();
  }

  // basic information of the current node
  size_t blockWidth = currNode->getBlock()->getWidth(), blockHeight = currNode->getBlock()->getHeight();
  size_t placeX = currNode->getX(), placeY = currNode->getY();

  if (_heightMap.empty())
  {
    _heightMap[placeX] = blockHeight;
    _heightMap[placeX + blockWidth] = placeY;
  }

  if (currNode->getLeft() != nullptr)
  {
    // calculate the x position
    currNode->getLeft()->setX(placeX + blockWidth);
    currNode->getLeft()->setY(calculateTreeY(currNode->getLeft()));
    calculateTreePosition(currNode->getLeft());
  }

  if (currNode->getRight() != nullptr)
  {
    // calculate the x position
    currNode->getRight()->setX(placeX);
    currNode->getRight()->setY(calculateTreeY(currNode->getRight()));
    calculateTreePosition(currNode->getRight());
  }

  return;
}

void Floorplanner::clearTreePosition(TreeNode *currNode)
{
  if (currNode == nullptr)
  {
    return;
  }

  // clear the x and y position for next iteration
  currNode->setX(0);
  currNode->setY(0);

  clearTreePosition(currNode->getLeft());
  clearTreePosition(currNode->getRight());

  return;
}

size_t Floorplanner::calculateTreeChipWidth()
{
  // calculate the chip width by height map
  size_t chipWidth = 0;
  for (map<size_t, size_t>::iterator it = _heightMap.begin(); it != _heightMap.end(); ++it)
  {
    chipWidth = max(chipWidth, it->first);
  }
  return chipWidth;
}

size_t Floorplanner::calculateTreeChipHeight()
{
  // calculate the chip height by height map
  size_t chipHeight = 0;
  for (map<size_t, size_t>::iterator it = _heightMap.begin(); it != _heightMap.end(); ++it)
  {
    chipHeight = max(chipHeight, it->second);
  }
  return chipHeight;
}

double Floorplanner::calculateTreeWirelength()
{
  // calculate the wirelength
  double wirelength = 0;
  for (int i = 0; i < _netNum; ++i)
  {
    vector<Terminal *> terminalList = _netArray[i]->getTermList();
    double minX = _outlineWidth, minY = _outlineHeight, maxX = 0, maxY = 0;
    for (int j = 0; j < terminalList.size(); ++j)
    {
      double midX, midY;
      if (_blockName2Id.count(terminalList[j]->getName()))
      {
        TreeNode *node = _blockName2TreeNode[terminalList[j]->getName()];
        midX = node->getX() + double(node->getBlock()->getWidth() / 2.0);
        midY = node->getY() + double(node->getBlock()->getHeight() / 2.0);
      }
      else
      {
        midX = terminalList[j]->getX1();
        midY = terminalList[j]->getY1();
      }
      minX = min(minX, midX);
      minY = min(minY, midY);
      maxX = max(maxX, midX);
      maxY = max(maxY, midY);
    }
    wirelength += (maxX - minX) + (maxY - minY);
  }

  return wirelength;
}

double Floorplanner::calculateTreeCost()
{
  // clear the position
  this->clearTreePosition(_bStarTreeRoot);
  this->calculateTreePosition(_bStarTreeRoot);

  // calculate the chip width and chip height
  size_t chipWidth = this->calculateTreeChipWidth();
  size_t chipHeight = this->calculateTreeChipHeight();

  // calculate the wirelength
  double wirelength = this->calculateTreeWirelength();

  // calculate cost
  // double cost = _alpha * (chipWidth * chipHeight) / _averageArea + (1 - _alpha) * wirelength / _averageWirelength;
  double cost = _alpha * (chipWidth * chipHeight) + (1 - _alpha) * wirelength;

  return cost;
}

void Floorplanner::randomlyRotateBlock()
{
  // randomly pick a block
  int blockId = rand() % _blockNum;
  TreeNode *rotatedNode = _blockName2TreeNode[_blockArray[blockId]->getName()];

  // rotate the block
  Block *rotatedBlock = rotatedNode->getBlock();
  rotatedBlock->rotateBolock();

  // record resored data
  rotatedNode->setLastRotated(true);
  _modifiedNodes.push_back(rotatedNode);

  return;
}

void Floorplanner::randomlySwapNodes()
{
  // randomly pick two blocks
  int blockIdA = rand() % _blockNum;
  int blockIdB = rand() % _blockNum;

  // make sure two blocks are different
  while (blockIdA == blockIdB)
  {
    blockIdB = rand() % _blockNum;
  }
  TreeNode *swapedNodeA = _blockName2TreeNode[_blockArray[blockIdA]->getName()];
  TreeNode *swapedNodeB = _blockName2TreeNode[_blockArray[blockIdB]->getName()];

  // swap two nodes
  Block *blockA = swapedNodeA->getBlock();
  Block *blockB = swapedNodeB->getBlock();

  // record the swapped block
  swapedNodeA->setLastBlock(blockA);
  swapedNodeB->setLastBlock(blockB);
  _modifiedNodes.push_back(swapedNodeA);
  _modifiedNodes.push_back(swapedNodeB);

  swapedNodeA->setBlock(blockB);
  swapedNodeB->setBlock(blockA);

  // update the blockName2TreeNode
  _blockName2TreeNode[blockA->getName()] = swapedNodeB;
  _blockName2TreeNode[blockB->getName()] = swapedNodeA;

  return;
}

void Floorplanner::randomlyMove()
{
  int blockId = rand() % _blockNum;
  TreeNode *movedNode = _blockName2TreeNode[_blockArray[blockId]->getName()];
  while (movedNode == _bStarTreeRoot)
  {
    blockId = rand() % _blockNum;
    movedNode = _blockName2TreeNode[_blockArray[blockId]->getName()];
  }

  movedNode->setOldParent(movedNode->getParent());
  if (movedNode->getParent()->getLeft() == movedNode)
  {
    movedNode->getParent()->setLeft(nullptr);
    movedNode->getParent()->setOldLeft(movedNode);
  }
  else
  {
    movedNode->getParent()->setRight(nullptr);
    movedNode->getParent()->setOldRight(movedNode);
  }
  _modifiedNodes.push_back(movedNode);
  _modifiedNodes.push_back(movedNode->getParent());

  // pick a random leaf
  TreeNode *leafNode = _bStarTreeRoot;
  while (leafNode->getLeft() != nullptr || leafNode->getRight() != nullptr)
  {
    if (rand() % 2 == 0)
    {
      if (leafNode->getLeft() != nullptr)
      {
        leafNode = leafNode->getLeft();
      }
      else
      {
        leafNode = leafNode->getRight();
      }
    }
    else
    {
      if (leafNode->getRight() != nullptr)
      {
        leafNode = leafNode->getRight();
      }
      else
      {
        leafNode = leafNode->getLeft();
      }
    }
  }

  if (rand() % 2 == 0)
  {
    leafNode->setLeft(movedNode);
    leafNode->setOldLeft(_lastIsNull);
    movedNode->setParent(leafNode);
  }
  else
  {
    leafNode->setRight(movedNode);
    leafNode->setOldRight(_lastIsNull);
    movedNode->setParent(leafNode);
  }
  _modifiedNodes.push_back(leafNode);

  return;
}

void Floorplanner::perturb()
{
  int operation = rand() % 3; // 0: rotate, 1: swap, 2: move
  if (operation == 0)
  {
    // rotate a block
    this->randomlyRotateBlock();
  }
  else if (operation == 1)
  {
    // swap two nodes
    this->randomlySwapNodes();
  }
  else
  {
    // move a block
    this->randomlyMove();
  }
  return;
}

void Floorplanner::deleteLastStatus()
{
  // clean the last status
  for (int i = 0; i < _modifiedNodes.size(); ++i)
  {
    TreeNode *currNode = _modifiedNodes[i];
    currNode->setOldLeft(nullptr);
    currNode->setOldRight(nullptr);
    currNode->setOldParent(nullptr);
    currNode->setLastRotated(false);
    currNode->setLastBlock(nullptr);
  }
  _modifiedNodes.clear();

  return;
}

void Floorplanner::restoreLastStatus()
{
  // restore the last status
  for (int i = 0; i < _modifiedNodes.size(); ++i)
  {
    TreeNode *currNode = _modifiedNodes[i];

    // restore left
    if (currNode->getOldLeft() == _lastIsNull)
    {
      currNode->setLeft(nullptr);
      currNode->setOldLeft(nullptr);
    }
    else if (currNode->getOldLeft() != nullptr)
    {
      currNode->setLeft(currNode->getOldLeft());
      currNode->setOldLeft(nullptr);
    }

    // restore right
    if (currNode->getOldRight() == _lastIsNull)
    {
      currNode->setRight(nullptr);
      currNode->setOldRight(nullptr);
    }
    else if (currNode->getOldRight() != nullptr)
    {
      currNode->setRight(currNode->getOldRight());
      currNode->setOldRight(nullptr);
    }

    // restore parent
    if (currNode->getOldParent() == _lastIsNull)
    {
      currNode->setParent(nullptr);
      currNode->setOldParent(nullptr);
    }
    else if (currNode->getOldParent() != nullptr)
    {
      currNode->setParent(currNode->getOldParent());
      currNode->setOldParent(nullptr);
    }

    // restore rotated block
    if (currNode->getLastRotated())
    {
      currNode->getBlock()->rotateBolock();
      currNode->setLastRotated(false);
    }

    // restore swapped block
    if (currNode->getLastBlock() != nullptr)
    {
      _blockName2TreeNode[currNode->getLastBlock()->getName()] = currNode;
      currNode->setBlock(currNode->getLastBlock());
      currNode->setLastBlock(nullptr);
    }
  }

  // clear the modified nodes
  _modifiedNodes.clear();

  return;
}

void Floorplanner::writeBestCoordinate(TreeNode *currNode)
{
  if (currNode == nullptr)
  {
    return;
  }

  // write the best coordinate to blocks
  currNode->getBlock()->setPos(currNode->getX(), currNode->getY(), currNode->getX() + currNode->getBlock()->getWidth(), currNode->getY() + currNode->getBlock()->getHeight());

  writeBestCoordinate(currNode->getLeft());
  writeBestCoordinate(currNode->getRight());

  return;
}

void Floorplanner::calculateAverage()
{
  _averageArea = 0;
  _averageWirelength = 0;
  for (int i = 0; i < _blockNum; ++i)
  {
    this->clearTreePosition(_bStarTreeRoot);
    this->perturb();
    this->calculateTreePosition(_bStarTreeRoot);
    size_t chipWidth = this->calculateTreeChipWidth();
    size_t chipHeight = this->calculateTreeChipHeight();
    double wirelength = this->calculateTreeWirelength();
    this->clearTreePosition(_bStarTreeRoot);
    this->deleteLastStatus();
    _averageArea += chipWidth * chipHeight;
    _averageWirelength += wirelength;
  }
  _averageArea /= _blockNum;
  _averageWirelength /= _blockNum;
  cout << "average area: " << _averageArea << " average wirelength: " << _averageWirelength << endl;
  return;
}

void Floorplanner::SA(double initTemp, double coolingRate, double stopTemp)
{
  // Simulated Annealing starts
  for (double T = initTemp; T > stopTemp; T *= coolingRate)
  {
    int iterNum;
    if (T > 100)
    {
      iterNum = 500;
    }
    else if (T > 30)
    {
      iterNum = 1000;
    }
    else
    {
      iterNum = 500;
    }
    for (int i = 0; i < iterNum; ++i)
    {
      double oldCost = this->calculateTreeCost();
      this->perturb();

      double newCost = this->calculateTreeCost();
      double prob = (double)rand() / (RAND_MAX);
      // size_t chipWidth = this->calculateTreeChipWidth();
      // size_t chipHeight = this->calculateTreeChipHeight();
      // bool isLegal = chipWidth <= _outlineWidth && chipHeight <= _outlineHeight;
      if (newCost < oldCost)
      {
        // accept and update best
        this->writeBestCoordinate(_bStarTreeRoot);
        this->deleteLastStatus();
      }
      else if (1 - prob < exp((oldCost - newCost) / T))
      {
        // accept but not update best
        this->deleteLastStatus();
      }
      else
      {
        // restore last status
        this->restoreLastStatus();
      }
    }
  }
}

void Floorplanner::floorplan()
{
  clock_t start = clock();
  _chipHeight = _outlineHeight + 1;
  _chipWidth = _outlineWidth + 1;

  // this->calculateTreeCost();
  // this->writeBestCoordinate(_bStarTreeRoot);

  while (_chipHeight > _outlineHeight || _chipWidth > _outlineWidth || (_chipHeight == 0 && _chipWidth == 0))
  {
    srand(time(NULL));
    this->createBStarTree();
    // this->calculateAverage();
    this->SA(1000, 0.9, 1);
    this->calculateOutput();
    cout << "chip width: " << _chipWidth << " chip height: " << _chipHeight << " total area: " << _totalArea << " total wirelength: " << _totalWirelength << " final cost: " << _finalCost << endl;
  }

  _totalRuntime = (double)(clock() - start) / CLOCKS_PER_SEC;
  return;
}

void Floorplanner::reportModifiedNodes() const
{
  cout << "Report modified nodes..." << endl;
  for (int i = 0; i < _modifiedNodes.size(); ++i)
  {
    cout << "Node " << i << ": " << _modifiedNodes[i]->getBlock()->getName() << " " << _modifiedNodes[i]->getX() << " " << _modifiedNodes[i]->getY() << endl;
  }
  return;
}

void Floorplanner::calculateOutput()
{
  _chipWidth = 0;
  _chipHeight = 0;
  _totalWirelength = 0;

  for (int i = 0; i < _blockNum; ++i)
  {
    _chipWidth = max(_chipWidth, _blockArray[i]->getX2());
    _chipHeight = max(_chipHeight, _blockArray[i]->getY2());
  }

  for (int i = 0; i < _netNum; ++i)
  {
    vector<Terminal *> terminalList = _netArray[i]->getTermList();
    double minX = _outlineWidth, minY = _outlineHeight, maxX = 0, maxY = 0;
    for (int j = 0; j < terminalList.size(); ++j)
    {
      double midX, midY;
      if (_blockName2Id.count(terminalList[j]->getName()))
      {
        Block *block = _blockArray[_blockName2Id[terminalList[j]->getName()]];
        midX = (double)(block->getX1() + block->getX2()) / 2.0;
        midY = (double)(block->getY1() + block->getY2()) / 2.0;
      }
      else
      {
        midX = terminalList[j]->getX1();
        midY = terminalList[j]->getY1();
      }
      minX = min(minX, midX);
      minY = min(minY, midY);
      maxX = max(maxX, midX);
      maxY = max(maxY, midY);
    }
    _totalWirelength += (maxX - minX) + (maxY - minY);
  }

  _totalArea = _chipWidth * _chipHeight;
  _finalCost = _alpha * (_totalArea) + (1 - _alpha) * _totalWirelength;
}

void Floorplanner::reportBlockName2TreeNode() const
{
  cout << "Report block name to tree node..." << endl;
  for (unordered_map<string, TreeNode *>::const_iterator it = _blockName2TreeNode.begin(); it != _blockName2TreeNode.end(); ++it)
  {
    cout << "Block name: " << it->first << " x: " << it->second->getX() << " y: " << it->second->getY() << endl;
  }
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

void Floorplanner::reportBStarTree(TreeNode *node) const
{
  if (node == nullptr)
  {
    cout << "NULL" << endl;
    return;
  }

  cout << "Block: " << node->getBlock()->getName() << " width: " << node->getBlock()->getWidth() << " height: " << node->getBlock()->getHeight() << " x: " << node->getX() << " y: " << node->getY() << endl;

  cout << "Left child: " << endl;
  reportBStarTree(node->getLeft());
  cout << "Right child: " << endl;
  reportBStarTree(node->getRight());

  return;
}

void Floorplanner::reportHeightMap() const
{
  cout << "Report height map..." << endl;
  for (map<size_t, size_t>::const_iterator it = _heightMap.begin(); it != _heightMap.end(); ++it)
  {
    cout << "x: " << it->first << " y: " << it->second << endl;
  }
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
    // TreeNode *currNode = _blockName2TreeNode[_blockArray[i]->getName()];
    buff << _blockArray[i]->getName() << " " << _blockArray[i]->getX1() << " " << _blockArray[i]->getY1() << " " << _blockArray[i]->getX2() << " " << _blockArray[i]->getY2() << endl;
    // buff << currNode->getBlock()->getName() << " " << currNode->getX() << " " << currNode->getY() << " " << currNode->getX() + currNode->getBlock()->getWidth() << " " << currNode->getY() + currNode->getBlock()->getHeight() << endl;
    outFile << buff.str();
    buff.str("");
  }

  return;
}

void Floorplanner::clearTree(TreeNode *currNode)
{
  if (currNode == nullptr)
  {
    return;
  }

  clearTree(currNode->getLeft());
  clearTree(currNode->getRight());

  delete currNode;

  _blockName2TreeNode.clear();
  _bStarTreeRoot = nullptr;

  return;
}

void Floorplanner::clear()
{
  for (int i = 0; i < _blockNum; ++i)
  {
    delete _blockArray[i];
  }
  for (int i = 0; i < _terminalNum; ++i)
  {
    delete _terminalArray[i];
  }
  for (int i = 0; i < _netNum; ++i)
  {
    delete _netArray[i];
  }
  clearTree(_bStarTreeRoot);

  return;
}