#include <stdio.h>
#include <stdlib.h>
#include "../../../src/menuitem.h"
#include "../../../src/timeoperation.h"

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="noise";

//**********************************************************
int operation (TimeOperation *operation)
{
  int *sample=operation->getSample();
  int len=operation->getLength();

  for (int i=0;i<len;i++)
    {
      sample[i]=(int)((drand48()*(1<<24)-1)-(1<<23));
      operation->setCounter (i);
    }

  operation->done();
  return 0;
}
//**********************************************************













