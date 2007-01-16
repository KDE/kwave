/***************************************************************************
  CompressionWidget.cpp  -  widget for setting ogg or mp3 compression rates
                             -------------------
    begin                : Sat Jun 14 2003
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

#ifndef _COMPRESSION_WIDGET_H_
#define _COMPRESSION_WIDGET_H_

#include "config.h"
#include <qobject.h>
#include "libkwave/FileInfo.h"
#include "CompressionWidgetBase.uih.h"

class FileInfo;
class QWidget;

class CompressionWidget: public CompressionWidgetBase
{
    Q_OBJECT
public:
    /** Compression Mode: ABR or VBR */
    typedef enum {
	ABR_MODE = 0, /**< average bitrate mode */
	VBR_MODE      /**< variable bitrate mode */
    } Mode;

    /** Constructor */
    CompressionWidget(QWidget *parent, const char *name);

    /** Destructor */
    virtual ~CompressionWidget();

    /**
     * @param info the FileInfo used for getting the property descriptions
     */
    virtual void init(FileInfo &info);

    /**
     * Enable or disable ABR mode controls
     * @param enable controls the global ABR mode setting enable/disable
     * @param lowest checks/unchecks the lowest bitrate setting checkbox
     * @param highest checks/unchecks the highest bitrate setting checkbox
     */
    virtual void enableABR(bool enable, bool lowest, bool highest);

    /** Enable or disable VBR mode */
    virtual void enableVBR(bool enable);


    /**
     * Sets the bitrates in ABR mode
     * @param nominal the nominal bitrate or zero if not used
     * @param lower the lowest bitrate or zero if not used
     * @param upper the highest bitrate or zero if not used
     */
    virtual void setBitrates(int nominal, int lower, int upper);

    /**
     * Sets the VBR base quality or zero if unused.
     */
    virtual void setQuality(int quality);

    /** Returns the current bitrate mode: ABR or VBR */
    virtual Mode mode();

    /** Selects ABR or VBR mode */
    virtual void setMode(Mode mode);

    /** Returns the state of the "use lowest" checkbox */
    virtual bool lowestEnabled();

    /** Returns the state of the "use highest" checkbox */
    virtual bool highestEnabled();

    /**
     * Returns the bitrate settings of ABR mode
     * @param nominal receives the nominal bitrate
     * @param lowest receives the lowest bitrate or null if unused
     * @param highest receives the highest bitrate or null if unused
     */
    virtual void getABRrates(int &nominal, int &lowest, int &highest);

    /**
     * Returns the base quality in VBR mode, as
     * percentage from 1...100
     */
    virtual int baseQuality();

private slots:

    /** called when the selection state of the ABR radio button changed */
    virtual void selectABR(bool checked);

    /** called when the "lowest bitrate" checkbox has been toggled */
    virtual void lowestToggled(bool on);

    /** called when the "average bitrate" slider has changed */
    virtual void abrChanged(int value);

    /** called when the "lowest bitrate" slider has changed */
    virtual void lowestChanged(int value);

    /** called when the "highest bitrate" slider has changed */
    virtual void highestChanged(int value);

    /** called when the "highest bitrate" checkbox has been toggled */
    virtual void highestToggled(bool on);

private:

    /**
     * Sets the tooltip and "what's this" of a widget.
     * @param widget any QWidget derived widget
     * @param name of the setting, normally equal to it's label
     * @param description verbose descriptive text that says
     *        what can be set
     */
    void describeWidget(QWidget *widget, const QString &name,
                        const QString &description);

    /**
     * Sets the text of the label to the name of a file property and
     * initializes the tool tip of the corresponding edit/display control.
     * @param label the label to be set
     * @param widget the control to get the tool tip
     * @param property the file property which it belongs to
     * @param info the FileInfo used for getting the property descriptions
     */
    void initInfo(QLabel *label, QWidget *widget, FileProperty property,
                  FileInfo &info);


};

#endif /* _COMPRESSION_WIDGET_H_ */

