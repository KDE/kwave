
#include "libgui/Dialog.h"
#include "libgui/Slider.h"
#include "libkwave/DialogOperation.h"

class Signal;
class TimeLine;
class Curve;
class KIntegerLine;
class ScaleWidget;
class CornerPatchWidget;
class QLabel;
class QComboBox;

//*****************************************************************************
class AddSynthWidget : public QWidget {
    Q_OBJECT
public:
    AddSynthWidget (QWidget *parent = 0);
    ~AddSynthWidget ();
    void setSines (int, int *, int*, int *);

public slots:

    void setFunction (int);

signals:

protected:

    void paintEvent(QPaintEvent *);

private:

    int func;
    int *power;
    int *phase;
    int *mult;
    int count;
};
//***********************************************************************
class AddSynthDialog : public Dialog {
    Q_OBJECT

public:

    AddSynthDialog (int, int, bool);
    ~AddSynthDialog ();

    const char *getCommand ();
    Signal *getSignal();

public slots:

    void setChannels (const char *);
    void getFrequency();
    void showPower (const char *);
    void showPower (int);
    void showPhase (const char *);
    void showPhase (int);
    void showMult (const char *);
    void showMult (int);
    void popMenu ();
    void oddMult ();
    void evenMult ();
    void primeMult ();
    void zeroPhase ();
    void sinusPhase ();
    void negatePhase ();
    void enumerateMult ();
    void randomizePower ();
    void randomizePhase ();
    void fibonacciMult ();
    void invertPower ();
    void sinusPower ();
    void maxPower ();
    void dbPower ();
protected:
    void mousePressEvent( QMouseEvent *e);
    void updateView ();
    bool getNSlider (int, bool);
    void resizeEvent (QResizeEvent *);
    int getCount();           //get number of partials

private:

    Dialog *sweep;
    Curve *times;

    AddSynthWidget *view;
    ScaleWidget *x, *y;
    CornerPatchWidget *corner;

    KIntegerLine **mult;
    KIntegerLine **powerlabel;
    KIntegerLine **phaselabel;

    Slider **power;
    Slider **phase;

    KIntegerLine *channel;
    QLabel *channellabel;

    QPushButton *freqbutton;
    QPushButton *calculate;
    Signal *test;              //for hearing not yet done...

    QComboBox *functype;
    QPopupMenu* menu;

    QLabel *phaselab;
    QLabel *powerlab;
    QLabel *multlab;
    QPushButton *ok, *cancel;

    int *apower, *aphase, *amult;
    int num;       //number of sines
    int rate;      //sampling rate

    char *command;

    bool tflag;    //flag if typing in integerline
}
;
