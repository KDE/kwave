/***************************************************************************
             Filter.cpp  -  parameters of a digital IIR or FIR filter
                             -------------------
    begin                : Jan 20 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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

#include "config.h"

#include <QFile>
#include <QLatin1Char>
#include <QString>
#include <QTextStream>

#include "libkwave/Filter.h"
#include "libkwave/Parser.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

//***************************************************************************
Kwave::Filter::Filter(const QString &command)
    :m_fir(true), m_rate(0), m_coeff(), m_delay()
{
    Kwave::Parser parse(command);

    m_rate = parse.toInt();
    m_fir = (parse.nextParam().toLower() == _("fir"));
    resize(parse.toInt());

    for (unsigned int i = 0; i < count(); i++) {
        m_delay[i] = parse.toInt();
        m_coeff[i] = parse.toDouble();
    }
}

//***************************************************************************
Kwave::Filter::Filter(int rate)
    :m_fir(true), m_rate(rate), m_coeff(), m_delay()
{
}

//***************************************************************************
Kwave::Filter::~Filter()
{
    resize(0);
}

//***************************************************************************
QString Kwave::Filter::command()
{
    QString s;

    s = _("filter (");
    s += QString::number(m_rate);
    s += QLatin1Char(',');
    s += _((m_fir) ? "fir" : "iir");
    s += QLatin1Char(',') + QString::number(count());

    for (unsigned int i = 0; i < count(); i++) {
        s += QLatin1Char(',');
        s += QString::number(m_delay[i]);
        s += QLatin1Char(',');
        s += QString::number(m_coeff[i]);
    }
    return s;
}

//***************************************************************************
unsigned int Kwave::Filter::resize(unsigned int newnum)
{
    unsigned int oldnum = count();
    if (newnum == oldnum) return oldnum; // nothing to do

    // resize both arrays
    m_delay.resize(newnum);
    m_coeff.resize(newnum);
    Q_ASSERT(m_delay.size() >= Kwave::toInt(newnum));
    Q_ASSERT(m_coeff.size() >= Kwave::toInt(newnum));
    Q_ASSERT(m_delay.size() == m_coeff.size());

    // initialize the new entries
    while (oldnum < newnum) {
        m_delay[oldnum] = oldnum;
        m_coeff[oldnum] = 0.0;
        oldnum++;
    }

    return newnum;
}

//***************************************************************************
unsigned int Kwave::Filter::count()
{
    Q_ASSERT(m_coeff.count() == m_delay.count());
    return static_cast<unsigned int>(m_coeff.count());
}

//***************************************************************************
double Kwave::Filter::coeff(unsigned int index)
{
    Q_ASSERT(Kwave::toInt(index) < m_coeff.count());
    return m_coeff[index];
}

//***************************************************************************
void Kwave::Filter::setCoeff(unsigned int index, double newval)
{
    Q_ASSERT(Kwave::toInt(index) < m_coeff.count());
    m_coeff[index] = newval;
}

//***************************************************************************
unsigned int Kwave::Filter::delay(unsigned int index)
{
    Q_ASSERT(Kwave::toInt(index) < m_delay.count());
    return m_delay[index];
}

//***************************************************************************
void Kwave::Filter::setDelay(unsigned int index, unsigned int newval)
{
    Q_ASSERT(Kwave::toInt(index) < m_delay.count());
    m_delay[index] = newval;
}

//***************************************************************************
void Kwave::Filter::save(const QString &filename)
{
    QString name(filename);
    Q_ASSERT(name.length());
    if (!name.length()) return;

    if (name.lastIndexOf(_(".filter")) != Kwave::toInt(name.length() - 7))
        name.append(_(".filter"));

    QFile file(name);
    if (!file.open(QIODevice::WriteOnly)) return;
    QTextStream out(&file);

    out << ((m_fir) ? "FIR " : "IIR ") << count() << Qt::endl;
    for (unsigned int i = 0; i < count(); i++) {
        out << m_delay[i] << ' ' << m_coeff[i] << Qt::endl;
    }

    file.close();
}

//***************************************************************************
void Kwave::Filter::load(const QString &filename)
{
    unsigned int i;
    bool ok;
    QString line;
    unsigned int linenr = 0;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return;
    QTextStream in(&file);

    // type of the filter (FIR/IIR)
    while (!in.atEnd()) {
        line = in.readLine().simplified();
        linenr++;

        if (line.isEmpty()) continue;
        if ((line[0] == QLatin1Char('#')) || (line[0] == QLatin1Char('/')))
            continue;
        break;
    }
    m_fir = line.startsWith(_("FIR "));
    qDebug("Filter::load(): fir = %d", m_fir);

    // order
    unsigned int order = line.remove(0,4).toUInt(&ok);
    resize(0);
    resize(order);
    qDebug("Filter::load(): order = %u", order);

    // read delays and coefficients
    i = 0;
    while (!in.atEnd()) {
        line = in.readLine().simplified();
        linenr++;

        if (line.isEmpty()) continue;
        if ((line[0] == QLatin1Char('#')) || (line[0] == QLatin1Char('/')))
            continue;

        qsizetype spacepos = line.indexOf(QLatin1Char(' '));
        ok = true;
        m_delay[i] = line.left(spacepos).toUInt(&ok);
        line.remove(0, spacepos);
        if (ok) m_coeff[i] = line.toDouble(&ok);

        if (ok) {
            i++;
        } else {
            qDebug("Filter::load(%s): syntax error in line %u",
                   DBG(filename), linenr);
        }
    }
}

//***************************************************************************
//***************************************************************************
