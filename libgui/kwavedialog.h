#ifndef KWAVE_DIALOG
#define KWAVE_DIALOG 1
#include <qdialog.h>

#define OK     klocale->translate ("&Ok")
#define CANCEL klocale->translate ("&Cancel")

class KwaveDialog : public QDialog
{
 Q_OBJECT
 public:

  KwaveDialog (bool=false);
  KwaveDialog (const char *,bool=false);
  ~KwaveDialog ();
  public slots:

   void accept ();
   void reject ();

  virtual const char *getCommand ()=0;
 signals:
  void command (const char *);
 private:
  bool modal;
};
#endif


