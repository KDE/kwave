/***************************************************************************
            ShortcutWrapper.h  -  wrapper for keyboard shortcuts
			     -------------------
    begin                : Sat Jan 12 2008
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

#ifndef _SHORTCUT_WRAPPER_H_
#define _SHORTCUT_WRAPPER_H_

#include "config.h"

#include <QObject>
#include <QShortcut>

class QWidget;
class QKeySequence;

/**
 * Wrapper for keyboard shortcuts, emits signal activated(id)
 */
namespace Kwave
{
    class ShortcutWrapper: public QShortcut
    {
    Q_OBJECT
    public:
	/**
	 * Constructor
	 * @param parent a parent widget
	 * @param key a keyboard shortcuts sequence
	 * @param id numeric id that gets emitted
	 */
	ShortcutWrapper(QWidget *parent, const QKeySequence &key, int id);

	/** Destructor */
	virtual ~ShortcutWrapper();

    signals:
	/**
	 * emitted when the shortcut is activated
	 * @param id the numeric id passed in the constructor
	 */
	void activated(int id);

    private slots:

	/** internally connected to the signal QShortcut::activated() */
	void triggered();

    private:

	/** numeric id for the activated() signal */
	int m_id;
    };
}

#endif /* _SHORTCUT_WRAPPER_H_ */

//***************************************************************************
//***************************************************************************
