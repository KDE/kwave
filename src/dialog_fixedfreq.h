#ifndef _DIALOGS_FIXED_H_
#define _DIALOGS_FIXED_H 1

#include <qdialog.h>
#include <kintegerline.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include "../libgui/slider.h"
//*****************************************************************************
class FixedFrequencyDialog : public QWidget
{
 Q_OBJECT

 public:
 	FixedFrequencyDialog 	(QWidget *parent,int rate,char *name);
 	~FixedFrequencyDialog 	();
 int    getFrequency();
 int    getTime();

 public slots:

  void showFrequency (int);
  void showTime  (int);

  protected:

 void resizeEvent (QResizeEvent *);

 private:
 KwaveSlider *timeslider;
 QLabel      *timelabel; 
 KwaveSlider *frequencyslider;
 QLabel      *frequencylabel; 
 int rate;
};
#endif
