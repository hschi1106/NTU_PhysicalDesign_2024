#ifndef FLOORPLANNER_H
#define FLOORPLANNER_H

#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <climits>
#include "module.h"
using namespace std;

class Floorplanner
{
public:
  // constructor and destructor
  Floorplanner(double alpha, fstream &blockInFile, fstream &netInFile) : _alpha(alpha), _totalArea(0), _chipWidth(0), _chipHeight(0), _totalWirelength(0), _finalCost(0), _totalRuntime(0)
  {
    parseInput(blockInFile, netInFile);
  }
  ~Floorplanner()
  {
    clear();
  }

  // modify method
  void parseInput(fstream &blockInFile, fstream &netInFile);

  // B*-tree construction
  void createBStarTree();                         // create the B* tree
  void calculateTreePosition(TreeNode *currNode); // calculate the position of the blocks
  size_t calculateTreeY(TreeNode *currNode);      // calculate the y coordinate of the blocks
  void clearTreePosition(TreeNode *currNode);     // clear the position of the blocks
  size_t calculateTreeChipWidth();
  size_t calculateTreeChipHeight();
  double calculateTreeWirelength();
  double calculateTreeCost();

  // perturbation methods
  void randomlyRotateBlock();
  void randomlySwapNodes();
  void randomlyMoveLeaf();
  void randomlyMoveSubtree();
  void restoreLastStatus();
  void deleteLastStatus();
  void perturb();

  // floorplanning
  void floorplan();
  void SA(double initTemp, double coolingRate, double stopTemp);
  void fastSA();
  void writeBestCoordinate(TreeNode *currNode);
  void calculateAverage();

  // calculate output value
  void calculateOutput();

  // member functions about reporting
  void printSummary() const;
  void reportModule() const;
  void reportBStarTree(TreeNode *node) const;
  void reportHeightMap() const;
  void writeResult(fstream &outFile);
  void reportModifiedNodes() const;
  void reportBlockName2TreeNode() const;

private:
  // hyperparameters
  double _alpha;  // the alpha constant
  double _beta;   // the beta constant
  double _constP; // the constant P
  double _constC; // the constant c
  double _constk; // the constant k

  // attributes for floorplanner
  size_t _totalArea;     // total area of the modules
  size_t _outlineWidth;  // width of the outline
  size_t _outlineHeight; // height of the outline
  size_t _averageArea;   // average area for calculating cost
  size_t _averageWirelength;  // average wirelength for calculating cost
  size_t _averageCost;        // average cost for calculating cost
  bool _adaptOutline = false; // whether to adapt the outline

  // attributes for input
  int _terminalNum;                            // number of terminals
  int _blockNum;                               // number of blocks
  int _netNum;                                 // number of nets
  vector<Terminal *> _terminalArray;           // array of terminals
  vector<Block *> _blockArray;                 // array of blocks
  vector<Net *> _netArray;                     // array of nets
  unordered_map<string, int> _terminalName2Id; // terminal name to id
  unordered_map<string, int> _blockName2Id;    // block name to id

  // attributes for B*-tree
  unordered_map<string, TreeNode *> _blockName2TreeNode; // block name to tree node
  TreeNode *_bStarTreeRoot;                              // the root of B* tree
  map<size_t, size_t> _heightMap;                        // the height map
  vector<TreeNode *> _modifiedNodes;                     // the modified nodes
  TreeNode *_lastIsNull = new TreeNode();                // point to this if the last is null

  // attributes for output
  size_t _chipWidth;       // width of the chip
  size_t _chipHeight;      // height of the chip
  double _totalWirelength; // total wirelength of the floorplan
  double _finalCost;       // the cost of the floorplan
  double _totalRuntime;    // total runtime of the floorplanner

  void clearTree(TreeNode *currNode);
  void clear();
};

#endif // FLOORPLANNER_H