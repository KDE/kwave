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

#include <QChar>
#include <QLatin1Char>
#include <QRegularExpression>
#include <QString>

#include "libkwave/Parser.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

//***************************************************************************
Kwave::Parser::Parser (const QString &init)
    :m_command(_("")), m_param(), m_current(0), m_commands()
{
    QString line       = init.trimmed();
    unsigned int level = 0;
    bool escaped       = false;

    m_commands = splitCommands(line);
    line = m_commands.first();

    // --- parse the command ---
    qsizetype pos = line.indexOf(QLatin1Char('('));
    if (pos >= 0) {
        // command present
        m_command = line.left(pos).simplified();
        line.remove(0, pos+1);
    } else {
        m_command.clear();
    }

    // --- parse the list of parameters ---
    QString param;
    while (line.length()) {
        QChar c = line[0];
        line.remove(0,1);

        // the next character is escaped
        if (!escaped && (c.toLatin1() == '\\')) {
            escaped = true;
            param += c;
            continue;
        }

        // escaped character
        if (escaped) {
            escaped = false;
            param += c;
            continue;
        }

        switch (c.toLatin1()) {
            case ',':
                if (!level) {
                    m_param.append(unescape(param.trimmed()));
                    param.clear();
                } else param += c;
                break;

            case '(':
                level++;
                param += c;
                break;
            case ')':
                if (!level) {
                    param = unescape(param.trimmed());
                    if (param.length())
                        m_param.append(param);
                    // break, belongs to command, end of line
                    line.clear();
                }
                else
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
    bool escaped = false;

    while (line.length()) {
        QChar c = line[0];
        line.remove(0,1);

        // the next character is escaped
        if (!escaped && (c.toLatin1() == '\\')) {
            escaped = true;
            cmd += c;
            continue;
        }

        // escaped character
        if (escaped) {
            escaped = false;
            cmd += c;
            continue;
        }

        switch (c.toLatin1()) {
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
QStringList Kwave::Parser::remainingParams()
{
    QStringList list;
    while (!isDone())
        list.append(nextParam());
    return list;
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
unsigned int Kwave::Parser::toUInt()
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
sample_index_t Kwave::Parser::toSampleIndex()
{
    const QString &p = nextParam();
    bool ok;
    sample_index_t value = p.toULongLong(&ok);

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
QString Kwave::Parser::escape(const QString &text)
{
    static const QString special = _(":;<=>?[\\]^`");
    QString escaped;

    for (QString::ConstIterator it = text.begin(); it != text.end(); ++it) {
        const QChar c(*it);

        if ((c.toLatin1() < '.') || (c.toLatin1() > 'z') || special.contains(c))
            escaped += _("\\");

        escaped += c;
    }

    return escaped;
}

//***************************************************************************
QString Kwave::Parser::escapeForFileName(const QString &text)
{
    QString result = text;

    // convert all kinds of (multiple) whitespaces, including tabs
    // and newlines into single spaces
    QRegularExpression rx(_("[\\s\\\t\\\r\\\n]+"));
    result.replace(rx, QChar(0x0020));

    // NOTE: this escapes any kind of special chars, but not a slash "/"
    result = escape(result);

    // special handling for slashes "/" -> replace with unicode FRACTION SLASH
    result.replace(QChar(0x002F), QChar(0x2044));

    return result;
}

//***************************************************************************
QString Kwave::Parser::unescape(const QString &text)
{
    QString unescaped;

    bool esc = false;
    for (QString::ConstIterator it = text.begin(); it != text.end(); ++it) {
        const QChar c(*it);

        if (!esc && (c.toLatin1() == '\\')) {
            // this is the leading escape character -> skip it
            esc = true;
            continue;
        }

        esc = false;
        unescaped += c;
    }

    return unescaped;
}

//***************************************************************************
QUrl Kwave::Parser::toUrl(const QString &command)
{
    QUrl url;

    url.setScheme(Kwave::urlScheme());
    Parser parser(command);

    // encode the command as "path"
    url.setPath(QString::fromLatin1(QUrl::toPercentEncoding(parser.command())));

    // encode the parameter list into a comma separated string
    unsigned int count = parser.count();
    QByteArray params;
    for (unsigned int i = 0; i < count; ++i) {
        QString param = parser.nextParam();
        if (params.length()) params += ',';
        params += QUrl::toPercentEncoding(param);
    }
    url.setQuery(QString::fromLatin1(params));

    return url;
}

//***************************************************************************
QString Kwave::Parser::fromUrl(const QUrl &url)
{
    if (url.scheme().toLower() != Kwave::urlScheme()) return QString();

    // get the command name (path)
    QString command = QUrl::fromPercentEncoding(url.path().toLatin1());

    // get the parameter list
    command += _("(");
    QStringList params = url.query().split(_(","));
    if (!params.isEmpty()) {
        bool first = true;
        foreach (const QString &param, params) {
            if (!first) command += _(",");
            command += QUrl::fromPercentEncoding(param.toLatin1());
            first = false;
        }
    }
    command += _(")");

    return command;
}

//***************************************************************************
//***************************************************************************
