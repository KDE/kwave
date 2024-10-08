/*************************************************************************
       AsciiDecoder.cpp  -  decoder for ASCII data
                             -------------------
    begin                : Sun Dec 03 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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
#include "config.h"

#include <ctype.h>
#include <string.h>

#include <new>

#include <QDateTime>
#include <QIODevice>
#include <QLatin1Char>
#include <QLatin1String>
#include <QRegularExpression>

#include <KLocalizedString>

#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiWriter.h"
#include "libkwave/Parser.h"
#include "libkwave/Sample.h"
#include "libkwave/String.h"
#include "libkwave/Writer.h"

#include "AsciiCodecPlugin.h"
#include "AsciiDecoder.h"

#define MAX_LINE_LEN  16384 /**< maximum line length in characters */

//***************************************************************************
Kwave::AsciiDecoder::AsciiDecoder()
    :Kwave::Decoder(),
     m_source(),
     m_dest(nullptr),
     m_queue_input(),
     m_line_nr(0)
{
    LOAD_MIME_TYPES
    REGISTER_COMPRESSION_TYPES
}

//***************************************************************************
Kwave::AsciiDecoder::~AsciiDecoder()
{
    if (m_source.device()) close();
}

//***************************************************************************
Kwave::Decoder *Kwave::AsciiDecoder::instance()
{
    return new(std::nothrow) Kwave::AsciiDecoder();
}

//***************************************************************************
bool Kwave::AsciiDecoder::open(QWidget *widget, QIODevice &src)
{
    Q_UNUSED(widget)

    metaData().clear();
    Q_ASSERT(!m_source.device());
    if (m_source.device()) qWarning("AsciiDecoder::open(), already open !");

    // try to open the source
    if (!src.open(QIODevice::ReadOnly)) {
        qWarning("failed to open source !");
        return false;
    }

    // take over the source
    m_source.setDevice(&src);

    Kwave::FileInfo info(metaData());
    Kwave::LabelList labels;

    /********** Decoder setup ************/
    qDebug("--- AsciiDecoder::open() ---");

    // read in all metadata until start of samples, EOF or user cancel
    qDebug("AsciiDecoder::open(...)");

    m_line_nr = 0;
    while (!m_source.atEnd()) {
        QString line = m_source.readLine(MAX_LINE_LEN).simplified();
        m_line_nr++;
        if (!line.length())
            continue; // skip empty line

        QRegularExpression regex(QRegularExpression::anchoredPattern(_(
            "(^##\\s*)"                  // 1 start of meta data line
            "([\\'\\\"])?"               // 2 property, quote start (' or ")
            "\\s*(\\w+[\\s\\w]*\\w)\\s*" // 3 property
            "(\\[\\d*\\])?"              // 4 index (optional)
            "(\\2)"                      // 5 property, quote end
            "(\\s*=\\s*)"                // 6 assignment '='
            "(.*)"                       // 7 rest, up to end of line
        )));
        QRegularExpressionMatch regex_match{regex.match(line)};
        if (regex_match.hasMatch()) {
            // meta data entry: "## 'Name' = value"
            QString name = Kwave::Parser::unescape(regex_match.captured(3) +
                                                   regex_match.captured(4));
            QString v    = regex_match.captured(7);

            QString value;
            if (v.length()) {
                // remove quotes from the value
                bool is_escaped = false;
                char quote = v[0].toLatin1();
                if ((quote != '\'') && (quote != '"'))
                    quote = -1;

                for (QString::ConstIterator it = v.constBegin();
                     it != v.constEnd(); ++it)
                {
                    const char c = QChar(*it).toLatin1();

                    if ((c == '\\') && !is_escaped) {
                        is_escaped = true;   // next char is escaped
                        continue;
                    }
                    if (is_escaped) {
                        value += *it;        // escaped char
                        is_escaped = false;
                        continue;
                    }

                    if (c == quote) {
                        if (!value.length())
                            continue;        // starting quote
                        else
                            break;           // ending quote
                    }

                    if ((quote == -1) && (c == '#'))
                        break;               // comment in unquoted text

                    // otherwise: normal character, part of text
                    value += *it;
                }

                // if the text was unquoted, remove leading/trailing spaces
                if (quote == -1)
                    value = value.trimmed();
            }

            // handle some well known aliases
            if (name == _("rate")) name = info.name(INF_SAMPLE_RATE);
            if (name == _("bits")) name = info.name(INF_BITS_PER_SAMPLE);

            // handle labels
            QRegularExpression regex_label(
                QRegularExpression::anchoredPattern(_("label\\[(\\d*)\\]")));
            QRegularExpressionMatch regex_label_match{regex_label.match(name)};
            if (regex_label_match.hasMatch()) {
                bool ok = false;
                sample_index_t pos =
                    regex_label_match.captured(1).toULongLong(&ok);
                if (!ok) {
                    qWarning("line %llu: malformed label position: '%s'",
                              m_line_nr, DBG(name));
                    continue; // skip it
                }
                Kwave::Label label(pos, value);
                labels.append(label);
                continue;
            }

            bool found = false;
            foreach (const Kwave::FileProperty &p, info.allKnownProperties()) {
                if (info.name(p).toLower() == name.toLower()) {
                    found = true;
                    info.set(p, QVariant(value));
                }
            }
            if (!found) {
                qWarning("line %llu: unknown meta data entry: '%s' = '%s'",
                         m_line_nr, DBG(name), DBG(value));
            }
        } else if (line.startsWith(QLatin1Char('#'))) {
            continue; // skip comment lines
        } else {
            // reached end of metadata:
            // -> push back the line into the queue
            m_queue_input.enqueue(line);
            break;
        }
    }

    // if the number of channels is not known, but "tracks" is given and
    // "track" is not present: old syntax has been used
    if ((info.tracks() < 1) && info.contains(INF_TRACKS) &&
        !info.contains(INF_TRACK))
    {
        info.set(INF_CHANNELS, info.get(INF_TRACKS));
        info.set(INF_TRACKS, QVariant());
    }

    metaData().replace(Kwave::MetaDataList(info));
    metaData().add(labels.toMetaDataList());

    return (info.tracks() >= 1);
}

