#ifndef _GUIITEMS_H_
#define _GUIITEMS_H_ 1

#include <qobject.h>
#include <qpopupmenu.h>
#include <krestrictedline.h>

class FloatLine : public KRestrictedLine
//Widget for entering Floats
{
  Q_OBJECT
    public:
 FloatLine (QWidget* parent,double=0);
 ~FloatLine ();

 void   setValue (double);
 double value    ();
 private:
 char  digits; //allowed number of digits behind the .
};
//*****************************************************************************
class TimeLine : public KRestrictedLine
//Widget for entering Time in various units
{
 Q_OBJECT
   public:
 TimeLine (QWidget* parent,int rate=48000);
 ~TimeLine ();

 void setSamples    (int);
 void setMs         (int);
 int  getValue      ();

 public slots:
 void setSampleMode ();
 void setMsMode     ();
 void setKbMode     ();
 void setSMode      ();
 void setValue      (const char *);
 void setRate       (int);

 protected:
 void mousePressEvent( QMouseEvent *);

 int value;
 int mode;     //flag for display and enter mode...
 int rate;
 QPopupMenu *menu;
};
#endif
