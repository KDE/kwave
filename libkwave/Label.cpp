#include <stdio.h>
#include "markers.h"
#include "parser.h"
#include "kwavestring.h"
#include "color.h"
#include "math.h"
#include "globals.h"

extern Global globals;
//****************************************************************************
Marker::Marker (const char *params)
{
  KwaveParser parser (params);

  this->pos=parser.toDouble ();
  const char *type=parser.getNextParam ();

  MarkerType *act;
  this->type=globals.markertypes.first();  
  for (act=globals.markertypes.first();act;act=globals.markertypes.next())
    {
      if (strcmp (act->name,type)==0)
	{
	  this->type=act;
	  break;
	}
    }

    if (this->type->named)
      {
	const char *name=parser.getNextParam ();
	this->name=duplicateString(name);
      }
  comstr=0;
}
//****************************************************************************
Marker::Marker (double pos,MarkerType *type,const char *name)
{
  this->pos=pos;
  this->type=type;

  if (name)
    this->name=duplicateString (name);
  else
    this->name=0;
  comstr=0;
}
//****************************************************************************
const char * Marker::getCommand ()
{
  if (comstr) deleteString (comstr);
  char buf[64];

  sprintf (buf,"%f",pos);

  if (type->named)
  comstr=catString ("label (",buf,",",type->name,",",name,")");
  else
  comstr=catString ("label (",buf,",",type->name,")");
  return comstr;
}
//****************************************************************************
void Marker::setName (const char *name)
{
  this->name=duplicateString (name);
}
//****************************************************************************
Marker::~Marker ()
{
  if (name) deleteString (name);
  if (comstr) deleteString (comstr);
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
  double res=(c->pos-d->pos);
  if (res>0) return (int) ceil (c->pos-d->pos);
  else return (int) floor (c->pos-d->pos);
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
