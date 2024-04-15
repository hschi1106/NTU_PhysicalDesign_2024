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
    _totalArea += block->getArea();
    _blockName2Id[_blockArray[i]->getName()] = i;
  }

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
  // create the B*-tree
  for (int i = 0; i < _blockNum; ++i)
  {
    int blockId = i;
    string name = _blockArray[i]->getName();
    size_t width = _blockArray[i]->getWidth();
    size_t height = _blockArray[i]->getHeight();
    TreeNode *node = new TreeNode(blockId, name, width, height);
    _blockName2TreeNode[_blockArray[i]->getName()] = node;
  }

  _treeRoot = _blockName2TreeNode[_blockArray[0]->getName()];

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

TreeNode *Floorplanner::replicateBStarTree(TreeNode *currNode, TreeNode *parent)
{
  // replicate the B*-tree for perturbation
  if (currNode == nullptr)
  {
    return nullptr;
  }

  int blockId = currNode->getBlockId();
  string name = currNode->getName();
  size_t width = currNode->getWidth();
  size_t height = currNode->getHeight();

  TreeNode *newNode = new TreeNode(blockId, name, width, height);
  _blockName2ModifiedTreeNode[currNode->getName()] = newNode;

  newNode->setParent(parent);
  newNode->setLeft(replicateBStarTree(currNode->getLeft(), newNode));
  newNode->setRight(replicateBStarTree(currNode->getRight(), newNode));
  return newNode;
}

void Floorplanner::deleteTree(TreeNode *currNode)
{
  // delete the tree
  if (currNode == nullptr)
  {
    return;
  }

  deleteTree(currNode->getLeft());
  deleteTree(currNode->getRight());
  delete currNode;

  return;
}

size_t Floorplanner::updateContourLine(TreeNode *currNode)
{
  // get the width and height of the current node
  ContourLineNode *curr = _start;
  size_t x1 = currNode->getX1(), x2 = currNode->getX2();
  size_t width = currNode->getWidth(), height = currNode->getHeight();
  if (currNode->getRotate())
  {
    swap(width, height);
  }

  // find the x1 coordinate
  while (curr->getX() < x1)
  {
    curr = curr->getNext();
  }
  // now: it->first == x1

  // find the y coordinate
  ContourLineNode *start = curr;
  size_t prevY = curr->getY(); // the previous y coordinate before x1
  size_t highestY = prevY;     // the highest y coordinate between x1 and x2
  bool hasNodeAtX2 = false;    // whether there is a node at x2
  while (curr != _end && curr->getX() < x2)
  {
    // update the highest y coordinate
    if (curr->getY() > highestY)
    {
      highestY = curr->getY();
    }

    // update the previous y coordinate before iterator moves to the next position
    prevY = curr->getY();

    // iterator moves to the next position
    curr = curr->getNext();

    // remove the redundant contour line node between x1 and x2
    if (curr != _start && curr->getPrev()->getX() > x1 && curr->getPrev()->getX() < x2)
    {
      // delete the previous node
      ContourLineNode *prev = curr->getPrev();
      prev->getPrev()->setNext(curr);
      curr->setPrev(prev->getPrev());
      delete prev;
    }
  }

  // make sure whether there is a node at x2
  if (curr->getX() == x2)
  {
    hasNodeAtX2 = true;
  }

  // update the contour line
  ContourLineNode *newNode1 = new ContourLineNode(x1, highestY + height);
  start->setNext(newNode1);
  newNode1->setPrev(start);
  if (hasNodeAtX2)
  {
    // update the node if there is a node at x2
    newNode1->setNext(curr);
    curr->setPrev(newNode1);
  }
  else
  {
    // create a new node if there is no node at x2
    ContourLineNode *newNode2 = new ContourLineNode(x2, prevY);
    newNode1->setNext(newNode2);
    newNode2->setPrev(newNode1);
    newNode2->setNext(curr);
    curr->setPrev(newNode2);
  }

  return highestY;
}

