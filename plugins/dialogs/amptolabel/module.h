#ifndef _DIALOGS_MARK_H_
#define _DIALOGS_MARK_H 1

#include <qdialog.h>
#include <qlabel.h>
#include "../../../libgui/guiitems.h"
#include "../../../libgui/slider.h"
#include "../../../libgui/kwavedialog.h"
#include <libkwave/dialogoperation.h>
#include <kintegerline.h>

struct Global;
//*****************************************************************************
class MarkSignalDialog : public KwaveDialog
{
 Q_OBJECT

 public:

 	MarkSignalDialog 	(const Global *,int,bool);
 	~MarkSignalDialog ();
 
 const char *getCommand ();

 public slots:

 void setAmpl (int);
 void setAmpl (const char *);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLabel         *timelabel,*ampllabel;
 FloatLine      *ampl;
 TimeLine	*time;
 KwaveSlider    *amplslider;
 QLabel         *mark1,*mark2;
 QComboBox	*marktype1,*marktype2;
 QPushButton	*ok,*cancel;
 char *         comstr;
 bool           tflag;
};
//*****************************************************************************
#endif