//***************************************************************************
bool Kwave::AsciiDecoder::readNextLine()
{
    if (!m_queue_input.isEmpty())
        return true; // there is still something in the queue

    while (!m_source.atEnd()) {
        QString line = m_source.readLine(MAX_LINE_LEN).simplified();
        m_line_nr++;
        if (!line.length()) {
            continue; // skip empty line
        } else if (line.startsWith(QLatin1Char('#'))) {
            continue; // skip comment lines
        } else {
            // -> push back the line into the queue
            m_queue_input.enqueue(line);
            return true;
        }
    }
    return false;
}

//***************************************************************************
bool Kwave::AsciiDecoder::decode(QWidget *widget,
                                 Kwave::MultiWriter &dst)
{
    Q_UNUSED(widget)

    Q_ASSERT(m_source.device());
    if (!m_source.device()) return false;

    m_dest = &dst;

    // for the moment: use a comma as separator <= TODO
    const char separators[] = {',', '\0' };

    Kwave::FileInfo info(metaData());
    unsigned int channels = info.tracks();
    QVector<sample_t> frame(channels);

    // read in all remaining data until EOF or user cancel
    qDebug("AsciiDecoder::decode(...)");
    while (readNextLine() && !dst.isCanceled()) {
        QByteArray d  = m_queue_input.dequeue().toLatin1();
        char *line    = d.data();
        char *saveptr = nullptr;

        frame.fill(0);
        for (unsigned int channel = 0; channel < channels; channel++) {
            sample_t  s = 0;

            char *token = strtok_r(line, separators, &saveptr);
            line = nullptr;
            if (token) {
                // skip whitespace at the start
                while (*token && isspace(*token)) ++token;
                if (*token) {
                    char *p = token + 1;
                    while (isdigit(*p) || (*p == '+') || (*p == '-')) ++p;
                    *p = 0;
                    s = atoi(token);
                    Kwave::Writer *w = dst[channel];
                    if (w) (*w) << s;
                }
            }
        }
    }

    m_dest = nullptr;
    info.setLength(dst.last() ? (dst.last() + 1) : 0);
    metaData().replace(Kwave::MetaDataList(info));

    // return with a valid Signal, even if the user pressed cancel !
    return true;
}

//***************************************************************************
void Kwave::AsciiDecoder::close()
{
    m_source.reset();
    m_source.setDevice(nullptr);
}

//***************************************************************************
//***************************************************************************