void Floorplanner::calculatePosition(TreeNode *currNode)
{
  // if the current node is nullptr, return
  if (currNode == nullptr)
  {
    return;
  }

  // if the current node is the root node, set the position and the contour line
  if (currNode->getParent() == nullptr)
  {
    size_t blockWidth = currNode->getWidth(), blockHeight = currNode->getHeight();
    if (currNode->getRotate())
    {
      swap(blockWidth, blockHeight);
    }
    currNode->setPos(0, 0, blockWidth, blockHeight);
    ContourLineNode *newNode1 = new ContourLineNode(0, blockHeight);
    ContourLineNode *newNode2 = new ContourLineNode(blockWidth, 0);
    newNode1->setNext(newNode2);
    newNode2->setPrev(newNode1);
    _start->setNext(newNode1);
    newNode1->setPrev(_start);
    _end->setPrev(newNode2);
    newNode2->setNext(_end);
  }

  if (currNode->getLeft() != nullptr)
  {
    // calculate the position of the left child
    size_t blockWidth = currNode->getLeft()->getWidth(), blockHeight = currNode->getLeft()->getHeight();
    if (currNode->getLeft()->getRotate())
    {
      swap(blockWidth, blockHeight);
    }

    // set the x1 and x2 of the left child
    size_t x1 = currNode->getX2();
    currNode->getLeft()->setPos(x1, 0, x1 + blockWidth, 0);

    // update the contour line and set the y1 and y2 of the left child
    size_t y1 = updateContourLine(currNode->getLeft());
    currNode->getLeft()->setPos(x1, y1, x1 + blockWidth, y1 + blockHeight);

    // recursively calculate the position of the left child
    calculatePosition(currNode->getLeft());
  }
  if (currNode->getRight() != nullptr)
  {
    // calculate the position of the right child
    size_t blockWidth = currNode->getRight()->getWidth(), blockHeight = currNode->getRight()->getHeight();
    if (currNode->getRight()->getRotate())
    {
      swap(blockWidth, blockHeight);
    }

    // set the x1 and x2 of the right child
    size_t x1 = currNode->getX1();
    currNode->getRight()->setPos(x1, 0, x1 + blockWidth, 0);

    // update the contour line and set the y1 and y2 of the right child
    size_t y1 = updateContourLine(currNode->getRight());
    currNode->getRight()->setPos(x1, y1, x1 + blockWidth, y1 + blockHeight);

    // recursively calculate the position of the right child
    calculatePosition(currNode->getRight());
  }

  return;
}

void Floorplanner::clearPosition(TreeNode *currNode)
{
  // if the current node is nullptr, return
  if (currNode == nullptr)
  {
    return;
  }

  // clear the position of the current node
  currNode->setPos(0, 0, 0, 0);

  // clear the position of the left and right child
  clearPosition(currNode->getLeft());
  clearPosition(currNode->getRight());

  return;
}

size_t Floorplanner::calculateChipWidth()
{
  // calculate the chip width by traversing the contour line backward
  ContourLineNode *currNode = _end->getPrev();
  size_t chipWidth = currNode->getX();

  return chipWidth;
}

size_t Floorplanner::calculateChipHeight()
{
  // calculate the chip height by traversing the contour line
  ContourLineNode *currNode = _start;
  size_t chipHeight = 0;
  while (currNode != nullptr)
  {
    chipHeight = max(chipHeight, currNode->getY());
    currNode = currNode->getNext();
  }

  return chipHeight;
}

