#ifndef _TIME_LINE_H_
#define _TIME_LINE_H_

#include "config.h"
#include <qobject.h>
#include <qpopupmenu.h>
#include <qstring.h>

#include <krestrictedline.h>

//*****************************************************************************
class TimeLine : public KRestrictedLine
	    //Widget for entering Time in various units
{
    Q_OBJECT
public:
    TimeLine(QWidget* parent, int rate = 48000);
    ~TimeLine();

    void setSamples(int);
    void setMs(int);
    int getValue();
    double getMs();
    QString getMsStr();

public slots:
    void setSampleMode();
    void setMsMode();
    void setKbMode();
    void setSMode();
    void setValue(const char *);
    void setRate(int);

protected:
    void mousePressEvent( QMouseEvent *);

    int value;        //# of samples
    int mode;         //flag for display and enter mode...
    int rate;         //rate for calculating time
    QPopupMenu *menu;
    char *comstr;
};

#endif // _TIME_LINE_H_
