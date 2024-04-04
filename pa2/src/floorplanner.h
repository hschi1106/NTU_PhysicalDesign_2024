#ifndef FLOORPLANNER_H
#define FLOORPLANNER_H

#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include "module.h"
#include "Bstar.h"
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

  // basic access methods
  size_t getTotalArea() const { return _totalArea; }
  size_t getOutlineWidth() const { return _outlineWidth; }
  size_t getOutlineHeight() const { return _outlineHeight; }
  TreeNode *getBStarTreeRoot() const { return _bStarTreeRoot; }

  // modify method
  void parseInput(fstream &blockInFile, fstream &netInFile);
  void createBStarTree();
  void floorplan();

  // member functions about reporting
  void printSummary() const;
  void reportModule() const;
  void reportBStarTree(TreeNode *node) const;
  void writeResult(fstream &outFile);

private:
  double _alpha;         // the alpha constant
  size_t _totalArea;     // total area of the modules
  size_t _outlineWidth;  // width of the outline
  size_t _outlineHeight; // height of the outline

  int _terminalNum;                                      // number of terminals
  int _blockNum;                                         // number of blocks
  int _netNum;                                           // number of nets
  vector<Terminal *> _terminalArray;                     // array of terminals
  vector<Block *> _blockArray;                           // array of blocks
  vector<Net *> _netArray;                               // array of nets
  unordered_map<string, int> _terminalName2Id;           // terminal name to id
  unordered_map<string, int> _blockName2Id;              // block name to id
  unordered_map<string, TreeNode *> _blockName2TreeNode; // net name to id
  TreeNode *_bStarTreeRoot;                              // the root of B* tree

  size_t _chipWidth;       // width of the chip
  size_t _chipHeight;      // height of the chip
  double _totalWirelength; // total wirelength of the floorplan
  double _finalCost;       // the cost of the floorplan
  double _totalRuntime;    // total runtime of the floorplanner

  void clear();
};

#endif // FLOORPLANNER_H