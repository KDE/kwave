

#ifndef _FILE_PROGRESS_H_
#define _FILE_PROGRESS_H_

#include <kdialog.h>

class KProgress;

class FileProgress: public KDialog
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param parent the parent widget
     */
    FileProgress(QWidget *parent);

public slots:
    /**
     * Advances the progress to a given percentage.
     */
    void setValue(int percent);

protected:
    KProgress *m_progress;

};

#endif /* _FILE_PROGRESS_H_ */
