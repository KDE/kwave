#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../src/interpolation.h"
#include "../../../src/menuitem.h"
#include "../../../src/timeoperation.h"
#include "../../../src/curve.h"
#include "../../../src/parser.h"         

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="amplifyfree";

//**********************************************************
int operation (TimeOperation *operation)
{
  int *sample=operation->getSample();
  int len=operation->getLength();
  KwaveParser parse (operation->getCommand());
  Interpolation interpolation (0);

  Curve *curve=new Curve (parse.getFirstParam());

    if (curve)
      {
	double *y=interpolation.getInterpolation (curve,len);

	if (y)
	  for (int i=0;i<len;i++)
	    sample[i]=(int) (sample[i]*y[i]),
	      operation->setCounter(i);
	else
	  {
	    double x;
	    interpolation.prepareInterpolation (curve);
	    for (int i=0;i<len;i++)
	      {
		x=interpolation.getSingleInterpolation(((double)i)/len);
		sample[i]=(int) (sample[i]*x);
		operation->setCounter(i);
	      }
	  }
      }


  operation->done();
  return 0;
}
//**********************************************************













