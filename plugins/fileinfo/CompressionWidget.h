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
#include "CompressionWidgetBase.uih.h"

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

    /** Enable or disable ABR mode */
    virtual void enableABR(bool enable);

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
    
    /**
     * Returns the bitrate settings of ABR mode
     * @param nominal receives the nominal bitrate
     * @param lower receives the lowest bitrate or null if unused
     * @param upper receives the highest bitrate or null if unused
     */
    virtual void getABRrates(int &nominal, int &lower, int &upper);

    /**
     * Returns the base quality in VBR mode, as
     * percentage from 1...100
     */
    virtual int baseQuality();

private slots:

    /** called when the selection state of the ABR radio button changed */
    virtual void selectABR(bool checked);

};

#endif /* _COMPRESSION_WIDGET_H_ */

