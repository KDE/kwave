#ifndef _PITCH_CONTAINER_H_
#define _PITCH_CONTAINER_H_

#include "config.h"
#include <qwidget.h>

class PitchWidget;
class ScaleWidget;
class CornerPatchWidget;
class ScaleWidget;
class QResizeEvent;

//***********************************************************************
class PitchContainer : public QWidget
{
    Q_OBJECT
public:
    PitchContainer(QWidget *parent);
    ~PitchContainer();
    void setObjects(PitchWidget *view, ScaleWidget *x, ScaleWidget *y,
		    CornerPatchWidget *corner);

public slots:

signals:

protected:

    void resizeEvent (QResizeEvent *);

private:
    PitchWidget *view;
    ScaleWidget *xscale, *yscale;
    CornerPatchWidget *corner;
};

#endif // _PITCH_CONTAINER_H_
