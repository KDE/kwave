#ifndef _PROGRESS_DIALOG_H_
#define _PROGRESS_DIALOG_H_ 1

#include <qdialog.h>

class TimeOperation;
class QTimer;
class QPaintEvent;

class ProgressDialog : public QDialog
{
 Q_OBJECT

 public:
	ProgressDialog 	(TimeOperation *,const char *caption);
 	ProgressDialog 	(int max=100,char*caption="Progress");
 	~ProgressDialog ();
 void	setProgress(int);

 public slots:

 void	timedProgress();

 protected:

 void paintEvent  (QPaintEvent *);

 private:

 int max;
 int act;
 int last;
 int lastx,lasty;
 int oldw;
 TimeOperation *operation;
 QTimer        *timer;
};

#endif // _PROGRESS_DIALOG_H_
