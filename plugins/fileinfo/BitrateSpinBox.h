/***************************************************************************
       BitrateSpinBox.h  -  spinbox for selecting a bitrate for MP3 or Ogg
                             -------------------
    begin                : Thu Oct 24 2002
    copyright            : (C) 2002 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BITRATE_SPIN_BOX_H
#define BITRATE_SPIN_BOX_H

#include "config.h"

#include <QList>
#include <QObject>
#include <QSpinBox>

namespace Kwave
{

    class BitrateSpinBox: public QSpinBox
    {
        Q_OBJECT
    public:
        /** Constructor */
        explicit BitrateSpinBox(QWidget *parent);

        /** Destructor */
        ~BitrateSpinBox() override;

        /** sets a list of allowed bitrates */
        virtual void allowRates(const QList<int> &list);

    signals:

        /** emitted when the value changed and snapped in to a bitrate */
        void snappedIn(int bitrate);

    public slots:

        /** snaps in to a new value */
        virtual void snapIn(int value);

    protected:

        /** find the nearest bitrate index of a current position */
        int nearestIndex(int rate);

    private:

        /** list of allowed bitrates, sorted ascending */
        QList<int> m_rates;

    };
}

#endif /* BITRATE_SPIN_BOX_H */

//***************************************************************************
//***************************************************************************
