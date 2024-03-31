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
#include "cell.h"
#include "net.h"
#include "partitioner.h"
using namespace std;

// added member functions
void Partitioner::initPartition()
{
    int maxPinNum = 0, partSize[2] = {0, 0};

    // initialize partition with f(x)=max(netSize)+alpha*average(netSize)
    vector<Cell *> cellArrayBySortingIndex;
    for (int i = 0; i < _cellNum; ++i)
    {
        int maxNetSize = 0;
        int totalNetSize = 0;
        Cell *pickedCell = _cellArray[i];
        vector<int> pickedCellNetList = pickedCell->getNetList();
        for (int j = 0; j < pickedCellNetList.size(); ++j)
        {
            int netSize = _netArray[pickedCellNetList[j]]->getCellList().size();
            totalNetSize += netSize;
            if (netSize > maxNetSize)
            {
                maxNetSize = netSize;
            }
        }
        pickedCell->setAvgNetSize(totalNetSize / pickedCellNetList.size());
        pickedCell->setMaxNetSize(maxNetSize);
        pickedCell->setAlpha(0);
        pickedCell->setSortingIndex(pickedCell->getMaxNetSize() + pickedCell->getAlpha() * pickedCell->getAvgNetSize());
        cellArrayBySortingIndex.push_back(pickedCell);
    }
    sort(cellArrayBySortingIndex.begin(), cellArrayBySortingIndex.end(), [](Cell *a, Cell *b)
         { return a->getSortingIndex() < b->getSortingIndex(); });

    // initialize partition with f(x)
    int splitSize = _cellNum / 2;
    double lowerBound = (1 - _bFactor) / 2 * _cellNum, upperBound = (1 + _bFactor) / 2 * _cellNum;
    for (int i = 0; i < _cellNum; ++i)
    {
        Cell *pickedCell = cellArrayBySortingIndex[i];
        if (i < splitSize)
        {
            pickedCell->setPart(0);
            partSize[0]++;
            _unlockNum[0]++;
            vector<int> pickedCellNetList = pickedCell->getNetList();
            for (int j = 0; j < pickedCellNetList.size(); ++j)
            {
                _netArray[pickedCellNetList[j]]->incPartCount(0);
            }
        }
        else
        {
            pickedCell->setPart(1);
            partSize[1]++;
            _unlockNum[1]++;
            vector<int> pickedCellNetList = pickedCell->getNetList();
            for (int j = 0; j < pickedCellNetList.size(); ++j)
            {
                _netArray[pickedCellNetList[j]]->incPartCount(1);
            }
        }
        // update maxPinNum
        if (pickedCell->getPinNum() > maxPinNum)
        {
            maxPinNum = pickedCell->getPinNum();
        }
    }

    // initialize _maxPinNum
    _maxPinNum = maxPinNum;

    // initialize _partSize and _cutSize
    _partSize[0] = partSize[0];
    _partSize[1] = partSize[1];
    for (int i = 0; i < _netNum; ++i)
    {
        if (_netArray[i]->getPartCount(0) > 0 && _netArray[i]->getPartCount(1) > 0)
        {
            _cutSize++;
        }
    }
}

void Partitioner::addNode(Node *targetNode)
{
    // add targetNode to the linkedlist
    Cell *addedCell = _cellArray[targetNode->getId()];
    addedCell->setNode(targetNode);
    map<int, Node *>::iterator it = _bList[addedCell->getPart()].find(addedCell->getGain());

    // if the gain is not in the map, create new
    if (it == _bList[addedCell->getPart()].end())
    {
        _bList[addedCell->getPart()].insert(pair<int, Node *>(addedCell->getGain(), targetNode));
    }
    else
    {
        Node *firstNode = it->second;
        _bList[addedCell->getPart()][addedCell->getGain()] = targetNode;
        targetNode->setNext(firstNode);
        firstNode->setPrev(targetNode);
    }
}

void Partitioner::removeNode(Node *targetNode)
{
    Cell *removedCell = _cellArray[targetNode->getId()];
    Node *prevNode = targetNode->getPrev();
    Node *nextNode = targetNode->getNext();

    // remove targetNode from the linkedlist
    if (prevNode != NULL && nextNode != NULL)
    {
        // if targetNode is not the first node nor the last node
        prevNode->setNext(nextNode);
        nextNode->setPrev(prevNode);
        targetNode->setPrev(NULL);
        targetNode->setNext(NULL);
    }
    else if (prevNode == NULL && nextNode != NULL)
    {
        // if targetNode is the first node
        _bList[removedCell->getPart()][removedCell->getGain()] = nextNode;
        nextNode->setPrev(NULL);
        targetNode->setPrev(NULL);
        targetNode->setNext(NULL);
    }
    else if (prevNode != NULL && nextNode == NULL)
    {
        // if targetNode is the last node
        prevNode->setNext(NULL);
        targetNode->setPrev(NULL);
        targetNode->setNext(NULL);
    }
    else
    {
        // if targetNode is the only node
        _bList[removedCell->getPart()].erase(removedCell->getGain());
        targetNode->setPrev(NULL);
        targetNode->setNext(NULL);
    }
}

