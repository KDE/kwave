
#include <qlayout.h>

#include <kprogress.h>

#include "libgui/FileProgress.h"

//***************************************************************************
FileProgress::FileProgress(QWidget *parent)
    :KDialog(parent)
{
    QGridLayout *top_layout = new QGridLayout(this, 1, 1);
    m_progress = new KProgress(0, 100, 0, Horizontal, this);

    top_layout->addWidget(m_progress, 0, 0);
    top_layout->activate();
}

//***************************************************************************
void FileProgress::setValue(int percent)
{
    ASSERT(m_progress);
    if (!m_progress) return;
    m_progress->setValue(percent);
}

//***************************************************************************
//***************************************************************************
