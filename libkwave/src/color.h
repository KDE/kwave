#ifndef _KWAVE_COLOR_H_
#define _KWAVE_COLOR_H_ 1

#include <qcolor.h>

class Color:public QColor
{
 public:
  Color  ();
  Color  (const char *);
  ~Color ();

         const char* getCommand           ();
 private:
 char *comstr;
};
#endif



