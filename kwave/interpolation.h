#ifndef _INTERPOLATION_H_
#define _INTERPOLATION_H_ 1

#include "curvewidget.h"

#define LINEAR 0
#define SPLINE 1
#define NPOLYNOMIAL 2
#define POLYNOMIAL3 3
#define POLYNOMIAL5 4
#define POLYNOMIAL7 5
#define SAH         6

class Interpolation
{
 public:

  Interpolation(int type);
  ~Interpolation();
  double *getInterpolation    (QList<CPoint> *points,int);
  int    prepareInterpolation (QList<CPoint> *);
  double getSingleInterpolation  (double pos);
  const char** getTypes ();
  int    getCount ();

 private:
  QList<CPoint> *points;
  double *y_out;
  double *x,*y,*der;
  int type;
  int count;
};


#endif /*interpolation.h*/

