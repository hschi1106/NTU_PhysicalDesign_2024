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
    bool getRotate() { return _rotate; }

    // set functions
    void setWidth(size_t w) { _w = w; }
    void setHeight(size_t h) { _h = h; }
    static void setMaxX(size_t x) { _maxX = x; }
    static void setMaxY(size_t y) { _maxY = y; }
    void rotateBolock()
    {
        _rotate = !_rotate;
    }
    void setRotate(bool rotate) { _rotate = rotate; }

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

class TreeNode : public Block
{
public:
    TreeNode(int blockId, string &name, size_t w, size_t h)
        : Block(name, w, h), _blockId(blockId), _left(nullptr), _right(nullptr), _parent(nullptr)
    {
    }

    // basic access methods
    TreeNode *getLeft() { return _left; }     // get the left child of the node
    TreeNode *getRight() { return _right; }   // get the right child of the node
    TreeNode *getParent() { return _parent; } // get the parent of the node
    int getBlockId() { return _blockId; }     // get the block id of the node

    // set functions
    void setLeft(TreeNode *left) { _left = left; }         // set the left child of the node
    void setRight(TreeNode *right) { _right = right; }     // set the right child of the node
    void setParent(TreeNode *parent) { _parent = parent; } // set the parent of the node
    void setBlockId(int blockId) { _blockId = blockId; }   // set the block id of the node

private:
    int _blockId;      // the id of the block
    TreeNode *_left;   // the left child of the node
    TreeNode *_right;  // the right child of the node
    TreeNode *_parent; // the parent of the node
};

class ContourLineNode
{
public:
    ContourLineNode(size_t x, size_t y) : _x(x), _y(y), _next(nullptr), _prev(nullptr) {}
    ~ContourLineNode() {}

    // basic access methods
    size_t getX() { return _x; }                 // get the x coordinate of the node
    size_t getY() { return _y; }                 // get the y coordinate of the node
    ContourLineNode *getNext() { return _next; } // get the next node in the contour line
    ContourLineNode *getPrev() { return _prev; } // get the previous node in the contour line

    // set functions
    void setX(size_t x) { _x = x; }                       // set the x coordinate of the node
    void setY(size_t y) { _y = y; }                       // set the y coordinate of the node
    void setNext(ContourLineNode *next) { _next = next; } // set the next node in the contour line
    void setPrev(ContourLineNode *prev) { _prev = prev; } // set the previous node in the contour line

private:
    size_t _x;              // x coordinate of the node
    size_t _y;              // y coordinate of the node
    ContourLineNode *_next; // the next node in the contour line
    ContourLineNode *_prev; // the previous node in the contour line
};

#endif // MODULE_H
