#ifndef KWAVEAPP_H
#define KWAVEAPP_H 1
#include <kapp.h>
#include <qtimer.h>

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
 void   saveConfig  ();

 private:

 QTimer *check;
 QList<TopWidget>   topwidgetlist; 
};
#endif






