#include "functions.h"


double rect (double param)
{
  double div=param/(2*M_PI);
  param=param-(floor(div)*(2*M_PI));
  if (param>M_PI) return -1;
  return 1;
}

double sin3 (double param)
{
  double y=sin(param);
  return y*y*y;
}

double saw (double param)
{
  double div=param/(2*M_PI);
  param-=(floor(div)*(2*M_PI));
  param/=M_PI;
  param-=1;
  return param;
}

double sawinv (double param)
{
  double div=param/(2*M_PI);
  param-=(floor(div)*(2*M_PI));
  param=2*M_PI-param;
  param/=M_PI;
  param-=1;
  return param;
}

double tri (double param)
{
  param+=M_PI/2;
  double div=param/(2*M_PI);
  param-=(floor(div)*(2*M_PI));
  if (param>M_PI) return (((param-M_PI)/M_PI)*2)-1;
  return (((M_PI-param)/M_PI)*2)-1;
}

static const char *periodicnames[]={"Sinus","Rectangular","Sawtooth","inverse Sawtooth","Triangular","cubic Sinus",0};
static const double (*periodicfunctions[])(double)={sin,rect,saw,sawinv,tri,sin3};

Functions::Functions ()
{
}
Functions::~Functions ()
{
}
const char **Functions::getTypes ()
{
  return periodicnames;
}
void *Functions::getFunc (int i)
{
  return periodicfunctions[i];
}