double Floorplanner::calculateWirelength(unordered_map<string, TreeNode *> blockName2TreeNode)
{
  double wirelength = 0;

  // calculate the wirelength
  for (int i = 0; i < _netNum; ++i)
  {
    vector<Terminal *> terminalList = _netArray[i]->getTermList();
    double minX = _outlineWidth, minY = _outlineHeight, maxX = 0, maxY = 0;
    for (int j = 0; j < terminalList.size(); ++j)
    {
      // calculate the mid point of the block or terminal
      double midX, midY;
      if (blockName2TreeNode.count(terminalList[j]->getName()))
      {
        TreeNode *node = blockName2TreeNode[terminalList[j]->getName()];
        midX = (double)(node->getX1() + node->getX2()) / 2.0;
        midY = (double)(node->getY1() + node->getY2()) / 2.0;
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

    // calculate the HPWL
    wirelength += (maxX - minX) + (maxY - minY);
  }

  return wirelength;
}

double Floorplanner::calculateCost(TreeNode *currRoot, unordered_map<string, TreeNode *> blockName2TreeNode)
{
  // clear the position of the tree and calculate the position
  this->clearPosition(currRoot);
  this->calculatePosition(currRoot);

  // calculate the chip width and chip height
  size_t chipWidth = this->calculateChipWidth();
  size_t chipHeight = this->calculateChipHeight();

  // calculate the wirelength
  double wirelength = this->calculateWirelength(blockName2TreeNode);

  // calculate the area cost and wirelength cost
  double areaCost = (_alpha) * (chipWidth * chipHeight) / _averageArea;
  double wirelengthCost = (_beta)*wirelength / _averageWirelength;

  // calculate the aspect ratio cost
  double desiredAspectRatio = (double)_outlineHeight / _outlineWidth;
  double aspectRatio = (double)chipHeight / chipWidth;
  double aspectRatioCost = (1 - _alpha - _beta) * (desiredAspectRatio - aspectRatio) * (desiredAspectRatio - aspectRatio);

  // calculate cost with area, wirelength, and aspect ratio
  double cost = areaCost + wirelengthCost + aspectRatioCost;

  return cost;
}

void Floorplanner::rotateNode(TreeNode *rotatedNode)
{
  // rotate the node
  rotatedNode->rotateBolock();

  return;
}

void Floorplanner::swapNode(TreeNode *swappedNodeA, TreeNode *swappedNodeB)
{
  // swap two nodes

  // swap the blockId, width, height, name, and rotate
  int blockAId = swappedNodeA->getBlockId();
  int blockBId = swappedNodeB->getBlockId();
  Block *blockA = _blockArray[blockAId];
  Block *blockB = _blockArray[blockBId];
  string blockAName = blockA->getName();
  string blockBName = blockB->getName();
  bool blockARotate = swappedNodeA->getRotate();
  bool blockBRotate = swappedNodeB->getRotate();

  // set the new values
  swappedNodeA->setBlockId(blockBId);
  swappedNodeB->setBlockId(blockAId);
  swappedNodeA->setWidth(blockB->getWidth());
  swappedNodeA->setHeight(blockB->getHeight());
  swappedNodeB->setWidth(blockA->getWidth());
  swappedNodeB->setHeight(blockA->getHeight());
  swappedNodeA->setName(blockBName);
  swappedNodeB->setName(blockAName);
  swappedNodeA->setRotate(blockBRotate);
  swappedNodeB->setRotate(blockARotate);

  // update the blockName2ModifiedTreeNode
  _blockName2ModifiedTreeNode[blockAName] = swappedNodeB;
  _blockName2ModifiedTreeNode[blockBName] = swappedNodeA;

  return;
}

void Floorplanner::deleteNode(TreeNode *deletedNode)
{
  // delete the node
  if (deletedNode->getLeft() == nullptr && deletedNode->getRight() == nullptr)
  {
    // if the deleted node is a leaf node
    TreeNode *parent = deletedNode->getParent();
    if (parent->getLeft() == deletedNode)
    {
      parent->setLeft(nullptr);
    }
    else
    {
      parent->setRight(nullptr);
    }
    _blockName2ModifiedTreeNode.erase(deletedNode->getName());
    delete deletedNode;
  }
  else if (deletedNode->getLeft() == nullptr)
  {
    // if the deleted node has only right child
    TreeNode *parent = deletedNode->getParent();
    TreeNode *rightChild = deletedNode->getRight();
    if (parent != nullptr)
    {
      if (parent->getLeft() == deletedNode)
      {
        parent->setLeft(rightChild);
      }
      else
      {
        parent->setRight(rightChild);
      }
      rightChild->setParent(parent);
    }
    else
    {
      rightChild->setParent(nullptr);
      _modifiedTreeRoot = rightChild;
    }
    _blockName2ModifiedTreeNode.erase(deletedNode->getName());
    delete deletedNode;
  }
  else if (deletedNode->getRight() == nullptr)
  {
    // if the deleted node has only left child
    TreeNode *parent = deletedNode->getParent();
    TreeNode *leftChild = deletedNode->getLeft();
    if (parent != nullptr)
    {
      if (parent->getLeft() == deletedNode)
      {
        parent->setLeft(leftChild);
      }
      else
      {
        parent->setRight(leftChild);
      }
      leftChild->setParent(parent);
    }
    else
    {
      leftChild->setParent(nullptr);
      _modifiedTreeRoot = leftChild;
    }
    _blockName2ModifiedTreeNode.erase(deletedNode->getName());
    delete deletedNode;
  }
  else
  {
    // if the deleted node has two children
    // swap the deleted node with its left or right child and delete the child
    bool leftOrRight = rand() % 2;
    TreeNode *successor;
    if (leftOrRight)
    {
      successor = deletedNode->getLeft();
    }
    else
    {
      successor = deletedNode->getRight();
    }
    swapNode(deletedNode, successor);
    deleteNode(successor);
  }

  return;
}

void Floorplanner::insertNode(TreeNode *insertedNode)
{
  // randomly choose a target node
  int targetId = rand() % _blockNum;

  // make sure the target node is not the deleted node
  while (_blockName2ModifiedTreeNode.count(_blockArray[targetId]->getName()) == 0)
  {
    targetId = rand() % _blockNum;
  }
  TreeNode *targetNode = _blockName2ModifiedTreeNode[_blockArray[targetId]->getName()];

  // insert the node to the target node's left or right
  bool leftOrRight = rand() % 2;
  if (leftOrRight)
  {
    TreeNode *leftChild = targetNode->getLeft();
    if (leftChild != nullptr)
    {
      leftChild->setParent(insertedNode);
    }
    targetNode->setLeft(insertedNode);
    insertedNode->setParent(targetNode);
    insertedNode->setLeft(leftChild);
  }
  else
  {
    TreeNode *rightChild = targetNode->getRight();
    if (rightChild != nullptr)
    {
      rightChild->setParent(insertedNode);
    }
    targetNode->setRight(insertedNode);
    insertedNode->setParent(targetNode);
    insertedNode->setRight(rightChild);
  }

  // update the blockName2ModifiedTreeNode
  _blockName2ModifiedTreeNode[insertedNode->getName()] = insertedNode;

  return;
}

void Floorplanner::perturb()
{
  int operation = rand() % 3; // 0: rotate, 1: swap, 2: move

  // randomly perturb the tree
  if (operation == 0)
  {
    // choose a random node
    int Id = rand() % _blockNum;
    string name = _blockArray[Id]->getName();
    TreeNode *rotatedNode = _blockName2ModifiedTreeNode[name];

    // rotate a node
    this->rotateNode(rotatedNode);
  }
  else if (operation == 1)
  {
    // choose two random nodes
    int IdA = rand() % _blockNum;
    int IdB = rand() % _blockNum;
    while (IdA == IdB)
    {
      IdB = rand() % _blockNum;
    }
    string nameA = _blockArray[IdA]->getName();
    string nameB = _blockArray[IdB]->getName();
    TreeNode *swappedNodeA = _blockName2ModifiedTreeNode[nameA];
    TreeNode *swappedNodeB = _blockName2ModifiedTreeNode[nameB];

    // swap two nodes
    this->swapNode(swappedNodeA, swappedNodeB);
  }
  else
  {
    // choose a random node
    int Id = rand() % _blockNum;
    string name = _blockArray[Id]->getName();
    TreeNode *deletedNode = _blockName2ModifiedTreeNode[name];

    // create a replicate node
    size_t width = deletedNode->getWidth();
    size_t height = deletedNode->getHeight();
    TreeNode *insertedNode = new TreeNode(Id, name, width, height);
    insertedNode->setRotate(deletedNode->getRotate());

    // delete the chosen node
    this->deleteNode(deletedNode);

    // insert replicate node
    this->insertNode(insertedNode);
  }
}

void Floorplanner::SA(double initTemp, double coolingRate, double stopTemp)
{
  srand(time(NULL));

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
      double oldCost = this->calculateCost(_treeRoot, _blockName2TreeNode);

      _modifiedTreeRoot = this->replicateBStarTree(_treeRoot, nullptr);

      // randomly perturb modified tree
      for (int i = 0; i < 2 * _blockNum + 20; ++i)
      {
        this->perturb();
      }

      double newCost = this->calculateCost(_modifiedTreeRoot, _blockName2ModifiedTreeNode);
      double prob = (double)rand() / (RAND_MAX);
      double deltaCost = newCost - oldCost;
      // cout << "deltaCost: " << deltaCost << endl;
      if (deltaCost <= 0)
      {
        // cout << "better and accept" << endl;
        // cout << "deltaCost: " << deltaCost << endl;
        // cout << "=============================" << endl;
        // accept the new status and update best
        this->deleteTree(_treeRoot);
        _treeRoot = _modifiedTreeRoot;
        _blockName2TreeNode = _blockName2ModifiedTreeNode;
        _blockName2ModifiedTreeNode.clear();
        if (newCost < _bestCost)
        {
          // cout << "update" << endl;
          // cout << "cost: " << newCost << endl;
          // cout << "=============================" << endl;
          _bestCost = newCost;
          this->writeBestCoordinateToBlock(_treeRoot);
        }
      }
      else if (1 - prob < exp((-deltaCost) / T))
      {
        // cout << "not better but accept" << endl;
        // cout << "deltaCost: " << deltaCost << endl;
        // cout << "=============================" << endl;
        // accept the new status but no update best
        this->deleteTree(_treeRoot);
        _treeRoot = _modifiedTreeRoot;
        _blockName2TreeNode = _blockName2ModifiedTreeNode;
        _blockName2ModifiedTreeNode.clear();
      }
      else
      {
        // restore the original status
        this->deleteTree(_modifiedTreeRoot);
        _blockName2ModifiedTreeNode.clear();
      }
    }
  }
}

