/***************************************************************************
            MessageBox.h -  threasafe wrapper for KMessageBox
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

#ifndef _MESSAGE_BOX_H_
#define _MESSAGE_BOX_H_

#include "config.h"

#include <QObject>
#include <QSemaphore>
#include <QString>

#include <kdemacros.h>
#include <kmessagebox.h>

class QWidget;

namespace Kwave {
    class KDE_EXPORT MessageBox: public QObject
    {
    Q_OBJECT

    public:

	/** @see KMessageBox::questionYesNo */
	static int questionYesNo(QWidget *widget,
	    QString message, QString caption = QString(),
	    const QString buttonYes = QString(),
	    const QString buttonNo  = QString());

	/** @see KMessageBox::questionYesNoCancel */
	static int questionYesNoCancel(QWidget *widget,
	    QString message, QString caption = QString(),
	    const QString buttonYes = QString(),
	    const QString buttonNo = QString());

	/** @see KMessageBox::sorry */
	static int sorry(QWidget *widget,
	    QString message, QString caption = QString());

	/** @see KMessageBox::warningYesNo */
	static int warningYesNo(QWidget *widget,
	    QString message, QString caption = QString(),
	    const QString buttonYes = QString(),
	    const QString buttonNo = QString());

	/** @see KMessageBox::warningYesNoCancel */
	static int warningYesNoCancel(QWidget *widget,
	    QString message, QString caption = QString(),
	    const QString buttonYes = QString(),
	    const QString buttonNo = QString());

	/** @see KMessageBox::warningContinueCancel */
	static int warningContinueCancel(QWidget *widget,
	    QString message, QString caption = QString(),
	    const QString buttonContinue = QString(),
	    const QString buttonCancel = QString());

	/** @see KMessageBox::error */
	static int error(QWidget *widget,
	    QString message, QString caption = QString());

    private:

	/** Default constructor, does nothing */
	MessageBox();

	/**
	 * Constructor
	 * @param mode the mode of the message box, error/warning/etc...
	 * @param parent parent widget
	 * @param message the message text of the box
	 * @param caption title of the window
	 * @param button1 a KGuiItem for the first button (optional)
	 * @param button2 a KGuiItem for the second button (optional)
	 * @return the return value of the KMessageBox
	 */
	MessageBox(KMessageBox::DialogType mode, QWidget *parent,
	    QString message, QString caption,
	    const QString &button1 = QString(),
	    const QString &button2 = QString());

	/** Destructor */
	virtual ~MessageBox();

	/** returns the return value of the KMessageBox */
	virtual int retval() const;

	/**
	 * Creates and executes a KMessageBox in the same thread or via
	 * a blocking signal, depending on whether the current context
	 * is the GUI thread or not.
	 *
	 * @param mode @see KMessageBox::DialogType
	 * @param parent the parent widget
	 * @param message the text of the message box
	 * @param caption the window title (optional)
	 * @param button1 a KGuiItem for the first button (optional)
	 * @param button2 a KGuiItem for the second button (optional)
	 * @return the result of the call to KMessageBox::xxx or -1
	 */
	static int exec(KMessageBox::DialogType mode, QWidget *parent,
	    QString message, QString caption = QString(),
	    const QString &button1 = QString(),
	    const QString &button2 = QString());

    protected:
	/**
	 * shows the KMessageBox, always called in the GUI
	 * thread context.
	 */
	void show();

    private:

	/** internal helper for showing the KMessageBox in the GUI thread */
	class Trigger: public QObject
	{
	public:
	    /**
	     * Constructor, re-parents itself to the GUI thread
	     * @param box the Kwave::MessageBox to use
	     */
	    Trigger(Kwave::MessageBox &box);

	    /**
	     * Destructor. Will always be executed in the GUI thread,
	     * because the instance of this object is moved to the GUI
	     * thread and deleteLater() is called. This calls the "show()"
	     * method of the Kwave::MessageBox in a safe context.
	     */
	    virtual ~Trigger();

	private:

	    /** reference to the Kwave::MessageBox instance */
	    Kwave::MessageBox &m_box;
	};

    private:

	/** semaphore for waiting on the GUI thread */
	QSemaphore m_semaphore;

	/** return value of the execution of the message box */
	int m_retval;

	/** @see KMessageBox::DialogType */
	KMessageBox::DialogType m_mode;

	/** the parent widget */
	QWidget *m_parent;

	/** the text of the message box */
	QString m_message;

	/** the window title (optional) */
	QString m_caption;

	/** first button (optional) */
	const QString m_button1;

	/** second button (optional) */
	const QString m_button2;
    };
}

#endif /* _MESSAGE_BOX_H_ */
