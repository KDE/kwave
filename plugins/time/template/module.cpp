#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../lib/interpolation.h"
#include "../../../lib/menuitem.h"
#include "../../../lib/timeoperation.h"
#include "../../../lib/curve.h"
#include "../../../lib/parser.h"         

const char *version="";
const char *author="";
const char *name="";
//**********************************************************
int operation (TimeOperation *operation)
{
  int *sample=operation->getSample();
  int len=operation->getLength();

  operation->done();
  return 0;
}
//**********************************************************













