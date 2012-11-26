/***************************************************************************
        KeywordWidget.h  -  widget for editing a list of keywords
			     -------------------
    begin                : Fri 02 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KEYWORD_WIDGET_H_
#define _KEYWORD_WIDGET_H_

#include "config.h"

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include "ui_KeywordWidgetBase.h"

class QString;
class QWidget;

namespace Kwave
{
    /**
     * Re-implemented and slightly modified version of a KEditListBox. All
     * items in the list are unique and sorted alphabetically, leading and
     * trailing whitespaces are removed. An additional "Auto" button is
     * provided for populating the list with automatically generated entries.
     */
    class KeywordWidget: public QWidget,
                         Ui::KeywordWidgetBase
    {
	Q_OBJECT
    public:

	/** Constructor */
	KeywordWidget(QWidget *parent);

	/** Destructor */
	virtual ~KeywordWidget();

	/** Returns the list of keywords (sorted) */
	QStringList keywords();

	/** Sets/initializes the list of keywords */
	void setKeywords(const QStringList &keywords);

    signals:

	/** emitted if the user pressed the "Auto" button */
	void autoGenerate();

    private slots:

	/** updates the controls if the text edit changed */
	void editChanged(const QString &);

	/** called if the user pressed return in the edit line */
	void returnPressed(const QString &);

	/** add an entry to the list */
	void add();

	/** remove the currently selected item from the list */
	void remove();

	/** called when a new list entry has been selected */
	void listClicked(QListWidgetItem *item);

	/** forwards the click of the "Auto" button */
	void autoClicked();

    protected:

	/** returns true if the given string is contained in the list */
	bool contained(const QString &item);

	/** update the enable state of the buttons */
	void update();

    };
}

#endif /* _KEYWORD_WIDGET_H_ */

//***************************************************************************
//***************************************************************************
