#ifndef _KWAVE_CURVE_H_
#define _KWAVE_CURVE_H_ 1

#include "pointset.h"

class Interpolation;

class Curve
{
 public:
  Curve  (const char *);
  Curve  ();
  ~Curve ();

         void        firstHalf            ();
         void        secondHalf           ();
         void        deletePoint          (Point *,bool);
         void        deleteSecondPoint    ();
	 void        HFlip                ();
	 void        VFlip                ();
	 void        scaleFit             (int range=1024);
	 void        addPoint             (double,double);
	 void        addPoint             (Point *);
	 void        append               (double,double);
  inline void        append               (Point *p) {points->append (p);};

	 Point *      findPoint            (double x,double y,double tol=.05);
  inline Point *      first                ()      {return points->first();};
  inline unsigned int count                ()      {return points->count();};
  inline Point *      at                   (int x) {return points->at(x);};
  inline Point *      last                 ()      {return points->last();};
	 Point *      previous             (Point *);
	 Point *      next                 (Point *);

         const char* getCommand           ();
         const char* getInterpolationType ();
         void        setInterpolationType (const char *);
       	 double *    getInterpolation     (int width);
 private:
  PointSet*      points;
  Interpolation* interpolation;
  int            interpolationtype;
  char*          comstr;
};
#endif