void Partitioner::initGain()
{
    // initialize gain of each cell
    for (int i = 0; i < _netNum; ++i)
    {
        Net *pickedNet = _netArray[i];
        vector<int> pickedCellList = pickedNet->getCellList();
        for (int j = 0; j < pickedCellList.size(); ++j)
        {
            Cell *pickedCell = _cellArray[pickedCellList[j]];
            int fromSide = -1, toside = -1;
            if (pickedCell->getPart() == 0)
            {
                fromSide = pickedNet->getPartCount(0);
                toside = pickedNet->getPartCount(1);
            }
            else
            {
                fromSide = pickedNet->getPartCount(1);
                toside = pickedNet->getPartCount(0);
            }
            if (fromSide == 1)
            {
                pickedCell->incGain();
            }
            if (toside == 0)
            {
                pickedCell->decGain();
            }
        }
    }

    // blist initialize
    for (int i = 0; i < _cellNum; ++i)
    {
        this->addNode(_cellArray[i]->getNode());
    }
}

void Partitioner::updateGain()
{
    Cell *movedCell = _cellArray[_maxGainCell->getId()];
    movedCell->lock();
    vector<int> movedCellNetList = movedCell->getNetList();
    int netSize = movedCellNetList.size();

    // update the gain of all free cells in the net
    for (int i = 0; i < netSize; ++i)
    {
        Net *movedCellNet = _netArray[movedCellNetList[i]];
        int fromSide = movedCellNet->getPartCount(movedCell->getPart()), toSide = movedCellNet->getPartCount(!movedCell->getPart());
        if (toSide == 0)
        {
            // if toSide == 0, increment the gains of all free cells in the net
            vector<int> movedCellNetCellList = movedCellNet->getCellList();
            for (int j = 0; j < movedCellNetCellList.size(); ++j)
            {
                Cell *freeCell = _cellArray[movedCellNetCellList[j]];
                if (!freeCell->getLock())
                {
                    this->removeNode(freeCell->getNode());
                    freeCell->incGain();
                    this->addNode(freeCell->getNode());
                }
            }
        }
        else if (toSide == 1)
        {
            // if toSide == 1, decrement the gain of the only free cell at toSide
            vector<int> movedCellNetCellList = movedCellNet->getCellList();
            for (int j = 0; j < movedCellNetCellList.size(); ++j)
            {
                Cell *freeCell = _cellArray[movedCellNetCellList[j]];
                if (!freeCell->getLock() && freeCell->getPart() == !movedCell->getPart())
                {
                    this->removeNode(freeCell->getNode());
                    freeCell->decGain();
                    this->addNode(freeCell->getNode());
                    break;
                }
            }
        }
        fromSide--;
        toSide++;
        if (fromSide == 0)
        {
            // if fromSide == 0, decrement the gains of all free cells in the net
            vector<int> movedCellNetCellList = movedCellNet->getCellList();
            for (int j = 0; j < movedCellNetCellList.size(); ++j)
            {
                Cell *freeCell = _cellArray[movedCellNetCellList[j]];
                if (!freeCell->getLock())
                {
                    this->removeNode(freeCell->getNode());
                    freeCell->decGain();
                    this->addNode(freeCell->getNode());
                }
            }
        }
        else if (fromSide == 1)
        {
            // if fromSide == 1, increment the gain of the only free cell at fromSide
            vector<int> movedCellNetCellList = movedCellNet->getCellList();
            for (int j = 0; j < movedCellNetCellList.size(); ++j)
            {
                Cell *freeCell = _cellArray[movedCellNetCellList[j]];
                if (!freeCell->getLock() && freeCell->getPart() == movedCell->getPart())
                {
                    this->removeNode(freeCell->getNode());
                    freeCell->incGain();
                    this->addNode(freeCell->getNode());
                    break;
                }
            }
        }
    }
}

