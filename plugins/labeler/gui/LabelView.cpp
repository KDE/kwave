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

#include "LabelerPlugin.h"
#include "gui/LabelView.h"  // relative to LabelerPlugin dir

#include <libkwave/MessageBox.h>
#include "libkwave/MetaData.h"
#include "libkwave/MetaDataList.h"
#include "libkwave/SignalManager.h"
using namespace Kwave;


/** Macro used to create LBRPROP_VIEW_*PIXEL property keys. The "derivation" of the
 *  property names simplifies the name resolving in case of higher-level labels which
 *  no not contain the positions, but reffers to the position of the 0-th level
 *  labels they are aligned with. Use the macro to get LBRPROP_VIEW_(BEG|END|POS)PIXEL
 *  property names from Kwave::MetaData::STDPROP_(START|END|POS) property names. */
#define PROP_PIXELPOS(name) (name + "_VIEW_PIXEL")

//***************************************************************************
// initializers of the standard property names
const QString LabelView::LBRPROP_VIEW_BEGPIXEL(PROP_PIXELPOS(Kwave::MetaData::STDPROP_START));
const QString LabelView::LBRPROP_VIEW_ENDPIXEL(PROP_PIXELPOS(Kwave::MetaData::STDPROP_END));
const QString LabelView::LBRPROP_VIEW_POSPIXEL(PROP_PIXELPOS(Kwave::MetaData::STDPROP_POS));

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
  static const int levelshspace = 3; // the height of horizontal space between levels TODO: make it cofigurable

  // TODO: optimize the painting (seems to be quiet complex code). For example, for given zoom, the whole
  //       labels range could be paint to hidden graphics once, and the visible region could be displayed
  //       only (depending to the current offset)

    SignalView::paintEvent(event);

    // Paint the labels
    QPainter painter;
    QRect labelsregion = this->rect();
    MetaDataList & metadata = m_signal_manager->metaData();
    MetaDataList labels;

    // The number of levels:
    labels = metadata.selectByScope(Kwave::MetaData::None).selectByProperty(LabelerPlugin::LBRPROP_NUMLEVELS);
    // not set, do not paint ...
    if (labels.size() == 0)
        return;

    // Initialize the painter
    painter.begin(this);
    painter.fillRect(rect(), palette().background().color());

    // Set the height of name-box
    labelsregion.setHeight(painter.fontMetrics().boundingRect("Jj").height() + 4); // +4 is for space above and under

    int numlevels = labels.begin().value()[LabelerPlugin::LBRPROP_NUMLEVELS].toInt();
    int lblsheigt;
    // How many levels can be paint?
    while (height() < (lblsheigt = labelsregion.height() * numlevels + levelshspace * (numlevels -1))) {
        numlevels--;
    }
    // Not all levels can be paint!!
    if (numlevels != labels.begin().value()[LabelerPlugin::LBRPROP_NUMLEVELS].toInt()) {
        Kwave::MessageBox::sorry(this, i18n("Too many label levels to paint. Only %d will be shown").arg(numlevels));
    }
    // Set the top and bottom horizontal space
    labelsregion.moveTop((height() - lblsheigt) / 2);

    // The labels from the 0th level
    labels = metadata.selectByType("Label");  // TODO: string to a constant?
    // Paint the label markers and their name-boxes
    paintVMarks(painter, MetaDataList::Iterator(labels), this->rect());
    paintLabels(painter, MetaDataList::Iterator(labels), labelsregion);

//////////////////  !!!! TRICKY CODE !!!! force the labels back to the signal manager.
///////////////                           We need the position properties be there ...
    MetaDataList::Iterator ii(labels);
    while (ii.hasNext()) {
         ii.next();
         m_signal_manager->metaData().add(ii.value());
    }
///////////////
//////////////////

    for (int level = 1; level < numlevels; level++) {
        labels = metadata.selectByValue(LabelerPlugin::LBRPROP_LEVEL, level);
        // set new paint region
        labelsregion.moveTop(labelsregion.bottom() + levelshspace);
        // Set the draw positions to the labels and paint their name-boxes
        setPaintPos(MetaDataList::Iterator(labels), labelsregion);
        paintLabels(painter, MetaDataList::Iterator(labels), labelsregion);
    }

    painter.end();
}

