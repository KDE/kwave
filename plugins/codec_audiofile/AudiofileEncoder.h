/*************************************************************************
   AudiofileEncoder.cpp  -  export through libaudiofile
                             -------------------
    begin                : Tue Aug 18 2026
    copyright            : (C) 2026 by Thomas Eschenbacher
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

#ifndef AUDIOFILE_ENCODER_H
#define AUDIOFILE_ENCODER_H

#include "config.h"

#include <QByteArray>
#include <QList>

#include "libkwave/Encoder.h"

class QIODevice;
class QWidget;

namespace Kwave
{
    class AFvirtualfile;
    class MetaDataList;
    class MultiTrackReader;
    class LabelList;

    class AudiofileEncoder: public Kwave::Encoder
    {
    public:
        /**
         * Constructor
         *
         * @param name the mime type's name
         * @param description verbose description
         * @param patterns list of file patterns, passed as a single string,
         *                 separated by "; "
         * @param format_id one of the AF_FILE_... constants
         */
        AudiofileEncoder(const char *name,
                         const QString &description,
                         const char *patterns,
                         int format_id);

        /** Destructor */
        ~AudiofileEncoder() override;

        /** Returns a new instance of the encoder */
        Kwave::Encoder::Instance instance() override;

        /**
         * Encodes audio data from Kwave into a QIODevice via libaudiofile.
         *
         * @param widget parent widget for dialogs / error messages
         * @param src MultiTrackReader used as source of the audio data
         * @param dst file or buffer to receive the encoded stream
         * @param meta_data meta data of the file to save
         * @return true if succeeded, false on error
         */
        bool encode(QWidget *widget, Kwave::MultiTrackReader &src,
                    QIODevice &dst,
                    const Kwave::MetaDataList &meta_data) override;

        /**
         * Returns a list of supported sample formats.
         * @param info the current meta data (for mime type)
         */
        QList<Kwave::SampleFormat::Format> supportedSampleFormats(
            const FileInfo &info) const override;

        /**
         * Returns a list of supported bit per sample.
         * @param info the current meta data (ignored)
         */
        QList<unsigned int> supportedBitsPerSample(
            const FileInfo &info) const override;

        /** Returns a list of supported file properties */
        QList<Kwave::FileProperty> supportedProperties() override;

    private:

        /* container type for "meta data" for use in audiofile */
        struct MiscChunk {
            int        type; /**< one of AF_MISC_... */
            QByteArray data; /**< UTF-8 encoded data */
        };

    private:

        /**
         * get the maximum number of supported markers
         * @return number of supported markers or 0 if no support
         */
        unsigned long int supportedMarkers() const;

        /**
         * prepare meta data in m_misc_chunks
         * @param setup the libaudiofile setup structure
         * @param meta_data meta data of the file to save
         */
        void prepareMetaData(AFfilesetup &setup,
                             const Kwave::MetaDataList &meta_data);

        /**
         * prepare audiofile for writing markers
         * @param setup the libaudiofile setup structure
         * @param labels a list of labels to write
         */
        void prepareMarkers(AFfilesetup &setup,
                            const Kwave::LabelList &labels);

        /**
         * write the meta data itself
         * @param fh libaudiofile file handle
         */
        void writeMetaData(AFfilehandle fh);

        /**
         * write markers
         * @param fh libaudiofile file handle
         * @param labels a list of labels to write
         */
        void writeMarkers(AFfilehandle fh,
                          const Kwave::LabelList &labels);

        /**
         * Creates an AFvirtualfile adapter wrapping a QIODevice
         */
        AFvirtualfile *createVirtualFile(QIODevice &device);

    private:

        /** file format ID for libaudiofile, AF_FILE_... */
        int m_format_id;

        /** container for meta data */
        QList<MiscChunk> m_misc_chunks;
    };

}

#endif /* AUDIOFILE_ENCODER_H */
