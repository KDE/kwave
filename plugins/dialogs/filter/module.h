#include "../../../libgui/Dialog.h"
#include <libkwave/DialogOperation.h>

#include <qdialog.h>
#include <qdir.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qradiobt.h>

#include "../../../libgui/Slider.h"
#include <libkwave/Filter.h>
#include "../../../libgui/FFTWidget.h"
#include "../../../libgui/ScaleWidget.h"
#include "../../../libgui/CornerPatchWidget.h"
#include <kintegerline.h>

//*****************************************************************************
class FilterDialog : public Dialog
{
 Q_OBJECT

 public:
 	FilterDialog 	(bool modal,int rate);
 	~FilterDialog 	();
 void	refreshView ();
 const  char *getCommand ();

 public slots:

 void setTaps      (const char *);
 void setOffset    (const char *);
 void setMult      (int);
 void refresh      ();
 void loadFilter   ();
 void saveFilter   ();

 protected:

 void resizeEvent   (QResizeEvent *);
 void getNTaps      (int);
 
 private:

 int                w,h;
 Filter             *filter;

 int                oldnum;
 QLabel*            taplabel;
 KIntegerLine*      taps;

 QLabel**           label;
 Slider**           mult;
 KIntegerLine**     offset;

 QPushButton*       load;
 QPushButton*       save;

 QLabel*            iirlabel;
 QLabel*            firlabel;

 QButtonGroup*      bg;
 QRadioButton*      fir,*iir;
 
 QPushButton*       ok;
 QPushButton*       cancel;
 QPushButton*       dofilter;
 FFTWidget*         filterwidget;
 FFTWidget*         phasewidget;
 ScaleWidget*       ampx,*ampy;
 ScaleWidget*       phasex,*phasey;
 CornerPatchWidget* phasecorner;
 CornerPatchWidget* ampcorner;
 QDir *             filterDir;
 char *             comstr;
};
//*****************************************************************************
