/***************************************************************************
                Parser.h -  parser for Kwave's internal commands
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

#ifndef _PARSER_H_
#define _PARSER_H_

#include "config.h"
#include <qstring.h>
#include <qstringlist.h>

//*****************************************************************************
class Parser
{
public:
    /**
     * Constructor. Parses the given string into an optional
     * command part and a list of parameters.
     */
    Parser(const QString &init);

    /** Destructor. */
    virtual ~Parser();

    /** Returns the command part of the line */
    inline QString command() {
	return m_command;
    };

    /** Returns the complete list of parameters */
    inline const QStringList &params() {
	return m_param;
    }

    /** Returns true if the parameter list is not empty. */
    inline bool hasParams() {
	return (m_param.count() != 0);
    };

    /** Returns true if the end of the parameter list has been reached. */
    inline bool isDone () {
	return (m_current >= m_param.count());
    };

    /** Returns the number of parameters. */
    inline unsigned int count() {
	return m_param.count();
    };

    /**
     * Returns the first parameter and sets the current
     * position to the next.
     */
    const QString &firstParam();

    /**
     * Returns the next parameter and increases the current
     * position if not already at the end of the parameter
     * list. If the end of the parameter list has been
     * reached, the return value will be a zero-length string.
     */
    const QString &nextParam();

    /**
     * Skips a parameter and advances the current position
     * by one if the end has not already been reached.
     */
    void skipParam();

    /**
     * Gets the next parameter through calling nextParam() and
     * interpretes it as a "bool" value. It will recognize the
     * strings "true" and "false" (not case-sensitive) and
     * numeric values (true means not zero). On errors the
     * return value will be false.
     */
    bool toBool();

    /**
     * Gets the next parameter through calling nextParam() and
     * interpretes it as an "int" value. On errors the
     * return value will be zero.
     */
    int toInt();

    /**
     * Gets the next parameter through calling nextParam() and
     * interpretes it as an "unsigned int" value. On errors the
     * return value will be zero.
     */
    unsigned int toUInt();

    /**
     * Gets the next parameter through calling nextParam() and
     * interpretes it as a "double" value. On errors the
     * return value will be zero.
     */
    double toDouble();

private:
    /** the command part of the line */
    QString m_command;

    /** list of parsed parameters */
    QStringList m_param;

    /** number of the "current" parameter */
    unsigned int m_current;
};

#endif /* _PARSER_H_ */