void Floorplanner::fastSA(double constP, int constK, int constC)
{
  double T, T1;
  int iterNum = _blockNum * _blockNum;
  double totalDeltaCost; // for calculating deltaCostAvg

  // Fast Simulated Annealing starts
  for (int r = 1; r <= iterNum; ++r)
  {
    if (r == 1)
    {
      T = _deltaAvg / abs(log(constP));
      T1 = T;
    }
    else if (T <= constK)
    {
      T = T1 * totalDeltaCost / (2 * _blockNum + 20) / r / r / constC;
    }
    else
    {
      T = T1 * totalDeltaCost / (2 * _blockNum + 20) / r / r;
    }

    for (int i = 0; i < 2 * _blockNum + 20; ++i)
    {
      // calculate the cost of the original tree
      double oldCost = this->calculateCost(_treeRoot, _blockName2TreeNode);

      // replicate the tree for modification
      _modifiedTreeRoot = this->replicateBStarTree(_treeRoot, nullptr);

      // randomly perturb modified tree
      this->perturb();

      // calculate the cost of the modified tree
      double newCost = this->calculateCost(_modifiedTreeRoot, _blockName2ModifiedTreeNode);

      // calculate the delta cost and probability
      double prob = (double)rand() / (RAND_MAX);
      double deltaCost = newCost - oldCost;
      totalDeltaCost += deltaCost;

      if (deltaCost <= 0)
      {
        // accept the new status and update
        this->deleteTree(_treeRoot);
        _treeRoot = _modifiedTreeRoot;
        _blockName2TreeNode = _blockName2ModifiedTreeNode;
        _blockName2ModifiedTreeNode.clear();
        if (newCost < _bestCost)
        {
          // if the status is the globally best status, update the best status
          cout << "update at " << r << "th iteration" << endl;
          _bestCost = newCost;
          this->writeBestCoordinateToBlock(_treeRoot);
        }
      }
      else if (1 - prob < exp((-deltaCost) / T))
      {
        // accept the new status but no update best
        this->deleteTree(_treeRoot);
        _treeRoot = _modifiedTreeRoot;
        _blockName2TreeNode = _blockName2ModifiedTreeNode;
        _blockName2ModifiedTreeNode.clear();
      }
      else
      {
        // restore the original status
        this->deleteTree(_modifiedTreeRoot);
        _blockName2ModifiedTreeNode.clear();
      }
    }
  }
}

