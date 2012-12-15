/***************************************************************************
             Parser.cpp  -  parser for Kwave's internal commands
			     -------------------
    begin                : Sat Feb  3 2001
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

#include <QtCore/QLatin1Char>
#include <QtCore/QString>

#include "libkwave/Parser.h"
#include "libkwave/String.h"

//***************************************************************************
Kwave::Parser::Parser (const QString &init)
    :m_command(_("")), m_param(), m_current(0), m_commands()
{
    QString line = init.trimmed();
    unsigned int level = 0;
    int pos;

    m_commands = splitCommands(line);
    line = m_commands.first();

    // --- parse the command ---
    pos = line.indexOf(QLatin1Char('('));
    if (pos >= 0) {
	// command present
	m_command = line.left(pos).simplified();
	line.remove(0, pos+1);
    } else {
	m_command = _("");
    }

    // --- parse the list of parameters ---
    QString param = _("");
    while (line.length()) {
	QChar c = line[0];
	line.remove(0,1);

	switch (c.toAscii()) {
	    case ',':
		if (!level) {
		    m_param.append(param.trimmed());
		    param = _("");
	        } else param += c;
	        break;

	    case '(':
		level++;
		param += c;
		break;
	    case ')':
		if (!level) {
		    m_param.append(param.trimmed());
		    // break, belongs to command, end of line
		    line = _("");
		}
		level--;
		param += c;
		break;
	    default:
		param += c;
	}
    }

    line = line.trimmed();
    if (line.length()) {
	qWarning("Parser: trailing garbage after command: '%s'", DBG(line));
    }
}

//***************************************************************************
Kwave::Parser::~Parser ()
{
}

//***************************************************************************
QStringList Kwave::Parser::splitCommands(QString &line)
{
    // split a line into commands
    unsigned int level = 0;
    QString cmd = _("");
    QStringList commands;

    while (line.length()) {
	QChar c = line[0];
	line.remove(0,1);
	switch (c.toAscii()) {
	    case ';':
		if (!level) {
		    // next command in the list
		    commands.append(cmd.trimmed());
		    cmd = _("");
	        } else cmd += c;
	        break;
	    case '(':
		level++;
		cmd += c;
		break;
	    case ')':
		level--;
		cmd += c;
		break;
	    default:
		cmd += c;
	}
    }

    if (cmd.length()) {
	commands.append(cmd.trimmed());
    }

    return commands;
}

//***************************************************************************
const QString &Kwave::Parser::firstParam()
{
    m_current = 0;
    return nextParam();
}

//***************************************************************************
const QString &Kwave::Parser::nextParam()
{
    static const QString empty = _("");
    if (m_current >= count()) return empty;
    return m_param[m_current++];
}

//***************************************************************************
void Kwave::Parser::skipParam()
{
    nextParam();
}

//***************************************************************************
bool Kwave::Parser::toBool()
{
    const QString &p = nextParam();

    // first test for "true" and "false"
    if (p.toLower() == _("true")) return true;
    if (p.toLower() == _("false")) return false;

    // maybe numeric ?
    bool ok;
    int value = p.toInt(&ok);
    if (ok) return (value != 0);

    qWarning("Parser: invalid bool format: '%s'", DBG(p));
    return false;
}

//***************************************************************************
int Kwave::Parser::toInt ()
{
    const QString &p = nextParam();
    bool ok;
    int value = p.toInt(&ok);

    if (!ok) {
	qWarning("Parser: unable to parse int from '%s'", DBG(p));
	value = 0;
    }

    return value;
}

//***************************************************************************
unsigned int Kwave::Parser::toUInt ()
{
    const QString &p = nextParam();
    bool ok;
    unsigned int value = p.toUInt(&ok);

    if (!ok) {
	qWarning("Parser: unable to parse unsigned int from '%s'", DBG(p));
	value = 0;
    }

    return value;
}

//***************************************************************************
double Kwave::Parser::toDouble()
{
    const QString &p = nextParam();
    bool ok;
    double value = p.toDouble(&ok);

    if (!ok) {
	qWarning("Parser: unable to parse double from '%s'", DBG(p));
	value = 0.0;
    }

    return value;
}

//***************************************************************************
//***************************************************************************
