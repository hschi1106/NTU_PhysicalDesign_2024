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
  // this may be deleted if no improvement
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

size_t Floorplanner::calculateY(TreeNode *currNode)
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
    if (it != _heightMap.begin() && (it)->first > placeX && prev(it)->first < placeX + blockWidth)
    {
      _heightMap.erase(prev(it));
    }
  }

  // update the height map
  _heightMap[placeX] = highestY + blockHeight;
  _heightMap[placeX + blockWidth] = _heightMap.find(placeX + blockWidth) == _heightMap.end() ? prevY : _heightMap[placeX + blockWidth];

  // cout << "updated height map" << endl;
  // cout << "x " << placeX << " y " << highestY + blockHeight << endl;
  // cout << "x " << placeX + blockWidth << " y " << prevY << endl;

  // this->reportHeightMap();

  return highestY;
}

void Floorplanner::calculatePosition(TreeNode *currNode)
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
    // this->reportHeightMap();
  }

  if (currNode->getLeft() != nullptr)
  {
    // calculate the x position
    currNode->getLeft()->setX(placeX + blockWidth);
    currNode->getLeft()->setY(calculateY(currNode->getLeft()));
    // cout << "place " << currNode->getLeft()->getBlock()->getName() << " at x: " << currNode->getLeft()->getX() << " y: " << currNode->getLeft()->getY() << endl;
    // cout << "blockWidth: " << currNode->getLeft()->getBlock()->getWidth() << " blockHeight: " << currNode->getLeft()->getBlock()->getHeight() << endl;
    // cout << endl;
    // cout << endl;
    calculatePosition(currNode->getLeft());
  }

  if (currNode->getRight() != nullptr)
  {
    // calculate the x position
    currNode->getRight()->setX(placeX);
    currNode->getRight()->setY(calculateY(currNode->getRight()));
    // cout << "place " << currNode->getRight()->getBlock()->getName() << " at x: " << currNode->getRight()->getX() << " y: " << currNode->getRight()->getY() << endl;
    // cout << "blockWidth: " << currNode->getRight()->getBlock()->getWidth() << " blockHeight: " << currNode->getRight()->getBlock()->getHeight() << endl;
    // cout << endl;
    // cout << endl;
    calculatePosition(currNode->getRight());
  }

  return;
}

void Floorplanner::clearPosition(TreeNode *currNode)
{
  if (currNode == nullptr)
  {
    return;
  }

  currNode->setX(0);
  currNode->setY(0);

  clearPosition(currNode->getLeft());
  clearPosition(currNode->getRight());

  return;
}

void Floorplanner::calculateChipSize()
{
  _chipWidth = 0;
  _chipHeight = 0;
  for (map<size_t, size_t>::iterator it = _heightMap.begin(); it != _heightMap.end(); ++it)
  {
    _chipWidth = max(_chipWidth, it->first);
    _chipHeight = max(_chipHeight, it->second);
  }

  return;
}

void Floorplanner::calculateWirelength()
{
  _totalWirelength = 0;
  for (int i = 0; i < _netNum; ++i)
  {
    vector<Terminal *> terminalList = _netArray[i]->getTermList();
    double minX = _chipWidth, minY = _chipHeight, maxX = 0, maxY = 0;
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
    _totalWirelength += (maxX - minX) + (maxY - minY);
  }

  return;
}

void Floorplanner::calculateCost()
{
  _finalCost = _alpha * (_chipWidth * _chipHeight) + (1 - _alpha) * _totalWirelength;

  return;
}

void Floorplanner::rotateBlock(TreeNode *rotatedNode)
{
  // rotate the block
  Block *rotatedBlock = rotatedNode->getBlock();
  rotatedBlock->rotateBolock();

  return;
}

void Floorplanner::swapNodes(TreeNode *swapedNodeA, TreeNode *swapedNodeB)
{
  // swap two nodes
  Block *blockA = swapedNodeA->getBlock();
  Block *blockB = swapedNodeB->getBlock();

  swapedNodeA->setBlock(blockB);
  swapedNodeB->setBlock(blockA);

  // update the blockName2TreeNode
  _blockName2TreeNode[blockA->getName()] = swapedNodeB;
  _blockName2TreeNode[blockB->getName()] = swapedNodeA;

  return;
}

