/***************************************************************************
       MemoryDialog.cpp  -  setup dialog of Kwave's memory management
                             -------------------
    begin                : Sun Aug 05 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
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
#include <limits.h>
#include <unistd.h>

#ifdef HAVE_MEMINFO
#include <linux/kernel.h> // for struct sysinfo
#include <sys/sysinfo.h>  // for sysinfo()
#endif

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qspinbox.h>

#include "MemoryDialog.h"

//***************************************************************************
MemoryDialog::MemoryDialog(QWidget* parent, bool physical_limited,
    unsigned int physical_limit, bool virtual_enabled, bool virtual_limited,
    unsigned int virtual_limit, const QString &virtual_dir)
    : MemDlg(parent, 0, true)
{
    unsigned int installed_physical = 4096;

#ifdef HAVE_MEMINFO
    struct sysinfo info;
    sysinfo(&info); // get physical memory
    installed_physical = (info.totalram >> 20); // convert to megabytes
#endif

    if (!isOK()) return;

    // initialize all controls
    chkLimitPhysical->setChecked(physical_limited);
    slPhysical->setMaxValue(installed_physical);
    sbPhysical->setMaxValue(installed_physical);
    slPhysical->setValue(physical_limit);
    sbPhysical->setValue(physical_limit);
    chkEnableVirtual->setChecked(virtual_enabled);
    chkLimitVirtual->setChecked(virtual_limited);
    sbVirtual->setValue(virtual_limit);
    edDirectory->setText(virtual_dir);

    // set fixed size
    setFixedWidth(sizeHint().width());
    setFixedHeight(sizeHint().height());
}

//***************************************************************************
MemoryDialog::~MemoryDialog()
{
}

//***************************************************************************
bool MemoryDialog::isOK()
{
    ASSERT(chkEnableVirtual);
    ASSERT(chkLimitPhysical);
    ASSERT(chkLimitVirtual);
    ASSERT(edDirectory);
    ASSERT(sbPhysical);
    ASSERT(sbVirtual);
    ASSERT(slPhysical);
    ASSERT(slVirtual);

    return chkEnableVirtual && chkLimitPhysical && chkLimitVirtual &&
           edDirectory && sbPhysical && sbVirtual && slPhysical &&
           slVirtual;
}

//***************************************************************************
void MemoryDialog::params(QStringList &par)
{
    par.clear();
    par << QString::number(chkLimitPhysical->isChecked() ? 1 : 0);
    par << QString::number(sbPhysical->value());
    par << QString::number(chkEnableVirtual->isChecked() ? 1 : 0);
    par << QString::number(chkLimitVirtual->isChecked() ? 1 : 0);
    par << QString::number(sbVirtual->value());
    par << edDirectory->text();
}

//***************************************************************************
//***************************************************************************
