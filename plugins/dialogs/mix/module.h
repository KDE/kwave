#ifndef _DIALOGS_MIX_H_
#define _DIALOGS_MIX_H_ 1

#include <qwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include "../../../libgui/slider.h"
#include "../../../libgui/guiitems.h"
#include "../../../libgui/kwavedialog.h"
#include "../../../lib/dialogoperation.h"

class ChannelMixDialog : public KwaveDialog
{
 Q_OBJECT

 public:

 	ChannelMixDialog 	(bool modal,int numberofchannels);
 	~ChannelMixDialog ();
 const char *getCommand   ();

 public slots:

 void setValue (int);
 void setValue (const char *);
 void setdBMode(bool);

 protected:

 void resizeEvent (QResizeEvent *);

 private:

 QLabel         **channelname;
 FloatLine      **valuebox;
 KwaveSlider    **slider;
 QPushButton	*ok,*cancel;
 QLabel         *tochannellabel;
 QComboBox      *tochannel;
 QLabel         *usedblabel;
 QCheckBox      *usedb;
 int            channels; //number of channels available
 double         *value;
 bool           tflag;
 bool           dbmode;
 char           *comstr;
};
//*****************************************************************************
#endif
