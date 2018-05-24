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

#include <unistd.h>

#include <limits>
#include <new>

#include <QCheckBox>
#include <QFileDialog>
#include <QLabel>
#include <QObject>
#include <QPointer>
#include <QPushButton>
#include <QSlider>
#include <QLineEdit>
#include <QSpinBox>
#include <QString>
#include <QStringList>

#include <KHelpClient>
#include <KLocalizedString>

#include "libkwave/MemoryManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/FileDialog.h"

#include "MemoryDialog.h"

//***************************************************************************
Kwave::MemoryDialog::MemoryDialog(QWidget* parent, bool physical_limited,
    unsigned int physical_limit, bool virtual_enabled, bool virtual_limited,
    unsigned int virtual_limit, const QString &virtual_dir,
    unsigned int undo_limit)
    :QDialog(parent), Ui::MemDlg()
{
    setupUi(this);
    setModal(true);

    Kwave::MemoryManager &mem = Kwave::MemoryManager::instance();
    int total_physical = Kwave::toInt(qMin(
	mem.totalPhysical(),
	static_cast<quint64>(std::numeric_limits<int>::max())
    ));

    if (!isOK()) return;

    physical_limit = qMin(physical_limit, Kwave::toUint(total_physical));

    // connect the controls
    connect(chkEnableVirtual, SIGNAL(toggled(bool)),
            this, SLOT(virtualMemoryEnabled(bool)));
    connect(btSearch, SIGNAL(clicked()),
            this, SLOT(searchSwapDir()));
    connect(buttonBox->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // initialize all controls
    chkLimitPhysical->setChecked(physical_limited);
    slPhysical->setMaximum(total_physical);
    sbPhysical->setMaximum(total_physical);
    slPhysical->setValue(physical_limit);
    sbPhysical->setValue(physical_limit);
    chkEnableVirtual->setChecked(virtual_enabled);
    chkLimitVirtual->setChecked(virtual_limited);
    sbVirtual->setValue(virtual_limit);
    edDirectory->setText(virtual_dir);
    slUndo->setMaximum(total_physical / 2);
    sbUndo->setMaximum(slUndo->maximum());
    sbUndo->setValue(undo_limit);

    virtualMemoryEnabled(virtual_enabled);

    // set fixed size
    setFixedWidth(sizeHint().width());
    setFixedHeight(sizeHint().height());

    // set the focus onto the "OK" button
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

//***************************************************************************
Kwave::MemoryDialog::~MemoryDialog()
{
}

//***************************************************************************
bool Kwave::MemoryDialog::isOK()
{
    Q_ASSERT(chkEnableVirtual);
    Q_ASSERT(chkLimitPhysical);
    Q_ASSERT(chkLimitVirtual);
    Q_ASSERT(edDirectory);
    Q_ASSERT(sbPhysical);
    Q_ASSERT(sbVirtual);
    Q_ASSERT(slPhysical);
    Q_ASSERT(slVirtual);

    return chkEnableVirtual && chkLimitPhysical && chkLimitVirtual &&
           edDirectory && sbPhysical && sbVirtual && slPhysical &&
           slVirtual;
}

//***************************************************************************
void Kwave::MemoryDialog::params(QStringList &par)
{
    par.clear();
    par << QString::number(chkLimitPhysical->isChecked() ? 1 : 0);
    par << QString::number(sbPhysical->value());
    par << QString::number(chkEnableVirtual->isChecked() ? 1 : 0);
    par << QString::number(chkLimitVirtual->isChecked() ? 1 : 0);
    par << QString::number(sbVirtual->value());
    par << edDirectory->text();
    par << QString::number(sbUndo->value());
}

//***************************************************************************
void Kwave::MemoryDialog::virtualMemoryEnabled(bool enable)
{
    bool limit = enable && (chkLimitVirtual->isChecked());

    chkLimitVirtual->setEnabled(enable);
    slVirtual->setEnabled(limit);
    sbVirtual->setEnabled(limit);

    txtDirectory->setEnabled(enable);
    edDirectory->setEnabled(enable);
    btSearch->setEnabled(enable);
}

//***************************************************************************
void Kwave::MemoryDialog::searchSwapDir()
{
    QPointer<Kwave::FileDialog> dlg = new(std::nothrow) Kwave::FileDialog(
	edDirectory->text(), Kwave::FileDialog::SelectDir, QString(), this);
    if (!dlg) return;
    dlg->setWindowTitle(i18n("Select Swap File Directory"));
    if ((dlg->exec() == QDialog::Accepted) && dlg) {
	QString dir = dlg->selectedUrl().toLocalFile();
	if (dir.length()) edDirectory->setText(dir);
    }
    delete dlg;
}

//***************************************************************************
void Kwave::MemoryDialog::invokeHelp()
{
    KHelpClient::invokeHelp(_("memory-setup"));
}

//***************************************************************************
//***************************************************************************
