#include "GlobalPlacer.h"

#include <cstdio>
#include <vector>
#include <map>
#include <climits>

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
    std::vector<Point2<double>> t(moduleNum);     // Optimization variables (in this example, there is only one t)
    ObjectiveFunction foo(_placement);            // Objective function
    const double kAlpha = foo.getBinSize() * 0.5; // Constant step size
    cout << "step size: " << kAlpha << endl;
    SimpleConjugateGradient optimizer(foo, t, kAlpha); // Optimizer

    // Set initial positions
    double outlineWidth = _placement.boundryRight() - _placement.boundryLeft();
    double outlineHeight = _placement.boundryTop() - _placement.boundryBottom();
    for (int i = 0; i < moduleNum; ++i)
    {
        // Initialize the position of the fixed module
        if (_placement.module(i).isFixed())
        {
            t[i].x = _placement.module(i).x() + _placement.module(i).width() / 2;
            t[i].y = _placement.module(i).y() + _placement.module(i).height() / 2;
            continue;
        }

        // Randomly place the module in the center of the outline
        double midX = (_placement.boundryLeft() + _placement.boundryRight()) / 2;
        double midY = (_placement.boundryBottom() + _placement.boundryTop()) / 2;
        t[i].x = midX + double(rand()) / RAND_MAX * outlineWidth * 0.1 - outlineWidth * 0.05;
        t[i].y = midY + double(rand()) / RAND_MAX * outlineHeight * 0.1 - outlineHeight * 0.05;
    }

    // Initialize the optimizer
    optimizer.Initialize();

    // Perform optimization, the termination condition is that the number of iterations reaches 100
    // TODO: You may need to change the termination condition, which is determined by the overflow ratio.
    int iterNum = 0;
    double lastObjectiveFunctionValue = foo(t); // The objective function value of the last iteration
    double lastOverflowRatio = INT_MAX;
    int canBeTerminated = INT_MAX;
    while (true)
    {
        iterNum++;
        optimizer.Step();
        double objectiveFunctionValue = foo(t);
        cout << "iter = " << iterNum << ", f = " << objectiveFunctionValue << " , overflow ratio = " << foo.getOverflowRatio() << " , gamma = " << foo.getGamma() << endl;

        if (lastObjectiveFunctionValue <= objectiveFunctionValue && foo.getOverflowRatio() > -0.1)
        {
            foo.increaseLambda();
            lastObjectiveFunctionValue = foo(t);
            // cout << "increase lambda to: " << foo.getLambda() << endl;
            if (optimizer.getAlpha() > foo.getBinSize() * 0.1)
            {
                optimizer.updateAlpha();
                // cout << "update step size to: " << optimizer.getAlpha() << endl;
            }
        }
        else
        {
            lastObjectiveFunctionValue = objectiveFunctionValue;
        }

        // Terminate
        if (iterNum == canBeTerminated + 200)
        {
            break;
        }

        // Termination condition
        if (foo.getOverflowRatio() <= -0.1)
        {
            // break;
            if (canBeTerminated == INT_MAX)
            {
                canBeTerminated = iterNum;
            }
        }

        if (iterNum % 500 == 0)
        {
            if (lastOverflowRatio - foo.getOverflowRatio() > 0.05)
            {
                lastOverflowRatio = foo.getOverflowRatio();
            }
            else if (foo.getOverflowRatio() < 0.01)
            {
                // break;
                if (canBeTerminated == INT_MAX)
                {
                    canBeTerminated = iterNum;
                }
            }
        }

        // randomly place out of bound modules, and place the fixed modules back to their original positions
        double outlineWidth = _placement.boundryRight() - _placement.boundryLeft();
        double outlineHeight = _placement.boundryTop() - _placement.boundryBottom();
        for (int i = 0; i < moduleNum; ++i)
        {
            double moduleWidth = _placement.module(i).width();
            double moduleHeight = _placement.module(i).height();
            if (_placement.module(i).isFixed())
            {
                t[i].x = _placement.module(i).x() + moduleWidth / 2;
                t[i].y = _placement.module(i).y() + moduleHeight / 2;
                continue;
            }

            // out of left bound
            if (t[i].x - moduleWidth / 2 < _placement.boundryLeft())
            {
                t[i].x = _placement.boundryLeft() + moduleWidth / 2 + double(rand()) / RAND_MAX * outlineWidth * 0.001;
            }
            // out of right bound
            else if (t[i].x + moduleWidth / 2 > _placement.boundryRight())
            {
                t[i].x = _placement.boundryRight() - moduleWidth / 2 - double(rand()) / RAND_MAX * outlineWidth * 0.001;
            }

            // out of bottom bound
            if (t[i].y - moduleHeight / 2 < _placement.boundryBottom())
            {
                t[i].y = _placement.boundryBottom() + moduleHeight / 2 + double(rand()) / RAND_MAX * outlineHeight * 0.001;
            }
            // out of top bound
            else if (t[i].y + moduleHeight / 2 > _placement.boundryTop())
            {
                t[i].y = _placement.boundryTop() - moduleHeight / 2 - double(rand()) / RAND_MAX * outlineHeight * 0.001;
            }
        }
    }

    cout << "total iterations: " << iterNum << endl;

    ////////////////////////////////////////////////////////////////////
    // Global placement algorithm
    ////////////////////////////////////////////////////////////////////

    // TODO: Implement your global placement algorithm here.
    const size_t num_modules = _placement.numModules(); // You may modify this line.
    std::vector<Point2<double>> positions(num_modules); // Optimization variables (positions of modules). You may modify this line.

    // place modules and deal with out of bound modules
    for (size_t i = 0; i < num_modules; ++i)
    {
        Point2<double> leftBottomPosition;
        leftBottomPosition.x = t[i].x - _placement.module(i).width() / 2;
        leftBottomPosition.y = t[i].y - _placement.module(i).height() / 2;
        if (leftBottomPosition.x < _placement.boundryLeft())
        {
            leftBottomPosition.x = _placement.boundryLeft();
        }
        else if (leftBottomPosition.x > _placement.boundryRight() - _placement.module(i).width())
        {
            leftBottomPosition.x = _placement.boundryRight() - _placement.module(i).width();
        }

        if (leftBottomPosition.y < _placement.boundryBottom())
        {
            leftBottomPosition.y = _placement.boundryBottom();
        }
        else if (leftBottomPosition.y > _placement.boundryTop() - _placement.module(i).height())
        {
            leftBottomPosition.y = _placement.boundryTop() - _placement.module(i).height();
        }

        positions[i] = leftBottomPosition;
    }

    ////////////////////////////////////////////////////////////////////
    // Write the placement result into the database. (You may modify this part.)
    ////////////////////////////////////////////////////////////////////
    for (size_t i = 0; i < num_modules; i++)
    {
        if (!_placement.module(i).isFixed())
        {
            _placement.module(i).setPosition(positions[i].x, positions[i].y);
        }
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
