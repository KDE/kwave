/*************************************************************************
   ConfirmCancelProxy.h  -  ask for confirm before cancelling an action
                             -------------------
    begin                : Fri Apr 26 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
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

#ifndef _CONFIRM_CANCEL_PROXY_H_
#define _CONFIRM_CANCEL_PROXY_H_

#include <config.h>

#include <QtCore/QObject>

#include <kdemacros.h>

class QString;
class QWidget;

namespace Kwave
{

    class KDE_EXPORT ConfirmCancelProxy: public QObject
    {
	Q_OBJECT
    public:

	/**
	 * Constructor.
	 * @param widget a QWidget used as parent for the confirmation
	 *        message box
	 * @param sender object that emits a cancelled signal, normally
	 *        something like a QProgressDialog. Can be null, in this
	 *        case the signal parameter is ignored and the cancel()
	 *        method has to be called manually.
	 * @param signal emitted signal, e.g. SIGNAL(cancelled())
	 * @param receiver object that receives the cancel signal if
	 *        a cancel has been received and the user confirmed
	 * @param slot receiver's cancel slot, e.g. SLOT(cancel())
	 */
	ConfirmCancelProxy(QWidget *widget,
	                   QObject *sender,   const char *signal,
	                   QObject *receiver, const char *slot);

	/** Destructor */
	virtual ~ConfirmCancelProxy();

    public slots:

	/** will be connected to the sender of the cancel */
	void cancel();

    signals:

	/** emitted if cancel was received and user confirmed */
	void canceled();

    private:

	/** widget used for the confirm message */
	QWidget *m_widget;

    };
}

#endif /* _CONFIRM_CANCEL_PROXY_H_ */

//***************************************************************************
//***************************************************************************
