#include <stdio.h>
#include <stdlib.h>
#include <qpushbutton.h>
#include <qkeycode.h>
#include "module.h"
#include <kapp.h>

const char *version="";
const char *author="";
const char *name="";
//**********************************************************
KwaveDialog *getDialog (DialogOperation *operation)
{
  return new KwaveDialog(operation->isModal());
}
//**********************************************************













