#ifndef _KWAVE_COLOR_H_
#define _KWAVE_COLOR_H_ 1

#include <qcolor.h>

class Color:public QColor
{
 public:
  Color  ();                 //zero constructor
  Color  (int,int,int);      //true color version
  Color  (const char *);     //construction via command string
  ~Color ();

         const char* getCommand           ();
 private:
 char *comstr;
};
#endif



