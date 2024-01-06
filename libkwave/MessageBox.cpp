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

#include <new>

#include <QApplication>
#include <QThread>
#include <QWidget>

#include <KMessageBox>

#include "libkwave/MessageBox.h"

//***************************************************************************
Kwave::MessageBox::MessageBox(KMessageBox::DialogType mode, QWidget *parent,
    QString message, QString caption,
    const QString &button1, const QString &button2,
    const QString &dontAskAgainName)
    :QObject(Q_NULLPTR), m_semaphore(0), m_retval(-1),
     m_mode(mode), m_parent(parent), m_message(message), m_caption(caption),
     m_button1(button1), m_button2(button2),
     m_dont_ask_again_name(dontAskAgainName)
{
    if (QThread::currentThread() == QApplication::instance()->thread()) {
        // we are already in the GUI thread -> direct call
        show();
    } else {
        // schedule execution in the GUI thread ...
        Trigger *trigger = new(std::nothrow) Trigger(*this);
        Q_ASSERT(trigger);
        if (!trigger) return;
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
    const QString buttonYes, const QString buttonNo,
    const QString &dontAskAgainName)
{
    return Kwave::MessageBox::exec(KMessageBox::QuestionTwoActions,
        parent, message, caption, buttonYes, buttonNo,
        dontAskAgainName);
}

//***************************************************************************
int Kwave::MessageBox::questionYesNoCancel(QWidget *parent,
    QString message, QString caption,
    const QString buttonYes, const QString buttonNo,
    const QString &dontAskAgainName)
{
    return Kwave::MessageBox::exec(KMessageBox::QuestionTwoActionsCancel,
        parent, message, caption, buttonYes, buttonNo,
        dontAskAgainName);
}

//***************************************************************************
int Kwave::MessageBox::sorry(QWidget *parent,
    QString message, QString caption)
{
    return Kwave::MessageBox::exec(KMessageBox::Error,
        parent, message, caption);
}

//***************************************************************************
int Kwave::MessageBox::warningYesNo(QWidget *parent,
    QString message, QString caption,
    const QString buttonYes, const QString buttonNo,
    const QString &dontAskAgainName)
{
    return Kwave::MessageBox::exec(KMessageBox::WarningTwoActions,
        parent, message, caption, buttonYes, buttonNo,
        dontAskAgainName);
}

//***************************************************************************
int Kwave::MessageBox::warningYesNoCancel(QWidget *parent,
    QString message, QString caption,
    const QString buttonYes, const QString buttonNo,
    const QString &dontAskAgainName)
{
    return Kwave::MessageBox::exec(KMessageBox::WarningTwoActionsCancel,
        parent, message, caption, buttonYes, buttonNo,
        dontAskAgainName);
}

//***************************************************************************
int Kwave::MessageBox::warningContinueCancel(QWidget *parent,
    QString message, QString caption,
    const QString buttonContinue, const QString buttonCancel,
    const QString &dontAskAgainName)
{
    return Kwave::MessageBox::exec(KMessageBox::WarningContinueCancel,
        parent, message, caption, buttonContinue, buttonCancel,
        dontAskAgainName);
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
    const QString &button1, const QString &button2,
    const QString &dontAskAgainName)
{
    Kwave::MessageBox box(
        mode, parent, message, caption,
        button1, button2,
        dontAskAgainName
    );
    return box.retval();
}

//***************************************************************************
KGuiItem Kwave::MessageBox::yes()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&Yes"),
                    QStringLiteral("dialog-ok"));
}

//***************************************************************************
KGuiItem Kwave::MessageBox::no()
{
    return KGuiItem(QApplication::translate("KStandardGuiItem", "&No"),
                    QStringLiteral("dialog-cancel"));
}

//***************************************************************************
void Kwave::MessageBox::show()
{
    switch (m_mode) {
        case KMessageBox::QuestionTwoActions:
            m_retval = KMessageBox::questionTwoActions(m_parent,
                m_message, m_caption,
                (!m_button1.isEmpty()) ? KGuiItem(m_button1) :
                                         Kwave::MessageBox::yes(),
                (!m_button2.isEmpty()) ? KGuiItem(m_button2) :
                                         Kwave::MessageBox::no(),
                m_dont_ask_again_name);
            break;
        case KMessageBox::QuestionTwoActionsCancel:
            m_retval = KMessageBox::questionTwoActionsCancel(m_parent,
                m_message, m_caption,
                (!m_button1.isEmpty()) ? KGuiItem(m_button1) :
                                         Kwave::MessageBox::yes(),
                (!m_button2.isEmpty()) ? KGuiItem(m_button2) :
                                         Kwave::MessageBox::no(),
                KStandardGuiItem::cancel(),
                m_dont_ask_again_name);
            break;
        case KMessageBox::WarningTwoActionsCancel:
            m_retval = KMessageBox::warningTwoActionsCancel(m_parent,
                m_message, m_caption,
                (!m_button1.isEmpty()) ? KGuiItem(m_button1) :
                                         Kwave::MessageBox::yes(),
                (!m_button2.isEmpty()) ? KGuiItem(m_button2) :
                                         Kwave::MessageBox::no(),
                KStandardGuiItem::cancel(),
                m_dont_ask_again_name);
            break;
        case KMessageBox::WarningTwoActions:
            m_retval = KMessageBox::warningTwoActions(m_parent,
                m_message, m_caption,
                (!m_button1.isEmpty()) ? KGuiItem(m_button1) :
                                         Kwave::MessageBox::yes(),
                (!m_button2.isEmpty()) ? KGuiItem(m_button2) :
                                         Kwave::MessageBox::no(),
                m_dont_ask_again_name);
            break;
        case KMessageBox::WarningContinueCancel:
            m_retval = KMessageBox::warningContinueCancel(m_parent,
                m_message, m_caption,
                (!m_button1.isEmpty()) ? KGuiItem(m_button1) :
                                         KStandardGuiItem::cont(),
                (!m_button2.isEmpty()) ? KGuiItem(m_button2) :
                                         KStandardGuiItem::cancel(),
                m_dont_ask_again_name);
            break;
        case KMessageBox::Error:
            KMessageBox::error(m_parent, m_message, m_caption);
            break;
        case KMessageBox::Information:
            KMessageBox::information(m_parent, m_message, m_caption,
                                     m_dont_ask_again_name);
            break;
        default:
            qWarning("unsupported messagebox mode");
            Q_ASSERT(0);
    }

    m_semaphore.release();
}

//***************************************************************************
Kwave::MessageBox::Trigger::Trigger(Kwave::MessageBox &box)
    :QObject(Q_NULLPTR), m_box(box)
{
    moveToThread(QApplication::instance()->thread());
}

//***************************************************************************
Kwave::MessageBox::Trigger::~Trigger()
{
    m_box.show();
}

//***************************************************************************
//***************************************************************************