bool Partitioner::pickMaxGainCell()
{
    // decide which partition to pick
    bool pickedPart, part0Avail, part1Avail;
    double lowerBound = (1 - _bFactor) / 2 * _cellNum, upperBound = (1 + _bFactor) / 2 * _cellNum;

    // check if the partition is available
    part0Avail = !_bList[0].empty() && _partSize[0] - 1 >= lowerBound && _partSize[0] - 1 <= upperBound;
    part1Avail = !_bList[1].empty() && _partSize[1] - 1 >= lowerBound && _partSize[1] - 1 <= upperBound;

    // early stop if reaching the stop constant
    if (_moveNum >= _stopConstant)
    {
        return 0;
    }

    // decide which partition to pick
    if (part0Avail && part1Avail)
    {
        map<int, Node *>::iterator max0It = _bList[0].end(), max1It = _bList[1].end();
        max0It--;
        max1It--;
        pickedPart = max0It->first > max1It->first ? 0 : 1;
    }
    else if (part0Avail)
    {
        pickedPart = 0;
    }
    else if (part1Avail)
    {
        pickedPart = 1;
    }
    else
    {
        return 0;
    }

    // pick the max gain cell
    map<int, Node *>::iterator maxIt = _bList[pickedPart].end();
    maxIt--;
    Node *pickedNode;
    pickedNode = maxIt->second;

    // remove the picked node from the bucket list, therefore no lock node in the bucket list
    this->removeNode(pickedNode);
    _maxGainCell = pickedNode;
    return 1;
}

void Partitioner::moveCell()
{
    // move the max gain cell
    Cell *movedCell = _cellArray[_maxGainCell->getId()];
    bool fromPart = movedCell->getPart();
    movedCell->move();

    // update nets connected to the moved cell
    vector<int> movedCellNetList = movedCell->getNetList();
    for (int i = 0; i < movedCellNetList.size(); ++i)
    {
        Net *movedCellNet = _netArray[movedCellNetList[i]];
        movedCellNet->decPartCount(fromPart);
        movedCellNet->incPartCount(!fromPart);
    }

    // update partition
    _moveStack.push_back(_maxGainCell->getId());
    _accGain += movedCell->getGain();
    ++_moveNum;
    if (_maxAccGain < _accGain)
    {
        _maxAccGain = _accGain;
        _bestMoveNum = _moveNum;
    }
    _unlockNum[fromPart]--;
    --_partSize[fromPart];
    ++_partSize[!fromPart];
}

void Partitioner::toBest()
{
    // back to the best move
    for (int i = _moveNum - 1; i >= _bestMoveNum; --i)
    {
        Cell *restoredCell = _cellArray[_moveStack[i]];
        restoredCell->move();
        for (int j = 0; j < restoredCell->getNetList().size(); ++j)
        {
            Net *restoredCellNet = _netArray[restoredCell->getNetList()[j]];
            restoredCellNet->decPartCount(!restoredCell->getPart());
            restoredCellNet->incPartCount(restoredCell->getPart());
        }
        ++_partSize[restoredCell->getPart()];
        --_partSize[!restoredCell->getPart()];
    }
    _cutSize -= _maxAccGain;
}

void Partitioner::reRunInit()
{
    // reinitialize the partition for next iteration
    _accGain = 0;
    _maxAccGain = 0;
    _moveNum = 0;
    _bestMoveNum = 0;
    _moveStack.clear();

    // unlock all cells
    for (int i = 0; i < _cellNum; ++i)
    {
        Cell *pickedCell = _cellArray[i];
        pickedCell->unlock();
        pickedCell->setGain(0);
        this->removeNode(pickedCell->getNode());
    }

    // initialize blist
    for (int i = 0; i < 2; ++i)
    {
        _bList[i].clear();
    }
    return;
}

void Partitioner::parseInput(fstream &inFile)
{
    string str;
    // Set balance factor
    inFile >> str;
    _bFactor = stod(str);

    // Set up whole circuit
    while (inFile >> str)
    {
        if (str == "NET")
        {
            string netName, cellName, tmpCellName = "";
            inFile >> netName;
            int netId = _netNum;
            _netArray.push_back(new Net(netName));
            _netName2Id[netName] = netId;
            while (inFile >> cellName)
            {
                if (cellName == ";")
                {
                    tmpCellName = "";
                    break;
                }
                else
                {
                    // a newly seen cell
                    if (_cellName2Id.count(cellName) == 0)
                    {
                        int cellId = _cellNum;
                        _cellArray.push_back(new Cell(cellName, 0, cellId));
                        _cellName2Id[cellName] = cellId;
                        _cellArray[cellId]->addNet(netId);
                        _cellArray[cellId]->incPinNum();
                        _netArray[netId]->addCell(cellId);
                        ++_cellNum;
                        tmpCellName = cellName;
                    }
                    // an existed cell
                    else
                    {
                        if (cellName != tmpCellName)
                        {
                            assert(_cellName2Id.count(cellName) == 1);
                            int cellId = _cellName2Id[cellName];
                            _cellArray[cellId]->addNet(netId);
                            _cellArray[cellId]->incPinNum();
                            _netArray[netId]->addCell(cellId);
                            tmpCellName = cellName;
                        }
                    }
                }
            }
            ++_netNum;
        }
    }
    return;
}