void Floorplanner::initContourLine()
{
  // clear the contour line
  ContourLineNode *curr = _start;
  if (curr != nullptr)
  {
    curr = curr->getNext();
    while (curr != _end)
    {
      ContourLineNode *next = curr->getNext();
      delete curr;
      curr = next;
    }
  }

  // initialize the contour line
  _start = new ContourLineNode(0, 0);
  _end = new ContourLineNode(SIZE_MAX, 0);
  _start->setNext(_end);
  _end->setPrev(_start);
  return;
}

void Floorplanner::calculateNorm()
{
  // set iteration number and perturbation number
  int iterNum = 20, perturbNum = _blockNum;

  // calculate the average area and average wirelength
  for (int i = 0; i < iterNum; ++i)
  {
    _modifiedTreeRoot = this->replicateBStarTree(_treeRoot, nullptr);
    this->initContourLine();
    for (int j = 0; j < perturbNum; ++j)
    {
      this->perturb();
    }
    this->calculatePosition(_modifiedTreeRoot);
    size_t chipWidth = this->calculateChipWidth();
    size_t chipHeight = this->calculateChipHeight();
    double wirelength = this->calculateWirelength(_blockName2ModifiedTreeNode);
    this->deleteTree(_modifiedTreeRoot);
    _blockName2ModifiedTreeNode.clear();
    _averageArea += chipWidth * chipHeight;
    _averageWirelength += wirelength;
    _maxArea = max(_maxArea, chipWidth * chipHeight);
    _minArea = min(_minArea, chipWidth * chipHeight);
    _maxWirelength = max(_maxWirelength, wirelength);
    _minWirelength = min(_minWirelength, wirelength);
  }
  _averageArea /= iterNum;
  _averageWirelength /= iterNum;
  _deltaAvg = _alpha * _averageArea / _minArea + (1 - _alpha) * _averageWirelength / _minWirelength;

  cout << "Average area: " << _averageArea << " Average wirelength: " << _averageWirelength << endl;
  cout << "Max area: " << _maxArea << " Max wirelength: " << _maxWirelength << endl;
  cout << "Min area: " << _minArea << " Min wirelength: " << _minWirelength << endl;

  return;
}

