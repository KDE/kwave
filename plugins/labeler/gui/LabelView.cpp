/***************************************************************************
    LabelView.cpp        - The label vizualizer for the plugin
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

#include <QPainter>

#include "gui/LabelView.h"  // relative to LabelerPlugin dir
#include "Labels.h"

#include "libkwave/MetaData.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/SignalManager.h"
using namespace Kwave;


//***************************************************************************
LabelView::LabelView(QWidget *parent, QWidget *controls, SignalManager *signal_manager,
               Location preferred_location, int track)
         : Kwave::SignalView(parent, controls, signal_manager, preferred_location, track)
{
    // get informed about selection changes
    connect(&(signal_manager->selection()),
            SIGNAL(changed(sample_index_t, sample_index_t)),
            this,
            SLOT(refreshSelectionLayer()));

    // update the playback position
    connect(&(signal_manager->playbackController()),
            SIGNAL(sigPlaybackPos(sample_index_t)),
            this,
            SLOT(needRepaint()));

//    // update when the track selection changed
//    connect(track, SIGNAL(sigSelectionChanged(bool)),
//            this,  SLOT(refreshSignalLayer()));
}

//***************************************************************************
void LabelView::paintEvent(QPaintEvent * event)
{
////// Just temporary
qDebug("LabelelView::paintEvent() called");
//qDebug(" - offset: %f", sample2float(offset()));
//qDebug(" - track:  %d", track());
qDebug(" - zoom:   %lf", zoom());
qDebug(" - offset: %llu",  offset());
//////

    SignalView::paintEvent(event);

    const sample_index_t offs  = offset();
    const sample_index_t wdth  = pixels2samples(width());

    // Paint the labels
    QPainter p;
    MetaDataList labels = m_signal_manager->metaData().selectByType(LabelerLabel::METADATA_TYPE);
//    float f = width() / static_cast<float>(m_signal_manager->length());

//    p.setFont(QFont());
    p.begin(this);

    /* Fill the background */
    p.fillRect(rect(), palette().background().color());
    /* Paint the labels */
    for (MetaDataList::iterator it = labels.begin(); it != labels.end(); it++) {
        // Time instant label
        if (it->property(LabelerLabel::LABELPROP_TYPE) == LabelInstant::LABEL_TYPE) {
            const LabelInstant label(it.value());

            // Not visible
            if (label.pos() < offs || label.pos() > offs + wdth)
                continue;
            // convert "first" to relative coordinate, clip left
            int x = samples2pixels(label.pos() - offs);
            QRect t = p.fontMetrics().boundingRect(label.name());
            t.moveCenter(QPoint(x, height() / 2));
            // draw the vertical line at the position of the label
            p.setPen(Qt::black);
            p.drawLine(x, 0, x, height());
            // draw the box with the label
            p.fillRect(t.left() -2, t.top() -2, t.width() + 2, t.height() + 2, QBrush(Qt::blue));
            p.setPen(Qt::white);
            p.drawText(t, label.name());

// label.dump();
// qDebug("pos: %d, f: %f, x: %f, width: %d\n", label.pos(), f, x, width());

            //p.drawRect()
        }
        if (it->property(LabelerLabel::LABELPROP_TYPE) == LabelRegion::LABEL_TYPE) {
            LabelRegion label(it.value());

            // TODO: finish it!!!
        }

    }

    p.end();
}

//***************************************************************************
void LabelView::setZoomAndOffset(double zoom, sample_index_t offset)
{
qDebug("LabelelView::setZoomAndOffset(%lf, %llu) called", zoom, offset);

    Kwave::SignalView::setZoomAndOffset(zoom, offset);
    repaint();
}

//***************************************************************************
#include "LabelView.moc"
//***************************************************************************
//***************************************************************************
