#include <math.h>
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
const char *name="quantize";
//**********************************************************
int operation (TimeOperation *operation)
{
  int *sample=operation->getSample();
  int len=operation->getLength();
  KwaveParser parser (operation->getCommand());
  int bits=parser.toInt();

  double a;
  for (int j=0;j<len;j++)
    {
      a=(double)(sample[j]+(1<<23))/(1<<24);
      a=floor(a*bits+.5);
      a/=bits;
      sample[j]=(int)((a-.5)*((1<<24)-1));   //24 because of double range (no more signed)
      operation->setCounter(j);
    }

  operation->done();
  return 0;
}
//**********************************************************













