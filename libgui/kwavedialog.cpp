#include "kwavedialog.h"
#include <libkwave/globals.h>

extern struct Global globals;

KwaveDialog::KwaveDialog (bool modal):QDialog (0,0,modal)
{
  this->modal=modal;
}
//**********************************************************************
KwaveDialog::KwaveDialog (const char *name,bool modal):QDialog (0,name,modal)
{
  if (name) setCaption (name);
  this->modal=modal;
}
//**********************************************************************
void KwaveDialog::accept ()
{
  hide ();

  if (!modal)
    {
      const char *c=getCommand(); //call virtual function getcommand
      if (globals.port) globals.port->putMessage (c);
      else printf ("Error:Port is missing\n");
      //      if (c) emit command (c);    //and emit result of dialog
      delete this;
    }
  setResult (true);
}
//**********************************************************************
void KwaveDialog::reject ()
{
  hide ();
  if (!modal) delete this;
  setResult (false);
}
//**********************************************************************
KwaveDialog::~KwaveDialog ()
{
}
