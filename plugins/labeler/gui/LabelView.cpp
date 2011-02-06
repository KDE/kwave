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
#include <QDebug>

#include "gui/LabelView.h"  // relative to LabelerPlugin dir

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
  // TODO: optimize the painting (seems to be quiet complex code). For example, for given zoom, the whole
  //       labels range could be paint to hidden graphics once, and the visible region could be displayed
  //       only (depending to the current offset)

    SignalView::paintEvent(event);

    const sample_index_t offs  = offset();
    const sample_index_t wdth  = pixels2samples(width());
    const static QChar dots = QChar(0x85); // '...' character

    // Paint the labels
    QPainter painter;
    MetaDataList labels = m_signal_manager->metaData().selectByType("Label"); // TODO: from a constant?

//    p.setFont(QFont());
    painter.begin(this);

    /* Fill the background */
    painter.fillRect(rect(), palette().background().color());
    /* Paint the labels */
    for (MetaDataList::iterator it = labels.begin(); it != labels.end(); it++) {
         // Whole label is not visible
         if (it->lastSample() < offs || it->firstSample() > offs + wdth)
             continue;

         // The rectangle occupied by the text of the label and the rectangle into which the
         // label will be paint
         QString lbltext = it->property(Kwave::MetaData::STDPROP_DESCRIPTION).toString();
         QRect lblrect = painter.fontMetrics().boundingRect(lbltext);
         QRect lbldraw(0, (height() - lblrect.height()) / 2 -2, wdth, lblrect.height() +2);

         painter.setPen(Qt::black); // TODO: colour configutable?

         // label spanning a range of samples
         if (it->scope() == Kwave::MetaData::Range) {
            // The vertical line at the beginning of the label
            if (it->firstSample() >= offs) {
                lbldraw.setLeft(samples2pixels(it->firstSample() - offs));
                painter.drawLine(lbldraw.left(), 0, lbldraw.left(), height());
            }
            // The vertical line at the end of the label
            if (it->lastSample() < offs + wdth) {
                lbldraw.setRight(samples2pixels(it->lastSample() - offs));
                painter.drawLine(lbldraw.right(), 0, lbldraw.right(), height());
            }
            // Lower the rectangle little bit
            lbldraw.setLeft(lbldraw.left() +2);
            lbldraw.setRight(lbldraw.right() -2);
         }
         // label assigned to a given position
         if (it->scope() == Kwave::MetaData::Position) {
            lbldraw.moveLeft(samples2pixels(it->firstSample() - offs));
            painter.drawLine(lbldraw.left(), 0, lbldraw.left(), height());
            // centre the label box to the label time
            lbldraw.setWidth(lblrect.width() + 4);
            lbldraw.moveLeft(lbldraw.left() - lbldraw.width() /2);
            // correct the width of the rectangle
            if (lbldraw.width() > width())
                lbldraw.setWidth(width());
            // correct the left edge
            if (lbldraw.left() < 0)
                lbldraw.moveLeft(0);
            // correct the right edge
            if (lbldraw.right() >= width())
                lbldraw.moveRight(width() -1);
         }
         lblrect.moveCenter(lbldraw.center());

         // Shorten the label's text until it will match the rectangle
         while (lbltext.length() > 0 && lblrect.width() > lbldraw.width()) {
                if (lbltext.length() <= 2)
                    lbltext.clear();
                else {
                    lbltext.truncate(lbltext.length() / 2);
                    lbltext.append(dots);
                }
                lblrect = painter.fontMetrics().boundingRect(lbltext);
         }

         // draw the box with the label
         painter.fillRect(lbldraw, QBrush(Qt::blue)); // TODO: colour configutable?
         painter.setPen(Qt::white);
         painter.drawText(lblrect, lbltext);
    }

    painter.end();
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
