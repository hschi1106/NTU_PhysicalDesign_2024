#ifndef BSTAR_H
#define BSTAR_H
#include <vector>
#include "module.h"

class TreeNode
{
public:
  TreeNode() : _left(nullptr), _right(nullptr), _parent(nullptr), _block(nullptr) {}
  TreeNode(Block *block) : _left(nullptr), _right(nullptr), _parent(nullptr), _block(block) {}
  Block *getBlock() { return _block; }                   // get the block of the node
  TreeNode *getLeft() { return _left; }                  // get the left child of the node
  TreeNode *getRight() { return _right; }                // get the right child of the node
  TreeNode *getParent() { return _parent; }              // get the parent of the node
  void setBlock(Block *block) { _block = block; }        // set the block of the node
  void setLeft(TreeNode *left) { _left = left; }         // set the left child of the node
  void setRight(TreeNode *right) { _right = right; }     // set the right child of the node
  void setParent(TreeNode *parent) { _parent = parent; } // set the parent of the node

private:
  TreeNode *_left;   // the left child of the node
  TreeNode *_right;  // the right child of the node
  TreeNode *_parent; // the parent of the node
  Block *_block;     // the block that the node represents
};

#endif // BSTAR_H