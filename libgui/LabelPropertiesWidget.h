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

#ifndef _LABEL_PROPERTIES_WIDGET_H_
#define _LABEL_PROPERTIES_WIDGET_H_

#include "config.h"
#include <qobject.h>
#include <qdialog.h>
#include "libgui/LabelPropertiesWidgetBase.h"

class Label;

class LabelPropertiesWidget: public LabelPropertiesWidgetBase
{
    Q_OBJECT
public:

    /** Constructor */
    LabelPropertiesWidget(QWidget *parent, const char *name);

    /** Destructor */
    virtual ~LabelPropertiesWidget();

    /**
     * sets the index of the label (read-only property)
     * @param index label index [0...N-1]
     */
    virtual void setLabelIndex(unsigned int index);

    /**
     * sets the position of the label
     * @param index pos the position of the label [samples]
     * @param length length of the complete signal [samples]
     * @param rate sample rate of the signal
     */
    virtual void setLabelPosition(unsigned int pos,
	unsigned int length, double rate);

    /**
     * sets the name/description of the label
     * @param name the name (string) of the label
     */
    virtual void setLabelName(const QString &name);

    /** returns the label position in samples */
    virtual unsigned int labelPosition();

    /** returns the label's name */
    virtual QString labelName();

    /**
     * save the dialog's settings. currently only the mode of the
     * time selection.
     */
    void saveSettings();

private:

    /** length of the signal, for transforming percentage -> samples */
    unsigned int m_length;

    /** sample rate, for transforming time -> samples */
    double m_sample_rate;

};

#endif /* _LABEL_PROPERTIES_WIDGET_H_ */
