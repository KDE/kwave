#ifndef _MULTI_STATE_WIDGET_H_
#define _MULTI_STATE_WIDGET_H_

#include "config.h"
#include <qwidget.h>
#include <qpainter.h>

class QString;

class MultiStateWidget : public QWidget
{
    Q_OBJECT

public:

    /** Constructor */
    MultiStateWidget(QWidget *parent = 0, int = 0, int = 2);

    /** Destructor */
    virtual ~MultiStateWidget ();

    /**
     * Sets the number that will passed as argument to the
     * "clicked" signal.
     */
    void setNumber(int number);

    /**
     * Adds a the content of pixmap file as pixmap for the
     * next state. The file is found through the KStandardDirs
     * mechanism. Adding a file for a second or further time
     * is not possible, in this case the pixmap will not be
     * loaded and the return value will be the id of the
     * existing version.
     * @see KStandardDirs
     * @param filename name of the file to be added, without
     *        path.
     * @return id of the corresponding state or -1 if
     *         something went wrong
     */
    int addPixmap(const QString &filename);

    void setStates (int *newstates);
    void setState (int newstate);
    void nextState ();
signals:

    /**
     * Signals that the widget has changed it's state.
     * @param number identifier of this widget's instance
     */
    void clicked(int number);

public slots:

protected:

    void mouseReleaseEvent(QMouseEvent * );
    void paintEvent(QPaintEvent *);

private:

    QPainter p;
    int *states;     // maps states to pixmap-list
    int act;         // current states
    int count;       // number of states
    int number;      // number of channels this object represents... used for signals
}
;

#endif  // _MULTI_STATE_WIDGET_H_
