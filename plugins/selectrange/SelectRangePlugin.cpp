/***************************************************************************
  SelectRangePlugin.cpp  -  Plugin for selecting a range of samples
                             -------------------
    begin                : Sat Jun 15 2002
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

#include "config.h"
#include <errno.h>
#include <math.h>

#include <qstring.h>
#include <klocale.h>

#include "kwave/PluginManager.h"
#include "kwave/SignalManager.h"
#include "kwave/UndoTransactionGuard.h"
#include "libkwave/KwavePlugin.h"
#include "SelectRangePlugin.h"
#include "SelectRangeDialog.h"

KWAVE_PLUGIN(SelectRangePlugin,"selectrange","Thomas Eschenbacher");

//***************************************************************************
SelectRangePlugin::SelectRangePlugin(PluginContext &c)
    :KwavePlugin(c), m_mode(SelectTimeWidget::bySamples), m_range(0)
{
}

//***************************************************************************
SelectRangePlugin::~SelectRangePlugin()
{
}

//***************************************************************************
QStringList *SelectRangePlugin::setup(QStringList &previous_params)
{
    // try to interprete the previous parameters
    interpreteParameters(previous_params);

    // create the setup dialog
    double rate = signalRate();
    unsigned int offset = 0;
    selection(&offset, 0, false);
    unsigned int length = signalLength();

    SelectRangeDialog *dialog = new SelectRangeDialog(parentWidget(),
        m_mode, m_range, rate, offset, length);
    ASSERT(dialog);
    if (!dialog) return 0;

    QStringList *list = new QStringList();
    ASSERT(list);
    if (list && dialog->exec()) {
	// user has pressed "OK"
	*list << QString::number(dialog->mode());
	*list << QString::number(dialog->range());
	
	emitCommand("plugin:execute(selectrange,"+
	    QString::number(dialog->mode())+","+
	    QString::number(dialog->range())+
	    ")"
	);
    } else {
	// user pressed "Cancel"
	if (list) delete list;
	list = 0;
    }

    if (dialog) delete dialog;
    return list;
}

//***************************************************************************
int SelectRangePlugin::start(QStringList &params)
{
    // interprete the parameters
    int result = interpreteParameters(params);
    if (result) return result;

    // get current offset of the signal
    unsigned int offset = 0;
    selection(&offset, 0, false);

    // transform into offset and length [samples]
    unsigned int length = 0;
    switch (m_mode) {
	case SelectTimeWidget::byTime: {
	    // convert from ms to samples
	    double rate = signalRate();
	    length = (unsigned int)rint(m_range / 1E3 * rate);
	    break;
	}
	case SelectTimeWidget::bySamples: {
	    // simple case -> already in samples
	    length = (unsigned int)rint(m_range);
	    break;
	}
	case SelectTimeWidget::byPercents: {
	    // by percentage of whole signal
	    unsigned sig_length = signalLength();
	    length = (unsigned int)rint((double)sig_length*(m_range/100.0));
	    break;
	}
    }

    // limit selection to end of signal
    if ((offset + length) >= signalLength())
	length = signalLength() - offset;

    // change the selection through the signal manager
    {
	UndoTransactionGuard undo_guard(*this, i18n("select range"));
	selectRange(offset, length);
    }

    return result;
}

//***************************************************************************
int SelectRangePlugin::interpreteParameters(QStringList &params)
{
    bool ok;
    QString param;

    // evaluate the parameter list
    ASSERT(params.count() == 2);
    if (params.count() != 2) {
	warning("SelectRangePlugin::interpreteParams(): params.count()=%d",
	      params.count());
	return -EINVAL;
    }

    // selection mode
    param = params[0];
    int mode = param.toInt(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;
    ASSERT((mode == (int)SelectTimeWidget::byTime) ||
           (mode == (int)SelectTimeWidget::bySamples) ||
           (mode == (int)SelectTimeWidget::byPercents));
    if ((mode != (int)SelectTimeWidget::byTime) &&
        (mode != (int)SelectTimeWidget::bySamples) &&
        (mode != (int)SelectTimeWidget::byPercents))
    {
	return -EINVAL;
    }
    m_mode = (SelectTimeWidget::Mode)mode;

    // range in ms, samples or percent
    param = params[1];
    m_range = (unsigned int)param.toDouble(&ok);
    ASSERT(ok);
    if (!ok) return -EINVAL;

    return 0;
}

//***************************************************************************
//***************************************************************************
