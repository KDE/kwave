#include <qlist.h>
#include "classes.h"

class TopWidget : public KTopLevelWidget
{
 Q_OBJECT
 public:

 	TopWidget	(const char *name=0);
 void	setSignal	(QString name);
 void	setSignal	(MSignal *);

 protected:

 private:

 MainWidget	*mainwidget;
 KMenuBar	*bar;
 KStatusBar	*status;
 QString	name;

 public slots:

 void 	inst_quit();
 void	file_new();
 void	inst_new();
 void	openFile();
 void	saveFile();
 void	saveFileas();
 void	saveSelection();
 void	about();
 void	cliptoNew();
 void	flushClip();

 void	deleteOp();
 void	cutOp	();
 void	copyOp	();
 void	zeroOp	();
 void	pasteOp	();
 void	cropOp	();
 void	flipOp	();
 void	reverseOp();
};

