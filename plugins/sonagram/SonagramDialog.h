/***************************************************************************
         SonagramDialog.h  -  dialog for setting up the sonagram window
                             -------------------
    begin                : Fri Jul 28 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
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

#ifndef SONAGRAM_DIALOG_H
#define SONAGRAM_DIALOG_H

#include "config.h"

#include <QtGui/QDialog>

#include "libkwave/Sample.h"
#include "libkwave/WindowFunction.h"
#include "ui_SonagramDlg.h"

class QStringList;

namespace Kwave
{

    class Plugin;

    class SonagramDialog : public QDialog,
                           public Ui::SonagramDlg
    {
	Q_OBJECT

    public:
	/** Constructor */
	explicit SonagramDialog(Kwave::Plugin &p);

	/** Destructor */
	virtual ~SonagramDialog();

	/**
	 * Fills the current parameters into a parameter list.
	 * The list always is cleared before it gets filled.
	 * The first parameter will contain the number of fft points [1...n]
	 * The second parameter will contain the id of a window function
	 * or zero if no window function was selected ("<none>").
	 */
	void parameters(QStringList &list);

    public slots:

	/** sets the number of fft points */
	void setPoints(int points);

	/** selects a window function */
	void setWindowFunction(Kwave::window_function_t type);

	/**
	 * sets the color mode. Currently only black/white (0) and
	 * rainbow color (1) are supported.
	 */
	void setColorMode(int color);

	/** enables/disables the "track changes" mode */
	void setTrackChanges(bool track_changes);

	/** enables/disables the "follow selection mode */
	void setFollowSelection(bool follow_selection);

	void setBoxPoints(int num);

    private slots:

	/** invoke the online help */
	void invokeHelp();

    private:

	/** length of the selection */
	sample_index_t m_length;

	/** sample rate of the signal */
	double m_rate;
    };
}

#endif /* SONAGRAM_DIALOG_H */

//***************************************************************************
//***************************************************************************
