/*************************************************************************
    LabelPropertiesWidget.h  -  dialog for editing label properties
                             -------------------
    begin                : Sun Sep 03 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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

#ifndef LABEL_PROPERTIES_WIDGET_H
#define LABEL_PROPERTIES_WIDGET_H

#include "config.h"

#include <QtGlobal>
#include <QDialog>
#include <QObject>
#include <QString>

#include "libgui/ui_LabelPropertiesWidgetBase.h"
#include "libkwave/Sample.h"

namespace Kwave
{

    class Label;

    class Q_DECL_EXPORT LabelPropertiesWidget
	:public QDialog,
	 public Ui::LabelPropertiesWidgetBase
    {
	Q_OBJECT
    public:

	/** Constructor */
	explicit LabelPropertiesWidget(QWidget *parent);

	/** Destructor */
	virtual ~LabelPropertiesWidget();

	/**
	 * sets the index of the label (read-only property)
	 * @param index label index [0...N-1]
	 */
	virtual void setLabelIndex(unsigned int index);

	/**
	 * sets the position of the label
	 * @param pos the position of the label [samples]
	 * @param length length of the complete signal [samples]
	 * @param rate sample rate of the signal
	 */
	virtual void setLabelPosition(sample_index_t pos,
	                              sample_index_t length,
	                              double rate);

	/**
	 * sets the name/description of the label
	 * @param name the name (string) of the label
	 */
	virtual void setLabelName(const QString &name);

	/** returns the label position in samples */
	virtual sample_index_t labelPosition();

	/** returns the label's name */
	virtual QString labelName();

	/**
	 * save the dialog's settings. currently only the mode of the
	 * time selection.
	 */
	void saveSettings();

    private:

	/** length of the signal, for transforming percentage -> samples */
	sample_index_t m_length;

	/** sample rate, for transforming time -> samples */
	double m_sample_rate;

    };
}

#endif /* LABEL_PROPERTIES_WIDGET_H */

//***************************************************************************
//***************************************************************************
