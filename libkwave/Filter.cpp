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

#include <stdlib.h>

#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>

#include <kurl.h>

#include "Filter.h"
#include "Parser.h"

//***************************************************************************
Filter::Filter(const QString &command)
:m_fir(true), m_rate(0), m_coeff(), m_delay()
{
    Parser parse (command);

    m_rate = parse.toInt();
    m_fir = (parse.nextParam().lower() == "fir");
    resize(parse.toInt());

    for (unsigned int i = 0; i < count(); i++) {
	m_delay[i] = parse.toInt();
	m_coeff[i] = parse.toDouble();
    }
}

//***************************************************************************
Filter::Filter(int rate)
:m_fir(true), m_rate(rate), m_coeff(), m_delay()
{
}

//***************************************************************************
Filter::~Filter()
{
    resize(0);
}

//***************************************************************************
QString Filter::command()
{
    QString s;

    s = "filter (";
    s += QString::number(m_rate);
    s += ',';
    s += (m_fir ? "fir" : "iir");
    s += ',' + QString::number(count());
			
    for (unsigned int i = 0; i < count(); i++) {
	s += ',';
	s += QString::number(m_delay[i]);
	s += ',';
	s += QString::number(m_coeff[i]);
    }
    return s;
}

//***************************************************************************
unsigned int Filter::resize(unsigned int newnum)
{
    unsigned int oldnum = count();
    if (newnum == oldnum) return oldnum; // nothing to do

    // resize both arrays
    if (!(m_delay.resize(newnum)) && (m_coeff.resize(newnum))) {
	// restore previous state if one of them failed
	debug("Filter::resize(%d) failed.", newnum);
	m_delay.resize(oldnum);
	m_coeff.resize(newnum);
	return oldnum;
    }

    // initialize the new entries
    while (oldnum < newnum) {
	m_delay[oldnum] = oldnum;
	m_coeff[oldnum] = 0.0;
	oldnum++;
    }

    return newnum;
}

//***************************************************************************
unsigned int Filter::count()
{
    ASSERT(m_coeff.count() == m_delay.count());
    return m_coeff.count();
}

//***************************************************************************
double Filter::coeff(unsigned int index)
{
    ASSERT(index < m_coeff.count());
    return m_coeff[index];
}

//***************************************************************************
void Filter::setCoeff(unsigned int index, double newval)
{
    ASSERT(index < m_coeff.count());
    m_coeff[index] = newval;
}

//***************************************************************************
unsigned int Filter::delay(unsigned int index)
{
    ASSERT(index < m_delay.count());
    return m_delay[index];
}

//***************************************************************************
void Filter::setDelay(unsigned int index, unsigned int newval)
{
    ASSERT(index < m_delay.count());
    m_delay[index] = newval;
}

//***************************************************************************
void Filter::save(const QString &filename)
{
    QString name(filename);
    ASSERT(name.length());
    if (!name.length()) return;

    if (name.findRev(".filter") != static_cast<int>(name.length()-7)){
	name.append(".filter");
    }

    QFile file(name);
    file.open(IO_WriteOnly);
    QTextStream out(&file);

    out << ((m_fir) ? "FIR " : "IIR ") << count() << endl;
    for (unsigned int i = 0; i < count(); i++) {
	out << m_delay[i] << ' ' << m_coeff[i] << endl;
    }

    file.close();
}

//***************************************************************************
void Filter::load(const QString &filename)
{
    unsigned int i;
    bool ok;
    QString line;
    unsigned int linenr = 0;

    QFile file(filename);
    file.open(IO_ReadOnly);
    QTextStream in(&file);

    // type of the filter (FIR/IIR)
    while (!in.atEnd()) {
	line = in.readLine().simplifyWhiteSpace();
	linenr++;
	
	if (line.isEmpty() || line.isNull()) continue;
	if ((line[0] == '#') || (line[0] == '/')) continue;
	break;
    };
    m_fir = line.startsWith("FIR ");
    debug("Filter::load(): fir = %d", m_fir);

    // order
    unsigned int order = line.remove(0,4).toUInt(&ok);
    resize(0);
    resize(order);
    debug("Filter::load(): order = %d", order);

    // read delays and coefficients
    i = 0;
    while (!in.atEnd()) {
	line = in.readLine().simplifyWhiteSpace();
	linenr++;
	
	if (line.isEmpty() || line.isNull()) continue;
	if ((line[0] == '#') || (line[0] == '/')) continue;

	int spacepos = line.find(' ');
	ok = true;
	if (ok) m_delay[i] = line.left(spacepos).toUInt(&ok);
	line.remove(0, spacepos);
	if (ok) m_coeff[i] = line.toDouble(&ok);
	
	if (ok) {
	    i++;
	} else {
	    debug("Filter::load(%s): syntax error in line %d",
		filename.data(), linenr);
	}
    }
}

//***************************************************************************
//***************************************************************************
