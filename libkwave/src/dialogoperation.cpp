#include <stdlib.h>
#include <string.h>
#include "dialogoperation.h"

#define CANCEL "&Cancel"
#define OK     "&Ok"
//**********************************************************************
DialogOperation::DialogOperation (int rate,bool modal)
{
  this->rate=rate;
  this->modal=modal;
  this->name=0;
  this->length=0;
  this->globals=0;
  this->channels=1;
}
//**********************************************************************
DialogOperation::DialogOperation (Global *g,int length,int rate,int channels,bool modal)
{
  this->globals=g;
  this->length=length;
  this->modal=modal;
  this->rate=rate;
  this->channels=1;
  this->name=0;
}
//**********************************************************************
DialogOperation::DialogOperation (const char * name,bool modal)
{
  this->globals=0;
  this->name=strdup (name);
  this->modal=modal;
  this->length=0;
  this->rate=0;
  this->channels=1;
}
//**********************************************************************
DialogOperation::~DialogOperation ()
{
  if (name) free (name);
}



