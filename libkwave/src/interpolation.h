#ifndef _KWAVE_INTERPOLATION_H_
#define _KWAVE_INTERPOLATION_H_ 1

#include "pointset.h"

#define INTPOL_LINEAR 0
#define INTPOL_SPLINE 1
#define INTPOL_NPOLYNOMIAL 2
#define INTPOL_POLYNOMIAL3 3
#define INTPOL_POLYNOMIAL5 4
#define INTPOL_POLYNOMIAL7 5
#define INTPOL_SAH         6

class Curve;

class Interpolation
{
 public:

         Interpolation                 (int type=0);
         ~Interpolation                ();
  void   incUsage                      ();
  void   decUsage                      ();
  int    getUsage                      ();
  double *getInterpolation             (Curve *points,int);
  double *getLimitedInterpolation      (Curve *points,int);
  int    prepareInterpolation          (Curve *);
  double getSingleLimitedInterpolation (double pos);
  double getSingleInterpolation        (double pos);
  static const char** getTypes         ();
  //  int    getCount                      ();
  inline void setType (int t) {type=t;};

 private:
  Curve     *points;        // List of points to be interpolated
  double    *y_out;         // arrays
  double    *x,*y,*der;     // used for temporary purposes
  int       type;           // type of interpolation
  int       count;          // number of points
  int       usagecount;     // number of tasks using this interpolation
};
#endif /*interpolation.h*/

