#ifndef GLOBALS_H
#define GLOBALS_H 1

#include <qlist.h>

class MarkerType;
class ClipBoard;
class KwavePlugin;
class KApplication;
struct Global
{
  KApplication       *app;
  ClipBoard          *clipboard;
  KwavePlugin        **dialogplugins;
  KwavePlugin        **timeplugins;
  QList<MarkerType>  markertypes;
  const char         *globalconfigDir;
  const char         *localconfigDir;
  const char         *filterDir;
};
#endif
