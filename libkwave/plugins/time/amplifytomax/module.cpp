#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../src/interpolation.h"
#include "../../../src/menuitem.h"
#include "../../../src/timeoperation.h"
#include "../../../src/curve.h"
#include "../../../src/parser.h"         

#define PROGRESS_SIZE 512*3*4

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="amplify";
//**********************************************************
int operation (TimeOperation *operation)
{
  int *sample=operation->getSample();
  int len=operation->getLength();

  int j;
	
  int max=0;
  for (int i=0;i<len;i++)
    if (max<abs(sample[i])) max=abs(sample[i]);

  double mult=((double)max)/((1<<23)-1);

  for (int i=0;i<len;)
    {
      if (i<len-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
      else j=len;

      for (;i<j;i++)
	sample[i]=(int) (sample[i]*mult);

      operation->setCounter(i);
    }

  operation->done();
  return 0;
}
//**********************************************************













