/*************************************************************************
           OpusHeader.h  -  Opus stream header
                             -------------------
    begin                : Wed Dec 26 2012
    copyright            : (C) 2012 by Thomas Eschenbacher
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

#ifndef OPUS_HEADER_H
#define OPUS_HEADER_H

#include "config.h"

#include <QtGlobal>

namespace Kwave
{
    typedef struct {
	quint8  magic[8];        /**< stream magic, must be "OpusHead" */
	quint8  version;         /**< version number 2:6 bits          */
	quint8  channels;        /**< channels, 1...255                */
	quint16 preskip;         /**< preskip                          */
	quint32 sample_rate;     /**< sample rate [samples/sec]        */
	quint16 gain;            /**< gain                             */
	quint8  channel_mapping; /**< channel mapping used, 0 or 1     */

	/* only present if channel_mapping != 0 */
	quint8 streams;          /**< number of streams                */
	quint8 coupled;          /**< number of couplings              */
	quint8 map[255];         /**< stream to channel map            */
    } Q_PACKED opus_header_t;
}

#endif /* OPUS_HEADER_H */

//***************************************************************************
//***************************************************************************
