#ifndef _KWAVEMARKERS_H_
#define _KWAVEMARKERS_H_ 1

#include <qstring.h>
#include <qlist.h>

class MarkerType
//Type Information for Markers mainly information concerning the outlook
{
 public:
  MarkerType ();
  ~MarkerType();

  QString*name;         //pointer to name
  QColor *color;        //color
  bool    named;         //boolean if named
  bool    selected;
};
//***********************************************************
class Marker
//Marker Class is for marking positions in the signal
{
 public:
  Marker();
  ~Marker();

  int     pos;                         //position
  QString *name;                       //pointer to name
  struct  MarkerType *type;            //pointer to type
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