void Floorplanner::deleteNode(TreeNode *deletedNode)
{
  if (deletedNode->getLeft() == nullptr && deletedNode->getRight() == nullptr)
  {
    // delete the node
    if (deletedNode->getParent()->getLeft() == deletedNode)
    {
      deletedNode->getParent()->setLeft(nullptr);
    }
    else
    {
      deletedNode->getParent()->setRight(nullptr);
    }
    _blockName2TreeNode.erase(deletedNode->getBlock()->getName());
    delete deletedNode;
  }
  else if (deletedNode->getLeft() == nullptr)
  {
    if (deletedNode->getParent() != nullptr)
    {
      if (deletedNode->getParent()->getLeft() == deletedNode)
      {
        deletedNode->getParent()->setLeft(deletedNode->getRight());
      }
      else
      {
        deletedNode->getParent()->setRight(deletedNode->getRight());
      }
    }
    else
    {
      _bStarTreeRoot = deletedNode->getRight();
    }

    // delete the node
    _blockName2TreeNode.erase(deletedNode->getBlock()->getName());
    delete deletedNode;
  }
  else if (deletedNode->getRight() == nullptr)
  {
    if (deletedNode->getParent() != nullptr)
    {
      if (deletedNode->getParent()->getLeft() == deletedNode)
      {
        deletedNode->getParent()->setLeft(deletedNode->getLeft());
      }
      else
      {
        deletedNode->getParent()->setRight(deletedNode->getLeft());
      }
    }
    else
    {
      _bStarTreeRoot = deletedNode->getLeft();
    }

    // delete the node
    _blockName2TreeNode.erase(deletedNode->getBlock()->getName());
    delete deletedNode;
  }
  else
  {
    // find the leftmost node of the left child
    TreeNode *leftmostNode = deletedNode->getLeft();
    while (leftmostNode->getLeft() != nullptr)
    {
      leftmostNode = leftmostNode->getLeft();
    }

    _blockName2TreeNode.erase(deletedNode->getBlock()->getName());
    // swap the rightmost node with the deleted node
    swapNodes(deletedNode, leftmostNode);

    // delete the rightmost node
    deleteNode(leftmostNode);
  }

  return;
}

void Floorplanner::insertNode(TreeNode *insertedNode, TreeNode *parent)
{
  // randomly pick left or right
  int leftOrRight = rand() % 2; // left: 0, right: 1
  if (leftOrRight == 0)
  {
    // insert the node to the left of the parent
    if (parent->getLeft() != nullptr)
    {
      parent->getLeft()->setParent(insertedNode);
    }
    insertedNode->setLeft(parent->getLeft());
    parent->setLeft(insertedNode);
    insertedNode->setParent(parent);
  }
  else
  {
    // insert the node to the right of the parent
    if (parent->getRight() != nullptr)
    {
      parent->getRight()->setParent(insertedNode);
    }
    insertedNode->setRight(parent->getRight());
    parent->setRight(insertedNode);
    insertedNode->setParent(parent);
  }

  _blockName2TreeNode[insertedNode->getBlock()->getName()] = insertedNode;

  return;
}

void Floorplanner::randomlyMoveNode(TreeNode *movedNode)
{
  // randomly pick target node
  int targetId = rand() % _blockNum;
  while (targetId == _blockName2Id[movedNode->getBlock()->getName()])
  {
    targetId = rand() % _blockNum;
  }
  Block *targetBlock = _blockArray[targetId];
  Block *movedBlock = movedNode->getBlock();
  TreeNode *targetNode = _blockName2TreeNode[targetBlock->getName()];

  // delete moved node
  deleteNode(movedNode);

  // restore moved block
  TreeNode *newNode = new TreeNode(movedBlock);

  // insert moved node to child of the target node
  insertNode(newNode, targetNode);

  return;
}

void Floorplanner::floorplan()
{
  clock_t start = clock();
  this->createBStarTree();
  this->calculatePosition(_bStarTreeRoot);
  // this->rotateBlock(_bStarTreeRoot->getLeft());
  // this->swapNodes(_bStarTreeRoot, _bStarTreeRoot->getRight());
  // Block *block = _bStarTreeRoot->getBlock();
  // TreeNode *node = new TreeNode(block);
  // this->deleteNode(_bStarTreeRoot);
  // this->insertNode(node, _bStarTreeRoot);
  this->randomlyMoveNode(_bStarTreeRoot);
  this->clearPosition(_bStarTreeRoot);
  this->calculatePosition(_bStarTreeRoot);
  this->calculateChipSize();
  this->calculateWirelength();
  this->calculateCost();
  _totalRuntime = (double)(clock() - start) / CLOCKS_PER_SEC;
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
    TreeNode *node = _blockName2TreeNode[_blockArray[i]->getName()];
    buff << _blockArray[i]->getName() << " " << node->getX() << " " << node->getY() << " " << node->getX() + _blockArray[i]->getWidth() << " " << node->getY() + _blockArray[i]->getHeight() << endl;
    outFile << buff.str();
    buff.str("");
  }

  return;
}