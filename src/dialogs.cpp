#include "kwavedialog.h"

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
  const char *c=getCommand();
  if (c) emit command (c);
  if (!modal) delete this;
}
//**********************************************************************
void KwaveDialog::reject ()
{
  hide ();
  if (!modal) delete this;
}
//**********************************************************************
KwaveDialog::~KwaveDialog ()
{
}
