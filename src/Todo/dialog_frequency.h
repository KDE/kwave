#ifndef _DIALOGS_FREQ_H_
#define _DIALOGS_FREQ_H 1

#include <qdialog.h>
#include <qlabel.h>
#include <qtabdialog.h>
#include "../libgui/curvewidget.h"
#include <kintegerline.h>

class SweepDialog;
class FixedFrequencyDialog;
//*****************************************************************************
class FrequencyDialog : public QTabDialog
{
 Q_OBJECT

 public:
        FrequencyDialog 	(QWidget *parent=0,int rate=48000,char *name="Choose Parameters :");
 	~FrequencyDialog 	();
Curve*  getFrequency            ();

 public slots:

 void   setSelected             (const char *);

 protected:

 void   resizeEvent (QResizeEvent *);

 private:

 FixedFrequencyDialog *fixed;
 SweepDialog          *sweep;
 const char           *type;
 int                  rate;
};
#endif
