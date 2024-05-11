#define _GLIBCXX_USE_CXX11_ABI 0  // Align the ABI version to avoid compatibility issues with `Placment.h`
#ifndef GLOBALPLACER_H
#define GLOBALPLACER_H

#include "Placement.h"
#include "Point.h"
#include <cstdlib>
#include <cstdio>
#include <fstream>

class GlobalPlacer 
{
public:
    GlobalPlacer(Placement &placement);
	void place();
    void plotPlacementResult( const string outfilename, bool isPrompt = false );

private:
    Placement& _placement;
    void plotBoxPLT( ofstream& stream, double x1, double y1, double x2, double y2 );
    void writeGlobalBest(vector<Point2<double>> &positions, vector<Point2<double>> &t);

};

#endif // GLOBALPLACER_H