void Partitioner::partition()
{
    // initialize partition
    this->initPartition();
    // cout << "Initial cutsize: " << _cutSize << endl;

    // start Fiduccia-Mattheyses algorithm
    while(true)
    {
        this->initGain();
        // set stop constant
        _stopConstant = _iterNum == 0 ? _cellNum * 0.5 : _cellNum * 0.2;
        _iterNum++;

        // start partitioning
        while (this->pickMaxGainCell())
        {
            // report blist for debugging
            // this->reportbList();
            this->updateGain();
            this->moveCell();
        }

        // decide whether to stop partitioning
        if (_maxAccGain > 0)
        {
            // cout << "max accumulated gain: " << _maxAccGain << endl;
            this->toBest();
            this->reRunInit();
        }
        else
        {
            // cout << "max accumulated gain: " << _maxAccGain << endl;
            // cout << "No more improvement, stop partitioning." << endl;
            this->toBest();
            break;
        }
    }
}

void Partitioner::printSummary() const
{
    cout << endl;
    cout << "==================== Summary ====================" << endl;
    cout << " Cutsize: " << _cutSize << endl;
    cout << " Total cell number: " << _cellNum << endl;
    cout << " Total net number:  " << _netNum << endl;
    cout << " Cell Number of partition A: " << _partSize[0] << endl;
    cout << " Cell Number of partition B: " << _partSize[1] << endl;
    cout << "=================================================" << endl;
    cout << endl;
    return;
}

void Partitioner::reportNet() const
{
    cout << "Number of nets: " << _netNum << endl;
    for (size_t i = 0, end_i = _netArray.size(); i < end_i; ++i)
    {
        cout << setw(8) << _netArray[i]->getName() << ": ";
        vector<int> cellList = _netArray[i]->getCellList();
        for (size_t j = 0, end_j = cellList.size(); j < end_j; ++j)
        {
            cout << setw(8) << _cellArray[cellList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::reportCell() const
{
    cout << "Number of cells: " << _cellNum << endl;
    for (size_t i = 0, end_i = _cellArray.size(); i < end_i; ++i)
    {
        cout << setw(8) << _cellArray[i]->getName() << ": ";
        vector<int> netList = _cellArray[i]->getNetList();
        for (size_t j = 0, end_j = netList.size(); j < end_j; ++j)
        {
            cout << setw(8) << _netArray[netList[j]]->getName() << " ";
        }
        cout << endl;
    }
    return;
}

void Partitioner::writeResult(fstream &outFile)
{
    stringstream buff;
    buff << _cutSize;
    outFile << "Cutsize = " << buff.str() << '\n';
    buff.str("");
    buff << _partSize[0];
    outFile << "G1 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i)
    {
        if (_cellArray[i]->getPart() == 0)
        {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    buff.str("");
    buff << _partSize[1];
    outFile << "G2 " << buff.str() << '\n';
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i)
    {
        if (_cellArray[i]->getPart() == 1)
        {
            outFile << _cellArray[i]->getName() << " ";
        }
    }
    outFile << ";\n";
    return;
}

void Partitioner::clear()
{
    for (size_t i = 0, end = _cellArray.size(); i < end; ++i)
    {
        delete _cellArray[i];
    }
    for (size_t i = 0, end = _netArray.size(); i < end; ++i)
    {
        delete _netArray[i];
    }
    return;
}

void Partitioner::reportbList()
{
    cout << "In the bList[0]" << endl;
    for (map<int, Node *>::iterator iter = _bList[0].begin(); iter != _bList[0].end(); iter++)
    {
        if (iter->second != NULL)
        {
            Node *sameGainNode = iter->second;
            cout << "Gain value = " << iter->first << endl;
            while (sameGainNode != NULL)
            {
                cout << "The Node and Cell with the same Gain has name: " << _cellArray[sameGainNode->getId()]->getName() << endl;
                sameGainNode = sameGainNode->getNext();
            }
        }
    }
    cout << endl;
    cout << "In the bList[1]" << endl;
    for (map<int, Node *>::iterator iter = _bList[1].begin(); iter != _bList[1].end(); iter++)
    {
        if (iter->second != NULL)
        {
            Node *sameGainNode = iter->second;
            cout << "Gain value = " << iter->first << endl;
            while (sameGainNode != NULL)
            {
                cout << "The Node and Cell with the same Gain has name: " << _cellArray[sameGainNode->getId()]->getName() << endl;
                sameGainNode = sameGainNode->getNext();
            }
        }
    }
    cout << endl;
    cout << endl;
}