#include "Dialog.h"
#include <libkwave/MessagePort.h>
#include <libkwave/Global.h>
#include <stdio.h>

extern struct Global globals;

Dialog::Dialog (bool modal):QDialog (0,0,modal)
{
  this->modal=modal;
}
//**********************************************************************
Dialog::Dialog (const char *name,bool modal):QDialog (0,name,modal)
{
  if (name) setCaption (name);
  this->modal=modal;
}
//**********************************************************************
void Dialog::accept ()
{
  hide ();

  if (!modal)
    {
      const char *c=getCommand(); //call virtual function getcommand
      if (globals.port) globals.port->putMessage (c);
      else printf ("Error:Port is missing\n");
      delete this;
    }
  setResult (true);
}
//**********************************************************************
void Dialog::reject ()
{
  hide ();
  if (!modal) delete this;
  setResult (false);
}
//**********************************************************************
Dialog::~Dialog ()
{
}
