/***************************************************************************
          MessageBox.cpp -  threasafe wrapper for KMessageBox
			     -------------------
    begin                : Sun Apr 13 2008
    copyright            : (C) 2008 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"

#include <QApplication>
#include <QThread>
#include <QWidget>

#include <kmessagebox.h>

#include "libkwave/MessageBox.h"

//***************************************************************************
Kwave::MessageBox::MessageBox()
    :QObject(), m_retval(-1)
{
}

//***************************************************************************
Kwave::MessageBox::MessageBox(KMessageBox::DialogType mode, QWidget *parent,
    QString message, QString caption,
    const QString &button1, const QString &button2)
    :QObject(0), m_semaphore(0), m_retval(-1),
     m_mode(mode), m_parent(parent), m_message(message), m_caption(caption),
     m_button1(button1), m_button2(button2)
{
    if (QThread::currentThread() == QApplication::instance()->thread()) {
	// we are already in the GUI thread -> direct call
	show();
    } else {
	// schedule execution in the GUI thread ...
	Trigger *trigger = new Trigger(*this);
	trigger->deleteLater();

	// ... and wait
	m_semaphore.acquire();
    }
}

//***************************************************************************
int Kwave::MessageBox::retval() const
{
    return m_retval;
}

//***************************************************************************
Kwave::MessageBox::~MessageBox()
{
}

//***************************************************************************
int Kwave::MessageBox::questionYesNo(QWidget *parent,
    QString message, QString caption,
    const QString buttonYes, const QString buttonNo)
{
    return Kwave::MessageBox::exec(KMessageBox::QuestionYesNo,
	parent, message, caption, buttonYes, buttonNo);
}

//***************************************************************************
int Kwave::MessageBox::questionYesNoCancel(QWidget *parent,
    QString message, QString caption,
    const QString buttonYes, const QString buttonNo)
{
    return Kwave::MessageBox::exec(KMessageBox::QuestionYesNoCancel,
	parent, message, caption, buttonYes, buttonNo);
}

//***************************************************************************
int Kwave::MessageBox::sorry(QWidget *parent,
    QString message, QString caption)
{
    return Kwave::MessageBox::exec(KMessageBox::Sorry,
	parent, message, caption);
}

//***************************************************************************
int Kwave::MessageBox::warningYesNo(QWidget *parent,
    QString message, QString caption,
    const QString buttonYes, const QString buttonNo)
{
    return Kwave::MessageBox::exec(KMessageBox::WarningYesNo,
	parent, message, caption, buttonYes, buttonNo);
}

//***************************************************************************
int Kwave::MessageBox::warningYesNoCancel(QWidget *parent,
    QString message, QString caption,
    const QString buttonYes, const QString buttonNo)
{
    return Kwave::MessageBox::exec(KMessageBox::WarningYesNoCancel,
	parent, message, caption, buttonYes, buttonNo);
}

//***************************************************************************
int Kwave::MessageBox::warningContinueCancel(QWidget *parent,
    QString message, QString caption,
    const QString buttonContinue, const QString buttonCancel)
{
    return Kwave::MessageBox::exec(KMessageBox::WarningContinueCancel,
	parent, message, caption, buttonContinue, buttonCancel);
}

//***************************************************************************
int Kwave::MessageBox::error(QWidget *parent,
    QString message, QString caption)
{
    return Kwave::MessageBox::exec(KMessageBox::Error,
	parent, message, caption);
}

//***************************************************************************
int Kwave::MessageBox::exec(KMessageBox::DialogType mode, QWidget *parent,
    QString message, QString caption,
    const QString &button1, const QString &button2)
{
    Kwave::MessageBox box(mode, parent, message, caption, button1, button2);
    return box.retval();
}

//***************************************************************************
void Kwave::MessageBox::show()
{
    switch (m_mode) {
	// QuestionYesNo
	case KMessageBox::QuestionYesNo:
	    m_retval = KMessageBox::questionYesNo(m_parent,
		m_message, m_caption,
		(!m_button1.isEmpty()) ? KGuiItem(m_button1) :
		                         KStandardGuiItem::yes(),
		(!m_button2.isEmpty()) ? KGuiItem(m_button2) :
		                         KStandardGuiItem::no());
	    break;
	// QuestionYesNoCancel
	case KMessageBox::QuestionYesNoCancel:
	    m_retval = KMessageBox::questionYesNoCancel(m_parent,
		m_message, m_caption,
		(!m_button1.isEmpty()) ? KGuiItem(m_button1) :
		                         KStandardGuiItem::yes(),
		(!m_button2.isEmpty()) ? KGuiItem(m_button2) :
		                         KStandardGuiItem::no());
	    break;
	// Sorry
	case KMessageBox::Sorry:
	    KMessageBox::sorry(m_parent, m_message, m_caption);
	    break;
	// WarningYesNoCancel
	case KMessageBox::WarningYesNoCancel:
	    m_retval = KMessageBox::warningYesNoCancel(m_parent,
		m_message, m_caption,
		(!m_button1.isEmpty()) ? KGuiItem(m_button1) :
		                         KStandardGuiItem::yes(),
		(!m_button2.isEmpty()) ? KGuiItem(m_button2) :
		                         KStandardGuiItem::no());
	    break;
	// WarningYesNo
	case KMessageBox::WarningYesNo:
	    m_retval = KMessageBox::warningYesNo(m_parent,
		m_message, m_caption,
		(!m_button1.isEmpty()) ? KGuiItem(m_button1) :
		                         KStandardGuiItem::yes(),
		(!m_button2.isEmpty()) ? KGuiItem(m_button2) :
		                         KStandardGuiItem::no());
	    break;
	// WarningContinueCancel
	case KMessageBox::WarningContinueCancel:
	    m_retval = KMessageBox::warningContinueCancel(m_parent,
		m_message, m_caption,
		(!m_button1.isEmpty()) ? KGuiItem(m_button1) :
		                         KStandardGuiItem::cont(),
		(!m_button2.isEmpty()) ? KGuiItem(m_button2) :
		                         KStandardGuiItem::cancel());
	    break;
	// Error
	case KMessageBox::Error:
	    KMessageBox::error(m_parent, m_message, m_caption);
	    break;
	default:
	    qWarning("unsupported messagebox mode");
	    Q_ASSERT(m_mode != m_mode);
    }

    m_semaphore.release();
}

//***************************************************************************
Kwave::MessageBox::Trigger::Trigger(Kwave::MessageBox &box)
    :QObject(0), m_box(box)
{
    moveToThread(QApplication::instance()->thread());
}

//***************************************************************************
Kwave::MessageBox::Trigger::~Trigger()
{
    m_box.show();
}

//***************************************************************************
using namespace Kwave;
#include "MessageBox.moc"
//***************************************************************************
//***************************************************************************