//***************************************************************************
void LabelView::paintVMarks(QPainter & painter, MetaDataList::Iterator labels, const QRect paintregion)
{
    // Paint the markers
    while (labels.hasNext()) {
         labels.next();
         Kwave::MetaData & label = const_cast<Kwave::MetaData &>(labels.value()); // need non-const reference :-(

                                                                                         // TODO: string to a constant!!
         Q_ASSERT(label.hasProperty(Kwave::MetaData::STDPROP_TYPE) && label[Kwave::MetaData::STDPROP_TYPE] == "Label");

         // Whole label is not visible, ignore it
         if (label.lastSample() < m_visibleSignal.left() || label.firstSample() > m_visibleSignal.right()) {
             label.setProperty(LBRPROP_VIEW_BEGPIXEL, QVariant::Invalid);
             label.setProperty(LBRPROP_VIEW_ENDPIXEL, QVariant::Invalid);
             label.setProperty(LBRPROP_VIEW_POSPIXEL, QVariant::Invalid);
             continue;
         }

         // At least part of the label is visible
         painter.setPen(Qt::black); // TODO: colour configurable?

         // label spanning a range of samples
         if (label.scope() == Kwave::MetaData::Range) {
            // The vertical marker at the beginning of the label
            if (label.firstSample() >= m_visibleSignal.left()) {
                int mark = samples2pixels(label.firstSample() - m_visibleSignal.left());
                label.setProperty(LBRPROP_VIEW_BEGPIXEL, QVariant(mark));
                painter.drawLine(mark, paintregion.top(), mark, paintregion.height());
            }
            else {
                // start at the beginning of the view
                label.setProperty(LBRPROP_VIEW_BEGPIXEL, paintregion.left());
            }
            // The vertical marker at the end of the label
            if (label.lastSample() < m_visibleSignal.right()) {
                int mark = samples2pixels(label.lastSample() - m_visibleSignal.left());
                label.setProperty(LBRPROP_VIEW_ENDPIXEL, QVariant(mark));
                painter.drawLine(mark, paintregion.top(), mark, paintregion.height());
            }
            else {
                // finish at the end of the view
                label.setProperty(LBRPROP_VIEW_ENDPIXEL, paintregion.right());
            }
         }
         // label assigned to a given position
         if (label.scope() == Kwave::MetaData::Position) {
            int mark = samples2pixels(label.firstSample() - m_visibleSignal.left());
            label.setProperty(LBRPROP_VIEW_POSPIXEL, QVariant(mark));
            painter.drawLine(mark, paintregion.top(), mark, paintregion.height());
         }
    }
}