void Floorplanner::floorplan()
{
  clock_t start = clock();
  srand(time(NULL));

  while (_chipHeight > _outlineHeight || _chipWidth > _outlineWidth)
  {
    // Initialize the tree and contour line
    this->initContourLine();
    this->createBStarTree();

    // calculate the norm to normalize the cost
    this->calculateNorm();

    // fast Simulated Annealing
    this->fastSA(0.9, 7, 100);

    // calculate the output
    this->calculateOutput();
    cout << "Chip width: " << _chipWidth << " Chip height: " << _chipHeight << " Total wirelength: " << _totalWirelength << " Final cost: " << _finalCost << endl;

    // clear the tree
    this->deleteTree(_treeRoot);
  }

  _totalRuntime = (double)(clock() - start) / CLOCKS_PER_SEC;

  return;
}

void Floorplanner::writeBestCoordinateToBlock(TreeNode *currNode)
{
  if (currNode == nullptr)
  {
    return;
  }

  Block *block = _blockArray[currNode->getBlockId()];
  block->setPos(currNode->getX1(), currNode->getY1(), currNode->getX2(), currNode->getY2());

  if (currNode->getLeft() != nullptr)
  {
    this->writeBestCoordinateToBlock(currNode->getLeft());
  }
  if (currNode->getRight() != nullptr)
  {
    this->writeBestCoordinateToBlock(currNode->getRight());
  }

  return;
}

