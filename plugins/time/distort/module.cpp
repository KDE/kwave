#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../lib/interpolation.h"
#include "../../../lib/menuitem.h"
#include "../../../lib/timeoperation.h"
#include "../../../lib/curve.h"
#include "../../../lib/parser.h"

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="distort";

//**********************************************************
int operation (TimeOperation *operation)
{
  int *sample=operation->getSample();
  int len=operation->getLength();
  KwaveParser parse (operation->getCommand());
  Interpolation interpolation(0);

  char *type=strdup(parse.getFirstParam());
  char *type2=strdup(parse.getNextParam());
  printf ("%s\n%s\n",type,type2);
  Curve *curve=new Curve (type2);

  if (curve)
    {
      interpolation.prepareInterpolation (curve);

      int x;
      double oldy,y;

      if (strcmp(type,"upper")==0) 
	for (int i=0;i<len;i++)
	  {
	    x=sample[i];

	    if (x>0)
	      {
		oldy=((double) abs(x))/((1<<23)-1);
		y=interpolation.getSingleInterpolation(oldy);

		sample[i]=(int) (y*((1<<23)-1));
	      }
	    operation->setCounter (i);
	  }
      if (strcmp(type,"lower")==0) 
	for (int i=0;i<len;i++)
	  {
	    x=sample[i];
	
	    if (x<=0)
	      {
		oldy=((double) abs(x))/((1<<23)-1);
		y=interpolation.getSingleInterpolation(oldy);

		sample[i]=-(int) (y*((1<<23)-1));
	      }
	    operation->setCounter (i);
	  }

      if (strcmp(type,"symmetric")==0) 
	for (int i=0;i<len;i++)
	  {
	    x=sample[i];
	    oldy=((double) abs(x))/((1<<23)-1);
	    y=interpolation.getSingleInterpolation(oldy);
	
	    if (x>0)
	      sample[i]=(int) (y*((1<<23)-1));
	    else
	      sample[i]=-(int) (y*((1<<23)-1));
	    operation->setCounter (i);
	  }
    }

  free (type);

  operation->done();
  return 0;
}
//**********************************************************













