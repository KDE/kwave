#ifndef _KWAVE_MARKERS_H_
#define _KWAVE_MARKERS_H_ 1

#include <qlist.h>
#include "color.h"


class MarkerType
//Type Information for Markers mainly information concerning the outlook
{
 public:
  MarkerType ();
  MarkerType (const char *);
  ~MarkerType();
  const char *getCommand ();

  const char* name;         //pointer to name
  Color*      color;        //color
  bool        named;        //boolean if named
  bool        selected;
 private:
  char        *comstr;
};
//***********************************************************
class Marker
//Marker Class is for marking positions in the signal
{
 public:
  Marker (int,MarkerType *,const char *name=0);
  ~Marker();

  int         pos;                         //position
  char*       name;                        //pointer to name
  MarkerType *type;                        //pointer to type
};
//***********************************************************
class MarkerList:public QListT<Marker>
//Class for list of Markers has to inherit from QList to implement own
//compareItems method for sorting
{
  //  Q_OBJECT
 public:
  MarkerList::MarkerList();
  MarkerList::~MarkerList();
  int MarkerList::compareItems (GCI,GCI);
};
#endif //_KWAVEMARKERS_H_
