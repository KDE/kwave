#include "markers.h"
#include "parser.h"
#include "color.h"
//****************************************************************************
Marker::Marker ()
{
  name=0;
}
//****************************************************************************
Marker::~Marker ()
{
  if (name) delete name;
}
//****************************************************************************
MarkerList::MarkerList ()
{
}
//****************************************************************************
MarkerList::~MarkerList ()
{
  clear();
}
//****************************************************************************
int MarkerList::compareItems (GCI a,GCI b)
{
  Marker *c = (Marker *)a;
  Marker *d = (Marker *)b;
  return c->pos-d->pos;
}
//****************************************************************************
MarkerType::MarkerType ()
{
  name=0;
  color=0;
}
//****************************************************************************
MarkerType::MarkerType (const char *command)
{
  KwaveParser parser (command);

  name=new QString (parser.getFirstParam());
  named=(strcmp(parser.getNextParam(),"true")==0);
  color=new Color (parser.getNextParam());
}
//****************************************************************************
MarkerType::~MarkerType ()
{
  if (name) delete name;
  if (color)delete color;
}