//***************************************************************************
void LabelView::paintLabels(QPainter & painter, MetaDataList::Iterator labels, const QRect paintregion)
{
    const static QChar dots = QChar(0x2026); // '...' character

    // Paint the box-decorated labels
    while (labels.hasNext()) {
         labels.next();
         const Kwave::MetaData & label = labels.value();

         Q_ASSERT(label.hasProperty(Kwave::MetaData::STDPROP_TYPE) &&  // TODO: string to a constant!!
                 (label[Kwave::MetaData::STDPROP_TYPE] == "Label" || label[Kwave::MetaData::STDPROP_TYPE] == "Label[level > 0]"));
         Q_ASSERT(label.hasProperty(LBRPROP_VIEW_VISIBLE));

         // Whole label is not visible. Ignore it.
         if (label.scope() == Kwave::MetaData::Range && ! label.hasProperty(LBRPROP_VIEW_POSPIXEL) && ! label.hasProperty(LBRPROP_VIEW_BEGPIXEL))
             continue;
         if (label.scope() == Kwave::MetaData::Position && ! label.hasProperty(LBRPROP_VIEW_POSPIXEL))
             continue;

         // The rectangle occupied by the text of the label and the rectangle into which the
         // label will be paint
         QString lbltext = label[Kwave::MetaData::STDPROP_DESCRIPTION].toString();
         QRect lblbound = painter.fontMetrics().boundingRect(lbltext);
         QRect lblpaint(paintregion);

         // If the label's box height is too small, do not paint the labels ...
         if (lblbound.height() > lblpaint.height())
            return;

         // label spanning a range of samples
         if (label.scope() == Kwave::MetaData::Range) {
            int begPixel = label.hasProperty(LBRPROP_VIEW_BEGPIXEL) ? label[LBRPROP_VIEW_BEGPIXEL].toInt() +2 : paintregion.left();
            int endPixel = label.hasProperty(LBRPROP_VIEW_ENDPIXEL) ? label[LBRPROP_VIEW_ENDPIXEL].toInt() -2 : paintregion.right();

            // Lower the label rectangle not to be paint cross the marks
            lblpaint.setLeft(begPixel);
            lblpaint.setRight(endPixel);
         }
         // label assigned to a given position
         if (label.scope() == Kwave::MetaData::Position) {
            int posPixel = label[LBRPROP_VIEW_POSPIXEL].toInt();

            lblpaint.setWidth(lblbound.width() +4);
            lblpaint.moveLeft(posPixel - lblpaint.width() /2);

            // Limit the box by the paint region
            int minx = paintregion.left();
            int maxx = paintregion.width() -1;
            // Limit the box not to overlap the previous label box
            // TODO: finish it! It does not work, since iterator's peekPrevious() and peekNext() methods
            //       do not return the labels preceding/succeeding the current label!!
            //if (labels.hasPrevious()) {
            //  const Kwave::MetaData & plabel = labels.peekPrevious().value();
            //  minx = qMax(plabel.hasProperty(LBRPROP_VIEW_ENDPIXEL) ? plabel[LBRPROP_VIEW_ENDPIXEL].toInt() +2 : minx, minx);
            //}
            //if (labels.hasNext()) {
            //  const Kwave::MetaData & nlabel = labels.peekNext().value();
            //  maxx = qMin(nlabel.hasProperty(LBRPROP_VIEW_BEGPIXEL) ? nlabel[LBRPROP_VIEW_BEGPIXEL].toInt() -2 : maxx, minx);
            //}
            // correct the left edge
            if (lblpaint.left() < minx)
                lblpaint.moveLeft(minx);
            // correct the right edge
            if (lblpaint.right() > maxx)
                lblpaint.moveRight(maxx);
         }
         // If the label's box is too small, do not paint it ...
         if (lblpaint.width() <= 2)
             continue;

         // Shorten the label's text until it will match the rectangle
         while (lbltext.length() > 0 && lblbound.width() > lblpaint.width() -2) {
                if (lbltext.length() <= 2)
                    lbltext.clear();
                else {
                    lbltext.truncate(lbltext.length() / 2);
                    lbltext.append(dots);
                }
                lblbound = painter.fontMetrics().boundingRect(lbltext);
         }
         // Centre the rectangle bounding the label with respects to the rectangle into which it will
         // be paint
         lblbound.moveCenter(lblpaint.center());

         // draw the box with the label
         painter.fillRect(lblpaint, QBrush(Qt::blue)); // TODO: colour configutable?
         // paint the labels only if height is OK
         if (lblpaint.height() >= lblbound.height()) {
            painter.setPen(Qt::white);
            painter.drawText(lblbound, lbltext);
         }
    }
}


//***************************************************************************
void LabelView::setZoomAndOffset(double zoom, sample_index_t offset)
{
qDebug("LabelelView::setZoomAndOffset(%lf, %llu) called", zoom, offset);

    Kwave::SignalView::setZoomAndOffset(zoom, offset);
    m_visibleSignal = SRegion(offset, pixels2samples(width()));
    repaint();
}

