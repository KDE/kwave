#ifndef _POINTSET_H_
#define _POINTSET_H_

#include <qlist.h>

struct Point {
    double x;
    double y;
};

typedef QList<Point> PointSet;

#endif
