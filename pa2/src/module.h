#ifndef MODULE_H
#define MODULE_H

#include <vector>
#include <string>
using namespace std;

class Terminal
{
public:
    // constructor and destructor
    Terminal(string &name, size_t x, size_t y) : _name(name), _x1(x), _y1(y), _x2(x), _y2(y) {}
    ~Terminal() {}

    // basic access methods
    const string getName() { return _name; }
    const size_t getX1() { return _x1; }
    const size_t getX2() { return _x2; }
    const size_t getY1() { return _y1; }
    const size_t getY2() { return _y2; }

    // set functions
    void setName(string &name) { _name = name; }
    void setPos(size_t x1, size_t y1, size_t x2, size_t y2)
    {
        _x1 = x1;
        _y1 = y1;
        _x2 = x2;
        _y2 = y2;
    }

protected:
    string _name; // module name
    size_t _x1;   // min x coordinate of the terminal
    size_t _y1;   // min y coordinate of the terminal
    size_t _x2;   // max x coordinate of the terminal
    size_t _y2;   // max y coordinate of the terminal
};

class Block : public Terminal
{
public:
    // constructor and destructor
    Block(string &name, size_t w, size_t h) : Terminal(name, 0, 0), _w(w), _h(h), _rotate(false) {}
    ~Block() {}

    // basic access methods
    const size_t getWidth(bool rotate = false) { return rotate ? _h : _w; }
    const size_t getHeight(bool rotate = false) { return rotate ? _w : _h; }
    const size_t getArea() { return _h * _w; }
    static size_t getMaxX() { return _maxX; }
    static size_t getMaxY() { return _maxY; }

    // set functions
    void setWidth(size_t w) { _w = w; }
    void setHeight(size_t h) { _h = h; }
    static void setMaxX(size_t x) { _maxX = x; }
    static void setMaxY(size_t y) { _maxY = y; }
    void rotateBolock()
    {
        _rotate = !_rotate;
        swap(_w, _h);
    }

private:
    size_t _w;           // width of the block
    size_t _h;           // height of the block
    static size_t _maxX; // maximum x coordinate for all blocks
    static size_t _maxY; // maximum y coordinate for all blocks
    bool _rotate;        // whether the block is rotated
};

class Net
{
public:
    // constructor and destructor
    Net() {}
    ~Net() {}

    // basic access methods
    const vector<Terminal *> getTermList() { return _terminalList; }

    // modify methods
    void addTerminal(Terminal *terminal) { _terminalList.push_back(terminal); }

    // other member functions
    double calcHPWL();

private:
    vector<Terminal *> _terminalList; // list of terminals the net is connected to
};

class TreeNode
{
public:
    TreeNode() : _left(nullptr), _right(nullptr), _parent(nullptr), _block(nullptr), _x(0), _y(0), _oldLeft(nullptr), _oldRight(nullptr), _oldParent(nullptr), _lastRotated(false), _lastBlock(nullptr) {}
    TreeNode(Block *block) : _left(nullptr), _right(nullptr), _parent(nullptr), _block(block), _x(0), _y(0), _oldLeft(nullptr), _oldRight(nullptr), _oldParent(nullptr), _lastRotated(false), _lastBlock(nullptr) {}

    // basic access methods
    Block *getBlock() { return _block; }            // get the block of the node
    TreeNode *getLeft() { return _left; }           // get the left child of the node
    TreeNode *getRight() { return _right; }         // get the right child of the node
    TreeNode *getParent() { return _parent; }       // get the parent of the node
    const size_t getX() { return _x; }              // get the x coordinate of the block
    const size_t getY() { return _y; }              // get the y coordinate of the block
    TreeNode *getOldLeft() { return _oldLeft; }     // get the left child of the node
    TreeNode *getOldRight() { return _oldRight; }   // get the right child of the node
    TreeNode *getOldParent() { return _oldParent; } // get the parent of the node
    bool getLastRotated() { return _lastRotated; }          // get the rotated status of the block
    Block *getLastBlock() { return _lastBlock; }    // get the last block of the node

    // set functions
    void setBlock(Block *block) { _block = block; }              // set the block of the node
    void setLeft(TreeNode *left) { _left = left; }               // set the left child of the node
    void setRight(TreeNode *right) { _right = right; }           // set the right child of the node
    void setParent(TreeNode *parent) { _parent = parent; }       // set the parent of the node
    void setX(size_t x) { _x = x; }                              // set the x coordinate of the block
    void setY(size_t y) { _y = y; }                              // set the y coordinate of the block
    void setOldLeft(TreeNode *left) { _oldLeft = left; }         // set the left child of the node
    void setOldRight(TreeNode *right) { _oldRight = right; }     // set the right child of the node
    void setOldParent(TreeNode *parent) { _oldParent = parent; } // set the parent of the node
    void setLastRotated(bool lastRotated) { _lastRotated = lastRotated; }       // set the rotated status of the block
    void setLastBlock(Block *block) { _lastBlock = block; }      // set the last block of the node

private:
    Block *_block;        // the block that the node represents
    size_t _x;            // the x coordinate of the block
    size_t _y;            // the y coordinate of the block
    TreeNode *_left;      // the left child of the node
    TreeNode *_right;     // the right child of the node
    TreeNode *_parent;    // the parent of the node
    TreeNode *_oldLeft;   // the left child of the node
    TreeNode *_oldRight;  // the right child of the node
    TreeNode *_oldParent; // the parent of the node
    bool _lastRotated;        // whether the block is rotated in latest perturbation
    Block *_lastBlock;    // the block that the node represents in latest perturbation
};

#endif // MODULE_H
