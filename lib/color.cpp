#include <stdio.h>
#include <stdlib.h>
#include "color.h"
#include "parser.h"
#include "kwavestring.h"

//****************************************************************************
Color::Color ():QColor ()
{
  comstr=0;
}
//****************************************************************************
Color::Color (const char *command):QColor ()
{
  KwaveParser parser(command);
  comstr=0;

  setRgb (parser.toInt (),parser.toInt(),parser.toInt());
}
//****************************************************************************
const char *Color::getCommand ()
{
  char r[64];
  char g[64];
  char b[64];

  sprintf (r,"%d",red());
  sprintf (g,"%d",green());
  sprintf (b,"%d",blue());

  if (comstr) free (comstr);
  comstr=catString ("color (",
		    r,
		    ",",
		    g,
		    ",",
		    b,
		    ")");
		
  return comstr;
}
//****************************************************************************
Color::~Color ()
{
  if (comstr) free (comstr);
}
