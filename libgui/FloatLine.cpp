#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libkwave/String.h>
#include "FloatLine.h"

extern QString mstotime (int ms);
extern char*   mstotimec (int ms);
//**********************************************************
FloatLine::FloatLine (QWidget *parent,double value):KRestrictedLine (parent)
{
  setValue (value);
  setValidChars ("-0123456789.E");
  digits=1;
}
//**********************************************************
void FloatLine::setValue (double value)
{
  char buf[64];
  char conv[32];
  
  sprintf (conv,"%%.%df",digits);
  sprintf (buf,conv,value);
  setText (buf);
}
//**********************************************************
double FloatLine::value ()
{
  return strtod (text(),0); 
}
//**********************************************************
FloatLine::~FloatLine ()
{
}
