/*************************************************************************
          WavDecoder.h  -  decoder for wav data
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef WAV_DECODER_H
#define WAV_DECODER_H

#include "config.h"

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

#include "libkwave/Decoder.h"
#include "libkwave/FileInfo.h"

#include "WavPropertyMap.h"


namespace Kwave
{

    class RecoverySource;
    class RIFFChunk;
    class VirtualAudioFile;

    class WavDecoder: public Kwave::Decoder
    {
    public:
        /** Constructor */
        WavDecoder();

        /** Destructor */
        ~WavDecoder() override;

        /** Returns a new instance of the decoder */
        Kwave::Decoder *instance() override;

        /**
         * Opens the source and decodes the header information.
         * @param widget a widget that can be used for displaying
         *        message boxes or dialogs
         * @param source file or other source with a stream of bytes
         * @return true if succeeded, false on errors
         */
        bool open(QWidget *widget, QIODevice &source) override;

        /**
         * Decodes a stream of bytes into a MultiWriter
         * @param widget a widget that can be used for displaying
         *        message boxes or dialogs
         * @param dst MultiWriter that receives the audio data
         * @return true if succeeded, false on errors
         */
        virtual bool decode(QWidget *widget, Kwave::MultiWriter &dst)
            override;

        /**
         * Closes the source.
         */
        void close() override;

    protected:
        /**
         * Fix all inconsistencies and create a repar list.
         * @internal
         */
        bool repair(QList<Kwave::RecoverySource *> *repair_list,
                    Kwave::RIFFChunk *riff_chunk,
                    Kwave::RIFFChunk *fmt_chunk,
                    Kwave::RIFFChunk *data_chunk);

        /**
         * Adds a chunk to a repair list
         * @internal
         */
        bool repairChunk(QList<Kwave::RecoverySource *> *repair_list,
                         Kwave::RIFFChunk *chunk, quint32 &offset);

    private:

        /** adds an entry to m_known_chunks and to m_property_map */
        void addPropertyChunk(const Kwave::FileProperty property,
                              const QByteArray &chunk_name);

    private:

        /** source of the audio data */
        QIODevice *m_source;

        /** adapter for libaudiofile */
        Kwave::VirtualAudioFile *m_src_adapter;

        /** list of all known chunk names */
        QStringList m_known_chunks;

        /** map for translating chunk names to FileInfo properties */
        Kwave::WavPropertyMap m_property_map;

    };
}

#endif /* WAV_DECODER_H */

//***************************************************************************
//***************************************************************************
