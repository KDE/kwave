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
#include "libkwave/MetaDataList.h"



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


private :

    /** Class representing a region of the signal */
    class SRegion
    {
    public:

      SRegion()
      {
        m_beg = m_len = 0;
      }
      SRegion(sample_index_t beg, sample_index_t len)
      {
        m_beg = beg;
        m_len = len;
      }
      SRegion(const SRegion & region)
      {
        m_beg = region.m_beg;
        m_len = region.m_len;
      }

      /** Gets the first sample on the region */
      sample_index_t left() const
      {
        return m_beg;
      }
      /** Gets the last sample on the region */
      sample_index_t right() const
      {
        return m_beg + m_len -1;
      }

    private:

      sample_index_t m_beg; /**< The index of the first sample in the region */
      sample_index_t m_len; /**< The number of samples in the region */

    };

    /** In this view it paints vertical position mark lines for the given labels. For the labels
     *  which are visible (from all the labels set), i.e. the mark lines are paint at their
     *  positions, the method sets <code>LBRPROP_VIEW_*PIXEL</code> properties set to the
     *  x-xoordinate of the line for the related property.
     *  @param painter the instance of painter to paint into
     *  @param labels the iterator through labels for which to paint the marks (the visible
     *         labels are detected in the method)
     *  @param paintregion limits the region to paint into; usually set to <code>this->rect()</code>
     */
    void paintVMarks(QPainter & painter, Kwave::MetaDataList::Iterator labels,
                     const QRect paintregion);
    /** In this view it paints the boxes with label description for the given labels. The
     *  visibility of labels is determined from the existence of<code>LBRPROP_VIEW_*PIXEL</code>
     *  properties of the labels.
     *  @param painter the instance of painter to paint into
     *  @param labels the iterator through labels for which to paint the marks (the visible
     *         labels are detected in the method)
     *  @param paintregion limits the region to paint into; limit the top and the bottom of the
     *         region to the coordinates to the strip into which boxes will be paint, the width
     *         must be equal to the region used for ::paintVMarks()
     *  @see ::paintVMarks()
     *  @see ::setPaintPos()
     */
    void paintLabels(QPainter & painter, Kwave::MetaDataList::Iterator labels,
                     const QRect paintregion);

    /** For the higher-level labels (those with Kwave::MetaData::STDPROP_TYPE equal to
     *  "Label[level > 0]" // TODO: to a constant
     *  updates (or sets) their <code>LBRPROP_VIEW_*PIXEL</code> position properties according
     *  to the value of the 0-th label which the current label is aligned to. The method must
     *  be called after ::paintVMarks()
     *  @param labels the iterator through labels for which to set/update the paint positions
     *  @param paintregion the region to paint into, used to determine x-axe constraints; the width
     *         must be equal to the region used for ::paintVMarks()
     *  @see ::paintVMarks()
     */
    void setPaintPos(Kwave::MetaDataList::Iterator labels, const QRect paintregion);



    /** The region of signal visible for the given zoom and offset. The variable is updated by
     *  ::setZoomAndOffset(double, sample_index_t) method and should only be read in other places. */
    SRegion m_visibleSignal;

    /** Property (stored to range-scope labels) holding the position of the label's beginning
     *  in this view [in pixels]. The value is of <code>int</code> type, and is defined only if
     *  the the label's start is visible for the given zoom and offset. <b>The property is for
     *  internal use only!</b> */
    static const QString LBRPROP_VIEW_BEGPIXEL;
    /** Property (stored to range-scope labels) holding the position of the label's end
     *  in this view [in pixels]. The value is of <code>int</code> type, and is defined only if
     *  the the label's end is visible for the given zoom and offset. <b>The property is for
     *  internal use only!</b> */
    static const QString LBRPROP_VIEW_ENDPIXEL;
    /** Property (stored to positions-scope labels) holding the position of the label in this
     *  view [in pixels]. The value is of <code>int</code> type, and is defined only if
     *  the the label is visible for the given zoom and offset. <b>The property is for internal
     *  use only!</b> */
    static const QString LBRPROP_VIEW_POSPIXEL;

};

#endif /* _LABEL_VIEW_H_ */
