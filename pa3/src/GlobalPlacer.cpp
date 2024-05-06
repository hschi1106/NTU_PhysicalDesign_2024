#include "GlobalPlacer.h"

#include <cstdio>
#include <vector>
#include <map>

#include "ObjectiveFunction.h"
#include "Optimizer.h"
#include "Point.h"

GlobalPlacer::GlobalPlacer(Placement &placement)
    : _placement(placement)
{
}

void GlobalPlacer::place()
{
    int moduleNum = _placement.numModules();
    std::vector<Point2<double>> t(moduleNum);          // Optimization variables (in this example, there is only one t)
    ObjectiveFunction foo(_placement);                 // Objective function
    const double kAlpha = foo.getBinSize() * 8;        // Constant step size
    SimpleConjugateGradient optimizer(foo, t, kAlpha); // Optimizer

    // traverse all nets to determine the initial partition
    vector<int> initPlacePart(_placement.numModules(), 0);
    int netNum = _placement.numNets(), countAtOne = 0;
    for (int i = 0; i < netNum; ++i)
    {
        Net &net = _placement.net(i);
        int pinNum = net.numPins();
        // traverse all pins in the net
        for (int j = 0; j < pinNum; ++j)
        {
            Pin &pin = net.pin(j);
            int moduleId = pin.moduleId();
            if (initPlacePart[moduleId] == 0)
            {
                countAtOne++;
            }
            initPlacePart[moduleId] = 1;
        }

        // if the number of modules in partition 1 is greater than half of the total number of modules, then break
        if (countAtOne >= moduleNum / 2)
        {
            break;
        }
    }

    // Set initial positions
    for (int i = 0; i < moduleNum; ++i)
    {
        // Initialize the position of the module by its partition
        double midX = (_placement.boundryLeft() + _placement.boundryRight()) / 2;
        double midY = (_placement.boundryBottom() + _placement.boundryTop()) / 2;
        if (initPlacePart[i] == 0)
        {
            t[i].x = midX + double(rand()) / RAND_MAX * 5 - 7.5;
            t[i].y = midY + double(rand()) / RAND_MAX * 5 - 7.5;
        }
        else
        {
            t[i].x = midX + double(rand()) / RAND_MAX * 5 + 2.5;
            t[i].y = midY + double(rand()) / RAND_MAX * 5 + 2.5;
        }
    }

    // Initialize the optimizer
    optimizer.Initialize();

    // Perform optimization, the termination condition is that the number of iterations reaches 100
    // TODO: You may need to change the termination condition, which is determined by the overflow ratio.
    int iterNum = 0;
    double lastOverflowRatio = 100; // The overflow ratio of the last 100 iteration
    while (true)
    {
        iterNum++;
        optimizer.Step();
        // cout << "iter = " << iterNum << ", f = " << foo(t) << " , overflow ratio = " << foo.getOverflowRatio() << " , gamma = " << foo.getGamma() << endl;

        // Termination condition
        if (foo.getOverflowRatio() <= 0.015 || iterNum >= 3000)
        {
            break;
        }

        // If the overflow ratio does not decrease, the optimization will be terminated
        if (iterNum % 100 == 0 && iterNum > 200)
        {
            if (lastOverflowRatio - foo.getOverflowRatio() < 0.001 || lastOverflowRatio < foo.getOverflowRatio())
            {
                break;
            }
            lastOverflowRatio = foo.getOverflowRatio();
        }
    }

    ////////////////////////////////////////////////////////////////////
    // Global placement algorithm
    ////////////////////////////////////////////////////////////////////

    // TODO: Implement your global placement algorithm here.
    const size_t num_modules = _placement.numModules(); // You may modify this line.
    std::vector<Point2<double>> positions(num_modules); // Optimization variables (positions of modules). You may modify this line.

    // place modules and deal with out of bound modules
    for (size_t i = 0; i < num_modules; ++i)
    {
        if (t[i].x < _placement.boundryLeft())
        {
            t[i].x = _placement.boundryLeft();
        }
        else if (t[i].x > _placement.boundryRight() - _placement.module(i).width())
        {
            t[i].x = _placement.boundryRight() - _placement.module(i).width();
        }

        if (t[i].y < _placement.boundryBottom())
        {
            t[i].y = _placement.boundryBottom();
        }
        else if (t[i].y > _placement.boundryTop() - _placement.module(i).height())
        {
            t[i].y = _placement.boundryTop() - _placement.module(i).height();
        }

        positions[i].x = t[i].x;
        positions[i].y = t[i].y;
    }

    ////////////////////////////////////////////////////////////////////
    // Write the placement result into the database. (You may modify this part.)
    ////////////////////////////////////////////////////////////////////
    for (size_t i = 0; i < num_modules; i++)
    {
        _placement.module(i).setPosition(positions[i].x, positions[i].y);
    }
}

void GlobalPlacer::plotPlacementResult(const string outfilename, bool isPrompt)
{
    ofstream outfile(outfilename.c_str(), ios::out);
    outfile << " " << endl;
    outfile << "set title \"wirelength = " << _placement.computeHpwl() << "\"" << endl;
    outfile << "set size ratio 1" << endl;
    outfile << "set nokey" << endl
            << endl;
    outfile << "plot[:][:] '-' w l lt 3 lw 2, '-' w l lt 1" << endl
            << endl;
    outfile << "# bounding box" << endl;
    plotBoxPLT(outfile, _placement.boundryLeft(), _placement.boundryBottom(), _placement.boundryRight(), _placement.boundryTop());
    outfile << "EOF" << endl;
    outfile << "# modules" << endl
            << "0.00, 0.00" << endl
            << endl;
    for (size_t i = 0; i < _placement.numModules(); ++i)
    {
        Module &module = _placement.module(i);
        plotBoxPLT(outfile, module.x(), module.y(), module.x() + module.width(), module.y() + module.height());
    }
    outfile << "EOF" << endl;
    outfile << "pause -1 'Press any key to close.'" << endl;
    outfile.close();

    if (isPrompt)
    {
        char cmd[200];
        sprintf(cmd, "gnuplot %s", outfilename.c_str());
        if (!system(cmd))
        {
            cout << "Fail to execute: \"" << cmd << "\"." << endl;
        }
    }
}

void GlobalPlacer::plotBoxPLT(ofstream &stream, double x1, double y1, double x2, double y2)
{
    stream << x1 << ", " << y1 << endl
           << x2 << ", " << y1 << endl
           << x2 << ", " << y2 << endl
           << x1 << ", " << y2 << endl
           << x1 << ", " << y1 << endl
           << endl;
}