//***************************************************************************
void LabelView::setPaintPos(MetaDataList::Iterator labels, const QRect paintregion)
{
    // Go through all the labels
    while (labels.hasNext()) {
         labels.next();
         Kwave::MetaData & label = const_cast<Kwave::MetaData &>(labels.value()); // need non-const reference :-(

         Q_ASSERT(label.hasProperty(Kwave::MetaData::STDPROP_TYPE));
         Q_ASSERT(label[Kwave::MetaData::STDPROP_TYPE] == LabelerPlugin::LBRPROP_TYPEVAL);

         // copy the necessary position values
         if (label.scope() == Kwave::MetaData::Range) {
             // must have the reference properties
             Q_ASSERT(label.hasProperty(LabelerPlugin::LBRPROP_START_REFID));
             Q_ASSERT(label.hasProperty(LabelerPlugin::LBRPROP_START_REFPOS));
             Q_ASSERT(label.hasProperty(LabelerPlugin::LBRPROP_END_REFID));
             Q_ASSERT(label.hasProperty(LabelerPlugin::LBRPROP_END_REFPOS));
             // the labels from 0-th level this is referred to must exist
             Q_ASSERT(m_signal_manager->metaData().contains(label[LabelerPlugin::LBRPROP_START_REFID]));
             Q_ASSERT(m_signal_manager->metaData().contains(label[LabelerPlugin::LBRPROP_END_REFID]));

             // get the reference to the 0-th level bounding labels
             Kwave::MetaData & b = m_signal_manager->metaData()[label[LabelerPlugin::LBRPROP_START_REFID].toString()];
             Kwave::MetaData & e = m_signal_manager->metaData()[label[LabelerPlugin::LBRPROP_END_REFID].toString()];
             QString bsamplprop(label[LabelerPlugin::LBRPROP_START_REFPOS].toString());
             QString esamplprop(label[LabelerPlugin::LBRPROP_END_REFPOS].toString());
             // not visible, ignore
             if (e[esamplprop].toULongLong() < m_visibleSignal.left() || b[bsamplprop].toULongLong() > m_visibleSignal.right()) {
                  label.setProperty(LBRPROP_VIEW_BEGPIXEL, QVariant::Invalid);
                  label.setProperty(LBRPROP_VIEW_ENDPIXEL, QVariant::Invalid);
                  continue;
             }

             // pixel position properties
             QString bpixelprop(PROP_PIXELPOS(label[LabelerPlugin::LBRPROP_START_REFPOS].toString()));
             QString epixelprop(PROP_PIXELPOS(label[LabelerPlugin::LBRPROP_END_REFPOS].toString()));
             // set the boundary pixels directly to the label
             label.setProperty(LBRPROP_VIEW_BEGPIXEL, b.hasProperty(bpixelprop) ? b[bpixelprop] : paintregion.left());
             label.setProperty(LBRPROP_VIEW_ENDPIXEL, e.hasProperty(epixelprop) ? e[epixelprop] : paintregion.right());
         }
         if (label.scope() == Kwave::MetaData::Position) {
             // must have the reference properties
             Q_ASSERT(label.hasProperty(LabelerPlugin::LBRPROP_POS_REFID));
             Q_ASSERT(label.hasProperty(LabelerPlugin::LBRPROP_POS_REFPOS));
             // the labels from 0-th level this is referred to must exist
             Q_ASSERT(m_signal_manager->metaData().contains(label[LabelerPlugin::LBRPROP_POS_REFID]));

             // get the reference to the 0-th level label
             Kwave::MetaData & l = m_signal_manager->metaData()[label[LabelerPlugin::LBRPROP_POS_REFID].toString()];
             QString samplprop(label[LabelerPlugin::LBRPROP_POS_REFPOS].toString());
             // not visible, ignore
             if (l[samplprop].toULongLong() < m_visibleSignal.left() || l[samplprop].toULongLong() > m_visibleSignal.right()) {
                  label.setProperty(LBRPROP_VIEW_POSPIXEL, QVariant::Invalid);
                  continue;
             }

             QString pixelprop(PROP_PIXELPOS(label[LabelerPlugin::LBRPROP_POS_REFPOS].toString()));
             // set the boundary pixels directly to the label
             label.setProperty(LBRPROP_VIEW_POSPIXEL, l.hasProperty(pixelprop) ? l[pixelprop] : QVariant::Invalid);
         }
    }
}

//***************************************************************************
#include "LabelView.moc"
//***************************************************************************
//***************************************************************************
