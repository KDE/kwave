#ifndef KWAVE_DIALOG_OPERATION
#define KWAVE_DIALOG_OPERATION 1
#include <kapp.h>
#include "kwavestring.h"

struct Global;
class DialogOperation
{
 public:

 DialogOperation::DialogOperation  (const char *name, bool modal=false);
 DialogOperation::DialogOperation  (Global *,int l,int r,int c,bool=false);
 DialogOperation::DialogOperation  (int rate,bool modal=false);
 DialogOperation::~DialogOperation ();

 inline const char   *getName     () {return name;};
 inline Global       *getGlobals  () {return globals;};
 inline int           getRate     () {return rate;};
 inline int           getLength   () {return length;};
 inline int           getChannels () {return channels;};
 inline bool          isModal     () {return modal;};
 private:
 Global      * globals;
 int           length;
 int           rate;
 int           channels;
 char        * name;
 bool          modal;
};
#endif