void Floorplanner::reportBlockName2TreeNode() const
{
  cout << "Report block name to tree node..." << endl;
  for (auto it = _blockName2TreeNode.begin(); it != _blockName2TreeNode.end(); ++it)
  {
    cout << "Block name: " << it->first << " ";
    cout << "Node name: " << it->second->getName() << " ";
    cout << "Block id: " << it->second->getBlockId() << " ";
    cout << "Block width: " << it->second->getWidth() << " ";
    cout << "Block height: " << it->second->getHeight() << endl;
  }
  return;
}

void Floorplanner::reportBlockName2ModifiedTreeNode() const
{
  cout << "Report block name to modified tree node..." << endl;
  for (auto it = _blockName2ModifiedTreeNode.begin(); it != _blockName2ModifiedTreeNode.end(); ++it)
  {
    cout << "Block name: " << it->first << " ";
    cout << "Node name: " << it->second->getName() << " ";
    cout << "Block id: " << it->second->getBlockId() << " ";
    cout << "Block width: " << it->second->getWidth() << " ";
    cout << "Block height: " << it->second->getHeight() << endl;
  }
  return;
}

void Floorplanner::calculateOutput()
{
  _chipHeight = 0;
  _chipWidth = 0;
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

  for (int i = 0; i < _blockNum; ++i)
  {
    _chipWidth = max(_chipWidth, _blockArray[i]->getX2());
    _chipHeight = max(_chipHeight, _blockArray[i]->getY2());
  }

  double aspectRatio = (1 - _alpha - _beta) * (_outlineHeight / _outlineWidth - _chipHeight / _chipWidth) * (_outlineHeight / _outlineWidth - _chipHeight / _chipWidth);

  _finalCost = _alpha * (_chipWidth * _chipHeight) + (1 - _alpha) * _totalWirelength;

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

  cout << "Block: " << _blockArray[node->getBlockId()]->getName() << " x1: " << node->getX1() << " y1: " << node->getY1() << " x2: " << node->getX2() << " y2: " << node->getY2() << endl;

  cout << "Left child: " << endl;
  reportBStarTree(node->getLeft());
  cout << "Right child: " << endl;
  reportBStarTree(node->getRight());

  return;
}

void Floorplanner::reportContourLine() const
{
  ContourLineNode *currNode = _start;
  while (currNode != nullptr)
  {
    cout << "x: " << currNode->getX() << " y: " << currNode->getY() << endl;
    currNode = currNode->getNext();
  }
  cout << endl;
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
    Block *block = _blockArray[i];
    buff << block->getName() << " " << block->getX1() << " " << block->getY1() << " " << block->getX2() << " " << block->getY2() << endl;
    outFile << buff.str();
    buff.str("");
  }

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
  deleteTree(_treeRoot);

  return;
}