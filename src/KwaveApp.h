#ifndef _KWAVE_APP_H
#define _KWAVE_APP_H 1

#include <kapp.h>
#include <qlist.h>

class QTimer;
class TopWidget;

class KwaveApp : public KApplication
{
  Q_OBJECT
 public:

 	KwaveApp	  (int argc, char **argv);
	~KwaveApp	  ();

 void   setOp             (const char *);
 void   addRecentFile     (char *);
 void   updateRecentFiles ();
 void   parseCommands     (const char *);
 void   closeWindow       (TopWidget *);

 public slots:
 void	timer();

 protected:

 void	newWindow   ();
 void   readConfig  ();
 void	saveRecentFiles ();
 void   saveConfig  ();

 private:

 QTimer *check;
 QList<TopWidget>   topwidgetlist; 
};

#endif // _KWAVE_APP_H
