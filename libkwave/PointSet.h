#ifndef _KWAVE_POINTSET_H_
#define _KWAVE_POINTSET_H_ 1

#include <qlist.h>

struct Point
{
  double x;
  double y;
};

#define PointSet QList<Point>

//class PointSet //to replace QList<Cpoint>, so linking to qt is not neccesary
//{
//};

#endif
