#include <stdio.h>
#include <stdlib.h>
#include "../../../src/menuitem.h"
#include "../../../src/timeoperation.h"

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="zero";

//**********************************************************
int operation (TimeOperation *operation)
{
  int *sample=operation->getSample();
  int len=operation->getLength();

  for (int i=0;i<len;sample[i++]=0);

  operation->done();
  return 0;
}
//**********************************************************













