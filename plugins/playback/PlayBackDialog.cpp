/***************************************************************************
     PlayBackDialog.cpp  -  dialog for configuring the playback
			     -------------------
    begin                : Sun May 13 2001
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

#include <stdio.h>
#include <stdlib.h>

#include <qlabel.h>
#include <qlistview.h>
#include <qslider.h>
#include <qstringlist.h>

#include <kapplication.h> // for invokeHelp
#include <kfiledialog.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <knuminput.h>
#include <kpushbutton.h>
#include <kcombobox.h>

#include "libgui/KwaveFileDialog.h"
#include "libkwave/KwavePlugin.h"
#include "PlayBackDialog.h"
#include "PlayBackPlugin.h"

#ifndef max
#define max(x,y) (( x > y ) ? x : y )
#endif

//***************************************************************************
PlayBackDialog::PlayBackDialog(KwavePlugin &p, const PlayBackParam &params)
    :PlayBackDlg(p.parentWidget(), i18n("playback"), true),
    m_playback_params(params), m_file_filter(""), m_devices_list_map(),
    m_enable_setDevice(true)
{
    // button for "test settings"
    // (not implemented yet)

    // fill the combo box with playback methods
    unsigned int index=0;
    for (index=0; index < m_methods_map.count(); index++) {
	cbMethod->insertItem(m_methods_map.description(index, true));
    }
    cbMethod->setEnabled(cbMethod->count() > 1);

    connect(cbMethod, SIGNAL(activated(int)),
            SLOT(methodSelected(int)));
    connect(cbDevice, SIGNAL(textChanged(const QString &)),
            SLOT(setDevice(const QString &)));
    connect(cbDevice, SIGNAL(activated(const QString &)),
            SLOT(setDevice(const QString &)));
    connect(cbBitsPerSample, SIGNAL(textChanged(const QString &)),
            SLOT(bitsPerSampleSelected(const QString &)));
    connect(cbBitsPerSample, SIGNAL(activated(const QString &)),
            SLOT(bitsPerSampleSelected(const QString &)));
    connect(sbChannels, SIGNAL(valueChanged(int)),
            SLOT(setChannels(int)));
    connect(slBufferSize, SIGNAL(valueChanged(int)),
            SLOT(setBufferSize(int)));
    connect(btSelectDevice, SIGNAL(clicked()),
            SLOT(selectPlaybackDevice()));
    connect(listDevices, SIGNAL(selectionChanged(QListViewItem *)),
            SLOT(listEntrySelected(QListViewItem *)));
    connect(btTest, SIGNAL(clicked()),
            SLOT(forwardSigTestPlayback()));
    connect(btHelp, SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // fix the dialog size
    setFixedHeight(sizeHint().height());

    // update the GUI elements
    // order is: Method -> Device -> "Select..."-button
    // -> Channels -> Bits per Sample
    setMethod(params.method);
    setDevice(params.device);
    setBitsPerSample(params.bits_per_sample);
    setChannels(params.channels);

    // buffer size is independend
    setBufferSize(params.bufbase);
}

//***************************************************************************
PlayBackDialog::~PlayBackDialog()
{
}

//***************************************************************************
void PlayBackDialog::setMethod(playback_method_t method)
{
    m_playback_params.method = method;
    cbMethod->setCurrentItem(m_methods_map.findFromData(
        m_playback_params.method));
}

//***************************************************************************
void PlayBackDialog::methodSelected(int index)
{
    playback_method_t method = m_methods_map.data(index);
    Q_ASSERT(method > PLAYBACK_NONE);
    Q_ASSERT(method < PLAYBACK_INVALID);
    if (method <= PLAYBACK_NONE) return;
    if (method >= PLAYBACK_INVALID) return;

    if (method != m_playback_params.method) {
	setMethod(method);
	emit sigMethodChanged(method);
    }
}

//***************************************************************************
void PlayBackDialog::setSupportedDevices(QStringList devices)
{
    Q_ASSERT(cbDevice);
    Q_ASSERT(listDevices);
    if (!cbDevice || !listDevices) return;
    QString current_device = m_playback_params.device;

    // disable all that noisy stuff that comes from modifying the
    // device controls...
    m_enable_setDevice = false;

    KIconLoader icon_loader;

    cbDevice->clearEdit();
    cbDevice->clear();
    cbDevice->insertStringList(devices);
    listDevices->clear();

    if (devices.contains("#EDIT#")) {
	devices.remove("#EDIT#");
	cbDevice->setEditable(true);
    } else {
	cbDevice->setEditable(false);
    }

    if (devices.contains("#SELECT#")) {
	devices.remove("#SELECT#");
	btSelectDevice->setEnabled(true);
	btSelectDevice->show();
    } else {
	btSelectDevice->setEnabled(false);
	btSelectDevice->hide();
    }

    if (devices.contains("#TREE#")) {
	// treeview mode
	devices.remove("#TREE#");
	listDevices->setEnabled(true);
	cbDevice->setEnabled(false);
	cbDevice->hide();
	m_devices_list_map.clear();

	// build a tree with all nodes in the list
	for (QStringList::Iterator it=devices.begin();
	     it != devices.end(); ++it)
	{
	    QString dev_id = *it;
	    QListViewItem *parent = 0;

	    QStringList list;
	    list = QStringList::split("||", dev_id, true);
	    for (QStringList::Iterator it=list.begin();
	         it != list.end(); ++it)
	    {
		QString token = *it;
		QListViewItem *item = 0;

		// split the icon name from the token
		QString icon_name;
		int pos = token.find('|');
		if (pos > 0) {
		    icon_name = token.mid(pos+1);
		    token     = token.left(pos);
		}

		// find the first item with the same text
		// and the same root
		for (QListViewItem *node = (parent) ? parent->firstChild() :
		     listDevices->firstChild(); (node);
		     node = node->nextSibling())
		{
		    if (node->text(0) == token) {
			item = node;
			break;
		    }
		}

		if (item) {
		    // already in the list
		    parent = item;
		} else if (parent) {
		    // new leaf, add to the parent
		    item = new QListViewItem(parent, token);
		    Q_ASSERT(item);
		    if (item) m_devices_list_map.insert(item, dev_id);

		    parent->setOpen(true);
		    parent->setSelectable(false);
		    parent->setExpandable(true);
		    if (m_devices_list_map.contains(parent)) {
			// make the parent not selectable
			m_devices_list_map.remove(parent);
		    }
		} else {
		    // new root node
		    item = new QListViewItem(listDevices, token);
		    Q_ASSERT(item);
		    if (item) m_devices_list_map.insert(item, dev_id);
		}

		if (icon_name.length()) {
		    QPixmap icon = icon_loader.loadIcon(icon_name,
		                                        KIcon::User);
		    item->setPixmap(0, icon);
		}

		// use the current item as parent for the next pass
		parent = item;
	    }
	}
    } else {
	// combo box mode
	cbDevice->show();
	listDevices->setEnabled(false);

	if (devices.contains(current_device)) {
	    // current device is in the list
	    cbDevice->setCurrentText(current_device);
	} else {
	    if (cbDevice->editable()) {
		// user defined device name
		cbDevice->setEditText(current_device);
	    } else if (devices.count()) {
		// one or more other possibilities -> take the first one
		cbDevice->setCurrentItem(0);
	    } else {
		// empty list of possibilities
		cbDevice->clearEdit();
		cbDevice->clear();
	    }
	}
	cbDevice->setEnabled(devices.count() > 1);
    }

    // enable changes in the device controls again
    m_enable_setDevice = true;
}

//***************************************************************************
void PlayBackDialog::listEntrySelected(QListViewItem *item)
{
    Q_ASSERT(item);
    Q_ASSERT(listDevices);
    if (!item || !listDevices) return;

    if (m_devices_list_map.contains(item))
	setDevice(m_devices_list_map[item]);
}

//***************************************************************************
void PlayBackDialog::setDevice(const QString &device)
{
    Q_ASSERT(cbDevice);
    Q_ASSERT(listDevices);
    if (!cbDevice || !listDevices) return;

    if (!m_enable_setDevice) return;
//     qDebug("PlayBackDialog::setDevice(%s)", device.data());

    if (listDevices->isEnabled()) {
	// treeview mode
	QListViewItemIterator it(listDevices);
	while (it.current()) {
	    QListViewItem *node = it.current();
	    if (m_devices_list_map.contains(node)) {
		if (m_devices_list_map[node] == device) {
		    listDevices->setSelected(node, true);
		}
	    }
	    ++it;
	}
    } else if (cbDevice->editable() && device.length()) {
	// user defined device name
	if (cbDevice->currentText() != device) {
	    cbDevice->setCurrentText(device);
	    cbDevice->setEditText(device);
	}
    } else {
	// just take one from the list
	if (cbDevice->listBox()->findItem(device)) {
	    cbDevice->setCurrentText(device);
	} else if (cbDevice->count()) {
	    cbDevice->setCurrentItem(0);
	}
    }

    if (m_playback_params.device != device) {
	m_playback_params.device = device;
	emit sigDeviceChanged(device);
    }
}

//***************************************************************************
void PlayBackDialog::setBufferSize(int exp)
{
    Q_ASSERT(slBufferSize);
    Q_ASSERT(txtBufferSize);
    if (!slBufferSize || !txtBufferSize) return;

    if (exp <  8) exp =  8;
    if (exp > 18) exp = 18;

    // update the slider if necessary
    if (slBufferSize->value() != exp) slBufferSize->setValue(exp);

    // take the value into our struct
    m_playback_params.bufbase = exp;

    // update the text
    unsigned int buffer_size = (1 << exp);
    QString text;
    if (buffer_size < 1024) {
	text = i18n("%1 Bytes");
    } else {
	text = i18n("%1 kB");
	buffer_size >>= 10;
    }
    txtBufferSize->setText(text.arg(buffer_size));
}

//***************************************************************************
void PlayBackDialog::setSupportedBits(const QValueList<unsigned int> &bits)
{
    Q_ASSERT(cbBitsPerSample);
    if (!cbBitsPerSample) return;

    int current_bits = m_playback_params.bits_per_sample;
    cbBitsPerSample->clear();
    QValueListConstIterator<unsigned int> it;
    QString txt;
    for (it=bits.begin(); it != bits.end(); ++it) {
	txt.setNum(*it);
	cbBitsPerSample->insertItem(txt);
    }

    // if possibilities are "unknown" -> use last known setting
    if (!bits.count()) {
	txt.setNum(current_bits);
	cbBitsPerSample->insertItem(txt);
    }

    if (!bits.contains(current_bits) && bits.count())
	current_bits = bits.last();

    setBitsPerSample(current_bits);
    cbBitsPerSample->setEnabled(bits.count() > 0);
}

//***************************************************************************
void PlayBackDialog::bitsPerSampleSelected(const QString &text)
{
    bool ok = false;
    unsigned int bits = text.toUInt(&ok);
    if (!ok) bits = 0;

    setBitsPerSample(bits);
}

//***************************************************************************
void PlayBackDialog::setBitsPerSample(unsigned int bits)
{
    Q_ASSERT(cbBitsPerSample);
    if (!cbBitsPerSample) return;

//     qDebug("PlayBackDialog::setBitsPerSample(%u)", bits);

    QString txt;
    txt.setNum(bits);
    if (cbBitsPerSample->listBox()->findItem(txt)) {
	cbBitsPerSample->setCurrentText(txt);
	m_playback_params.bits_per_sample = bits;
    }
}

//***************************************************************************
void PlayBackDialog::setSupportedChannels(unsigned int min, unsigned int max)
{
    Q_ASSERT(sbChannels);
    if (!sbChannels) return;

    int current_channels = m_playback_params.channels;

    // if possibilities are "unknown" -> use last known setting
    if (!min && !max && current_channels)
	min = max = current_channels;

    sbChannels->setMinValue(min);
    sbChannels->setMaxValue(max);
    setChannels(current_channels);
    sbChannels->setEnabled(min != max);
}

//***************************************************************************
void PlayBackDialog::setChannels(int channels)
{
    Q_ASSERT(sbChannels);
    if (!sbChannels) return;

//     qDebug("PlayBackDialog::setChannels(%d)", channels);
    m_playback_params.channels = channels;

    if ((sbChannels->value() != channels) &&
        (sbChannels->minValue() != sbChannels->maxValue()) &&
	(sbChannels->maxValue() > 0))
    {
	sbChannels->setValue(channels);
	channels = sbChannels->value();
    }
//     qDebug("PlayBackDialog::setChannels --> %d", channels);

    QString txt;
    switch (channels) {
	case 1: txt = i18n("(mono)");   break;
	case 2: txt = i18n("(stereo)"); break;
	case 4: txt = i18n("(quadro)"); break;
	default: txt = "";
    }
    lblChannels->setText(txt);
}

//***************************************************************************
const PlayBackParam &PlayBackDialog::params()
{
    return m_playback_params;
}

//***************************************************************************
void PlayBackDialog::setFileFilter(const QString &filter)
{
    m_file_filter = filter;
    if (btSelectDevice) btSelectDevice->setEnabled(m_file_filter.length());
}

//***************************************************************************
void PlayBackDialog::selectPlaybackDevice()
{
    QString filter = m_file_filter;

    KwaveFileDialog dlg(":<kwave_playback_device>", filter, this,
	"Kwave select playback device", true, "file:/dev");
    dlg.setKeepLocation(true);
    dlg.setOperationMode(KFileDialog::Opening);
    dlg.setCaption(i18n("Select Playback Device"));
    if (m_playback_params.device[0] != '[')
        dlg.setURL("file:"+m_playback_params.device);
    else
        dlg.setURL("file:/dev/*");
    if (dlg.exec() != QDialog::Accepted) return;

    QString new_device = dlg.selectedFile();

    // selected new device
    if (cbDevice) cbDevice->setEditText(new_device);
}

//***************************************************************************
void PlayBackDialog::forwardSigTestPlayback()
{
    emit sigTestPlayback();
}

//***************************************************************************
void PlayBackDialog::invokeHelp()
{
    kapp->invokeHelp("playback");
}

//***************************************************************************
//***************************************************************************
