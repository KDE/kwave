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

#include <qstring.h>
#include "libkwave/Parser.h"

//***************************************************************************
Parser::Parser (const QString &init)
    :m_command(""), m_param(), m_current(0), m_commands()
{
    QString line = init.stripWhiteSpace();
    unsigned int level = 0;
    int pos;

    m_commands = splitCommands(line);
    line = m_commands.first();

    // --- parse the command ---
    pos = line.find('(');
    if (pos >= 0) {
	// command present
	m_command = line.left(pos).simplifyWhiteSpace();
	line.remove(0, pos+1);
    } else {
	m_command = "";
    }

    // --- parse the list of parameters ---
    QString param = "";
    while (line.length()) {
	QChar c = line[0];
	line.remove(0,1);

	switch (c) {
	    case ',':
		if (!level) {
		    m_param.append(param.stripWhiteSpace());
		    param = "";
	        } else param += c;
	        break;

	    case '(':
		level++;
		param += c;
		break;
	    case ')':
		if (!level) {
		    m_param.append(param.stripWhiteSpace());
		    line = ""; // break, belongs to command, end of line
		}
		level--;
		param += c;
		break;
	    default:
		param += c;
	}
    }

    line = line.stripWhiteSpace();
    if (line.length()) {
	warning("Parser: trailing garbage after command: '%s'", line.data());
    }
}

//***************************************************************************
Parser::~Parser ()
{
}

//***************************************************************************
QStringList Parser::splitCommands(QString &line)
{
    // split a line into commands
    unsigned int level = 0;
    int pos;
    QString cmd = "";
    QStringList commands;

    while (line.length()) {
	QChar c = line[0];
	line.remove(0,1);
	switch (c) {
	    case ';':
		if (!level) {
		    // next command in the list
		    commands.append(cmd.stripWhiteSpace());
		    cmd = "";
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
	commands.append(cmd.stripWhiteSpace());
    }

    return commands;
}

//***************************************************************************
const QString &Parser::firstParam()
{
    m_current = 0;
    return nextParam();
}

//***************************************************************************
const QString &Parser::nextParam()
{
    static const QString empty("");
    if (m_current >= count()) return empty;
    return m_param[m_current++];
}

//***************************************************************************
void Parser::skipParam()
{
    nextParam();
}

//***************************************************************************
bool Parser::toBool()
{
    const QString &p = nextParam();

    // first test for "true" and "false"
    if (p.lower() == "true") return true;
    if (p.lower() == "false") return false;

    // maybe numeric ?
    bool ok;
    int value = p.toInt(&ok);
    if (ok) return (value != 0);

    warning("Parser: invalid bool format: '%s'", p.data());
    return false;
}

//***************************************************************************
int Parser::toInt ()
{
    const QString &p = nextParam();
    bool ok;
    int value = p.toInt(&ok);

    if (!ok) {
	warning("Parser: unable to parse int from '%s'", p.data());
	value = 0;
    }

    return value;
}

//***************************************************************************
unsigned int Parser::toUInt ()
{
    const QString &p = nextParam();
    bool ok;
    unsigned int value = p.toUInt(&ok);

    if (!ok) {
	warning("Parser: unable to parse unsigned int from '%s'", p.data());
	value = 0;
    }

    return value;
}

//***************************************************************************
double Parser::toDouble()
{
    const QString &p = nextParam();
    bool ok;
    double value = p.toDouble(&ok);

    if (!ok) {
	warning("Parser: unable to parse double from '%s'", p.data());
	value = 0.0;
    }

    return value;
}

//***************************************************************************
//***************************************************************************
