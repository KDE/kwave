#include <stdlib.h>
#include <string.h>
#include "timeoperation.h"
#include "kwavesignal.h"

//**********************************************************************
TimeOperation::TimeOperation (KwaveSignal *signal,const char * command,int begin,int len)
{
  this->begin=begin;
  this->len=len;
  this->signal=signal;
  this->command=strdup (command);
}
//**********************************************************************
int *TimeOperation::getSample ()
{
  return &(signal->getSample()[begin]);
}
//**********************************************************************
TimeOperation::~TimeOperation ()
{
  if (command) free (command);
}
