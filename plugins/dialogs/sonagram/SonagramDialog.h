#ifndef _SONAGRAM_DIALOG_H_
#define _SONAGRAM_DIALOG_H_

#include <qdialog.h>

class Slider;
class QComboBox;
class QLabel;
class QPushButton;
class KwavePlugin;

//*****************************************************************************
class SonagramDialog : public QDialog 
{
    Q_OBJECT

public:
    SonagramDialog(KwavePlugin &p);
    virtual ~SonagramDialog ();

    const char * getCommand ();

public slots:
    void setPoints (int);
    void setBoxPoints (int);

private:

    Slider *pointslider;
    QLabel *pointlabel;
    QLabel *windowlabel;
    QLabel *bitmaplabel;
    QLabel *windowtypelabel;
    QComboBox *pointbox;
    QComboBox *windowtypebox;
    QPushButton *ok, *cancel;
    int length, rate;
};

#endif /* _SONAGRAM_DIALOG_H_ */

/* end of SonagramDialog.h */
