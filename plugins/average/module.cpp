#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../src/interpolation.h"
#include "../../../src/menuitem.h"
#include "../../../src/timeoperation.h"
#include "../../../src/curve.h"
#include "../../../src/parser.h"         

#define PROGRESS_SIZE 2*2*3*3*5*5

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="delay";

//**********************************************************
int operation (TimeOperation *operation)
{
  int *sample=operation->getSample();
  int len=operation->getLength();
  bool fir=true;
  int  delay=0;
  int  ampl=100;

  if (fir)
    {
      int j;

      for (int i=delay;i<len;)
	{
	  if (i<len-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
	  else j=len;

	  for (;i<j;i++)
	    sample[i]=(sample[i]+(sample[i-delay]*ampl/100))/2;

	  operation->setCounter(i);
	}
    }
  else
    {
      int j;

      for (int i=len-1;i>=0;)
	{
	  if (i>PROGRESS_SIZE) j=i-PROGRESS_SIZE;
	  else j=0;

	  for (;i>=j;i--)
	    sample[i]=(sample[i]+(sample[i-delay]*ampl/100))/2;

	  operation->setCounter(len-i);
	}
    }
  operation->done();
  return 0;
}
//**********************************************************













