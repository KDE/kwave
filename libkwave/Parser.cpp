
#include <qstring.h>

#include "Parser.h"

////**********************************************************
//bool matchCommand (const char *com, const char *comp) {
//    Parser parser(com);
//    //printf ("comparing %s to %s\n",parser.getCommand(),comp);
//    return (strcmp (parser.getCommand(), comp) == 0);
//}

//***************************************************************************
Parser::Parser (const QString &init)
    :m_command(""), m_param(), m_current(0)
{
    QString line = init.stripWhiteSpace();
    int pos;

    // --- parse the command ---
    pos = line.find('(');
    if (pos >= 0) {
	// command present
	m_command = line.left(pos).simplifyWhiteSpace();
	line.remove(0, pos+1);
    } else {
	m_command = "";
    }
//    debug("Parser: command='%s'", m_command.data()); // ###

    // --- parse the list of parameters ---
    unsigned int level = 0;
    QString param = "";
    while (line.length()) {
	QChar c = line[0];
	line.remove(0,1);

	switch (c) {
	    case ',':
		if (level == 0) {
		    m_param.append(param.stripWhiteSpace());
		
//		    debug("Parser: found parameter #%3d: '%s'",
//			count(), m_param[count()-1].data()); // ###
		    param = "";
	        } else param += c;
	        break;

	    case '(':
		level++;
		param += c;
		break;
	    case ')':
		if (!level) {
//		    debug("Parser: reached terminating bracket"); // ###
		    m_param.append(param.stripWhiteSpace());

//		    debug("Parser: last parameter #%3d: '%s'",
//			count(), m_param[count()-1].data()); // ###
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
