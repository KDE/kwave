#ifndef GLOBALS_H
#define GLOBALS_H 1

#include <qlist.h>
#include "markers.h"
#include "messageport.h"

class ClipBoard;
class KwavePlugin;
class KwaveApp;
struct Global
{
  KwaveApp           *app;
  ClipBoard          *clipboard;
  KwavePlugin        **dialogplugins;
  KwavePlugin        **timeplugins;
  MessagePort        *port;
  QList<MarkerType>  markertypes;
  const char         *globalconfigDir;
  const char         *localconfigDir;
  const char         *filterDir;
  int                mmap_threshold;
  const char         *path_to_executable;
};
#endif
