#include "markers.h"
#include "parser.h"
#include "kwavestring.h"
#include "color.h"
//****************************************************************************
Marker::Marker (int pos,MarkerType *type,const char *name)
{
  this->pos=pos;
  this->type=type;
  if (name) this->name=duplicateString (name);
  else name=0;
}
//****************************************************************************
Marker::~Marker ()
{
  if (name) deleteString (name);
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

  name=duplicateString (parser.getFirstParam());
  named=(strcmp(parser.getNextParam(),"true")==0);
  color=new Color (parser.getNextParam());
  comstr=0;
}
//****************************************************************************
const char *MarkerType::getCommand ()
{
  if (comstr) deleteString (comstr);
  comstr=catString ("newlabeltype (",
		    name,
		    ",",
		    named?"true":"false",
		    ",",
  		    color->getCommand(),
		    ")"
		    );

  return comstr;
}
//****************************************************************************
MarkerType::~MarkerType ()
{
  if (name)  delete name;
  if (color) delete color;
  if (comstr) deleteString (comstr);
}
