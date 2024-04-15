#ifndef FLOORPLANNER_H
#define FLOORPLANNER_H

#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include "module.h"
using namespace std;

class Floorplanner
{
public:
  // constructor and destructor
  Floorplanner(double alpha, fstream &blockInFile, fstream &netInFile)
      : _alpha(alpha), _beta(0.1), _totalArea(0), _chipWidth(SIZE_MAX), _chipHeight(SIZE_MAX), _averageArea(0), _averageWirelength(0), _totalWirelength(0), _finalCost(0), _totalRuntime(0), _maxArea(0), _maxWirelength(0), _minArea(SIZE_MAX), _minWirelength(SIZE_MAX), _bestCost(SIZE_MAX)
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
  void createBStarTree();                                             // create the B* tree
  TreeNode *replicateBStarTree(TreeNode *currNode, TreeNode *parent); // replicate the B* tree
  void deleteTree(TreeNode *currNode);                                // delete the B* tree
  void calculatePosition(TreeNode *currNode);                         // calculate the position of the blocks
  size_t updateContourLine(TreeNode *currNode);                       // update the contour line and return y1
  void clearPosition(TreeNode *currNode);                             // clear the position of the blocks

  // perturbation methods
  void perturb();                                                // perturb the modified B* tree
  void rotateNode(TreeNode *rotatedNode);                        // rotate the block in the modified B* tree
  void swapNode(TreeNode *swappedNodeA, TreeNode *swappedNodeB); // swap the blocks in the modified B* tree
  void deleteNode(TreeNode *deletedNode);                        // delete the block in the modified B* tree
  void insertNode(TreeNode *insertedNode);                       // insert the block in the modified B* tree

  // floorplanning
  void floorplan();                                              // floorplanning
  void initContourLine();                                        // initialize the contour line
  void calculateNorm();                                          // calculate the norm of the floorplan
  void SA(double initTemp, double coolingRate, double stopTemp); // run simulated annealing
  void fastSA(int iterNum, double constP, int constK, int constC);            // run fast simulated annealing
  void writeBestCoordinateToBlock(TreeNode *currNode);           // write the best coordinate to the blocks
  void calculateOutput();                                        // calculate the output value

  // calculate output value
  size_t calculateChipWidth();                                                                    // calculate the width of the chip by the height map
  size_t calculateChipHeight();                                                                   // calculate the height of the chip by the height map
  double calculateWirelength(unordered_map<string, TreeNode *> blockName2TreeNode);               // calculate the wirelength by B*-tree
  double calculateCost(TreeNode *currRoot, unordered_map<string, TreeNode *> blockName2TreeNode); // calculate the cost of the floorplan by B*-tree

  // member functions about reporting
  void printSummary() const;                     // print the summary of the floorplanner
  void reportModule() const;                     // report the module information for debugging
  void reportBStarTree(TreeNode *node) const;    // report the B* tree for debugging
  void reportContourLine() const;                // report the contour line for debugging
  void reportBlockName2TreeNode() const;         // report the block name to tree node for debugging
  void reportBlockName2ModifiedTreeNode() const; // report the block name to modified tree node for debugging
  void writeResult(fstream &outFile);            // write the result to the output file

private:
  // attributes for floorplanner
  double _alpha;             // the alpha constant
  double _beta;              // the beta constant
  size_t _totalArea;         // total area of the modules
  size_t _outlineWidth;      // width of the outline
  size_t _outlineHeight;     // height of the outline
  size_t _averageArea;       // average area of the modules
  size_t _averageWirelength; // average wirelength of the nets
  size_t _maxArea;           // maximum area of norm
  size_t _minArea;           // minimum area of norm
  double _maxWirelength;     // maximum wirelength of norm
  double _minWirelength;     // minimum wirelength of norm
  double _deltaAvg;          // average uphill cost
  double _bestCost;          // global best cost

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
  unordered_map<string, TreeNode *> _blockName2TreeNode;         // block id to tree node
  unordered_map<string, TreeNode *> _blockName2ModifiedTreeNode; // block id to modified tree node
  TreeNode *_treeRoot;                                           // the root of the best B* tree
  TreeNode *_modifiedTreeRoot;                                   // the root of the modified B* tree
  ContourLineNode *_start;                                       // the contour line of the B* tree
  ContourLineNode *_end;                                         // the contour line of the B* tree

  // attributes for output
  size_t _chipWidth;       // width of the chip
  size_t _chipHeight;      // height of the chip
  double _totalWirelength; // total wirelength of the floorplan
  double _finalCost;       // the cost of the floorplan
  double _totalRuntime;    // total runtime of the floorplanner

  void clear();
};

#endif // FLOORPLANNER_H