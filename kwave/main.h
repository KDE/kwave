#include <qlist.h>
#include "mainwidget.h"
#include <drag.h>
#include "menumanager.h"

class TopWidget : public KTopLevelWidget
{
 Q_OBJECT
 public:

 	TopWidget	(KApplication   *app);
	~TopWidget	();
 void   closeWindow     ();
 void	setSignal	(QString name);
 void	setSignal	(MSignal *);
 void   addRecentFile   (char *);
 void   updateRecentFiles ();
 void   saveConfig ();
 void   readConfig ();

 protected:

 private:

 KApplication   *app;
 MainWidget	*mainwidget;
 KMenuBar	*bar;
 KStatusBar	*status;
 QString	name;
 QPopupMenu     *recent;
 QPopupMenu     *save;
 QPopupMenu     *mtypemenu;
 MenuManager    *manage;
 int            numchannels;
 int            bit24,bit16,bit8;
 int            bit;

 public slots:

 void	dropEvent	(KDNDDropZone *);
 void	deleteChannel	(int);
 void 	setRangeOp      (int);

 void 	quitInstance();
 void 	getHelp();
 void	newInstance();
 void	openFile();
 void   importAsciiFile();
 void   openRecent (int num);
 void	revert();
 void	saveFile();
 void	saveFileas();
 void	saveSelection();
 void	about();
 void	cliptoNew();
 void	flushClip();

 void	newOp();
 void	playBackOp ();
 void	memoryOp ();
 void   save24Bit ();
 void   save16Bit ();
 void   save8Bit ();
 void   saveBlocksOp();
};







