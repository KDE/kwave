#ifndef _INTERPOLATION_H_
#define _INTERPOLATION_H_ 1

#include "curvewidget.h"

#define INTPOL_LINEAR 0
#define INTPOL_SPLINE 1
#define INTPOL_NPOLYNOMIAL 2
#define INTPOL_POLYNOMIAL3 3
#define INTPOL_POLYNOMIAL5 4
#define INTPOL_POLYNOMIAL7 5
#define INTPOL_SAH         6

class Interpolation
{
 public:

  Interpolation(int type);
  ~Interpolation();
  void   incUsage ();
  void   decUsage ();
  int    getUsage ();
  double *getInterpolation        (QList<CPoint> *points,int);
  double *getLimitedInterpolation (QList<CPoint> *points,int);
  int    prepareInterpolation     (QList<CPoint> *);
  double getSingleLimitedInterpolation (double pos);
  double getSingleInterpolation (double pos);
  const char** getTypes ();
  int    getCount ();

 private:
  QList<CPoint> *points; // List of points to be interpolated
  double *y_out;         // arrays
  double *x,*y,*der;     // used for temporary purposes
  int type;              // type of interpolation
  int count;             // number of points
  int usagecount;        // number of tasks using this interpolation
};


#endif /*interpolation.h*/

