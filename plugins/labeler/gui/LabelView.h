/***************************************************************************
    LabelView.h          - The label vizualizer for the plugin
                             -------------------
    begin                : Jan 02 2010
    copyright            : (C) 2010 by Daniel Tihelka
    email                : dtihelka@kky.zcu.cz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _LABEL_VIEW_H_
#define _LABEL_VIEW_H_

#include "config.h"

#include "libgui/SignalView.h"



/**
 * TODO: finish it!!!
 *
 */
class LabelView: public Kwave::SignalView
{
    Q_OBJECT

public:

    /**
     * Constructor, just calls the parent
     * @see Kwave::SignalView(QWidget, QWidget, SignalManager, Location, int)
     */
    LabelView(QWidget *parent, QWidget *controls, SignalManager *signal_manager,
               Location preferred_location, int track = -1);


protected:

    /** Reimplementation of Kwave::SignalView::paintEvent(QPaintEvent *)
     *  It TODO: finish!!!
     */
    virtual void paintEvent(QPaintEvent * event);

    /**
     * Reimplementation of Kwave::SignalView::setZoomAndOffset(double, sample_index_t);
     */
    virtual void setZoomAndOffset(double zoom, sample_index_t offset);


};

#endif /* _LABEL_VIEW_H_ */
