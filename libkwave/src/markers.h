#ifndef _KWAVE_MARKERS_H_
#define _KWAVE_MARKERS_H_ 1

#include <qlist.h>
#include "color.h"


class MarkerType
//Type Information for Markers mainly information concerning the outlook
//of the particular type
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
  void setName (const char *);
  inline const char *getName () {return name;};
  inline MarkerType *getType () {return type;};

  int         pos;                         //position
 private:
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
