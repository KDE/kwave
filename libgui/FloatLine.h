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
