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

#include "config.h"

#include <stdlib.h>

#include <new>

#include <QApplication>
#include <QIcon>
#include <QLabel>
#include <QLatin1Char>
#include <QPointer>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <KComboBox>
#include <KConfig>
#include <KConfigGroup>
#include <KHelpClient>
#include <KLocalizedString>
#include <KIconLoader>
#include <KIconTheme>
#include <KSharedConfig>

#include "libkwave/PlayBackDevice.h"
#include "libkwave/PlayBackTypesMap.h"
#include "libkwave/PlaybackController.h"
#include "libkwave/Plugin.h"
#include "libkwave/String.h"

#include "libgui/FileDialog.h"

#include "PlayBackDialog.h"
#include "PlayBackPlugin.h"

//***************************************************************************
Kwave::PlayBackDialog::PlayBackDialog(
    Kwave::Plugin &p,
    Kwave::PlaybackController &playback_controller,
    const Kwave::PlayBackParam &params
)
    :QDialog(p.parentWidget()), PlayBackDlg(),
     m_playback_controller(playback_controller),
     m_device(nullptr),
     m_playback_params(params),
     m_methods_map(),
     m_file_filter(_("")),
     m_devices_list_map(),
     m_enable_setDevice(true)
{
    setupUi(this);
    setModal(true);

    // fill the combo box with playback methods
    unsigned int index=0;
    for (index = 0; index < m_methods_map.count(); index++) {
        cbMethod->addItem(
            m_methods_map.description(index, true),
            static_cast<int>(m_methods_map.data(index)) );
    }
    cbMethod->setEnabled(cbMethod->count() > 1);

    connect(cbMethod, SIGNAL(activated(int)),
            SLOT(methodSelected(int)));
    connect(cbDevice, SIGNAL(editTextChanged(QString)),
            SLOT(setDevice(QString)));
    connect(cbDevice, SIGNAL(textActivated(QString)),
            SLOT(setDevice(QString)));
    connect(cbBitsPerSample, SIGNAL(editTextChanged(QString)),
            SLOT(bitsPerSampleSelected(QString)));
    connect(cbBitsPerSample, SIGNAL(textActivated(QString)),
            SLOT(bitsPerSampleSelected(QString)));
    connect(sbChannels, SIGNAL(valueChanged(int)),
            SLOT(setChannels(int)));
    connect(slBufferSize, SIGNAL(valueChanged(int)),
            SLOT(setBufferSize(int)));
    connect(btSelectDevice, SIGNAL(clicked()),
            SLOT(selectPlaybackDevice()));

    connect(listDevices,
            SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(listEntrySelected(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(listDevices, SIGNAL(itemExpanded(QTreeWidgetItem*)),
            SLOT(listItemExpanded(QTreeWidgetItem*)));
    connect(listDevices, SIGNAL(focusLost()),
            SLOT(updateListSelection()));

    connect(btTest, SIGNAL(clicked()),
            SIGNAL(sigTestPlayback()));
    connect(btHelp->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // remove the header of the tree view
    listDevices->headerItem()->setHidden(true);

    // fix the dialog size
    setFixedHeight(sizeHint().height());

    // update the GUI elements
    // order is: Method -> Device -> "Select..."-button
    // -> Channels -> Bits per Sample
    setMethod(params.method);
    setDevice(params.device);
    setBitsPerSample(params.bits_per_sample);
    setChannels(params.channels);

    // buffer size is independent
    setBufferSize(params.bufbase);

    // set the focus onto the "OK" button
    buttonBox->button(QDialogButtonBox::Ok)->setFocus();
}

//***************************************************************************
Kwave::PlayBackDialog::~PlayBackDialog()
{
}

//***************************************************************************
void Kwave::PlayBackDialog::setMethod(Kwave::playback_method_t method)
{
    Kwave::playback_method_t old_method = m_playback_params.method;

    m_playback_params.method = method;

    // update the selection in the combo box if necessary
    int index = cbMethod->findData(static_cast<int>(method));
    if (cbMethod->currentIndex() != index) {
        cbMethod->setCurrentIndex(index);
        return; // we will get called again, through "methodSelected(...)"
    }

    qDebug("PlayBackDialog::setMethod('%s' [%d])",
           DBG(m_methods_map.name(m_methods_map.findFromData(method))),
           static_cast<int>(method) );

    // set hourglass cursor
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    // change the playback method (class PlayBackDevice)
    delete m_device;
    m_device = nullptr;

    // remember the device selection, just for the GUI
    // for the next time this method gets selected
    // change in method -> save the current device and use
    // the previous one
    QString section = _("plugin playback");
    KConfigGroup cfg = KSharedConfig::openConfig()->group(section);

    // save the current device
    cfg.writeEntry(
        QString(_("last_device_%1")).arg(static_cast<int>(old_method)),
        m_playback_params.device);
    qDebug("SAVE:    '%s' (%d) -> '%s'",
            DBG(m_methods_map.name(m_methods_map.findFromData(old_method))),
            static_cast<int>(old_method),
            DBG(m_playback_params.device.split(_("|")).at(0)));
    cfg.sync();

    // NOTE: the "method" may get modified here if not supported!
    m_playback_controller.checkMethod(method);
    if (method != m_playback_params.method) {
        // method has been modified to some fallback -> start through
        qDebug("    method has changed: %d -> %d",
               static_cast<int>(m_playback_params.method),
               static_cast<int>(method));
        setMethod(method); // -> recursion

        // remove hourglass
        QApplication::restoreOverrideCursor();

        return;
    }

    // if we found no playback method
    if (method == Kwave::PLAYBACK_INVALID) {
        qWarning("found no valid playback method");
    }

    // create a new playback device (will fail if method is unsupported)
    m_device = m_playback_controller.createDevice(method);
    if (!m_device) {
        // oops, something has failed :-(
        setSupportedDevices(QStringList());
        setDevice(QString());

        // remove hourglass
        QApplication::restoreOverrideCursor();

        return;
    }

    // restore the previous settings of the new method
    QString device = cfg.readEntry(
        _("last_device_%1").arg(static_cast<int>(method)));
    qDebug("RESTORE: '%s' (%d) -> '%s'",
            DBG(m_methods_map.name(m_methods_map.findFromData(method))),
            static_cast<int>(method),
           DBG(device.split(_("|")).at(0)));

    m_playback_params.device = device;

    // set list of supported devices
    setSupportedDevices(m_device->supportedDevices());

    // set current device, no matter if supported or not,
    // the dialog will take care of this.
    setDevice(m_playback_params.device);

    // check the filter for the "select..." dialog. If it is
    // empty, the "select" dialog will be disabled
    setFileFilter(m_device->fileFilter());

    // remove hourglass
    QApplication::restoreOverrideCursor();
}

//***************************************************************************
void Kwave::PlayBackDialog::methodSelected(int index)
{
    Kwave::playback_method_t method = static_cast<Kwave::playback_method_t>(
        cbMethod->itemData(index).toInt());

    qDebug("PlayBackDialog::methodSelected(%d) -> %s [%d]", index,
        DBG(m_methods_map.name(m_methods_map.findFromData(method))),
        static_cast<int>(method) );

    if (method <= Kwave::PLAYBACK_NONE) return;
    if (method >= Kwave::PLAYBACK_INVALID) return;

    setMethod(method);
}

//***************************************************************************
void Kwave::PlayBackDialog::setSupportedDevices(QStringList devices)
{
    Q_ASSERT(cbDevice);
    Q_ASSERT(listDevices);
    if (!cbDevice || !listDevices) return;
    QString current_device = m_playback_params.device;

    // disable all that noisy stuff that comes from modifying the
    // device controls...
    m_enable_setDevice = false;

//     qDebug("PlayBackDialog::setSupportedDevices():");
//     foreach (const QString &d, devices)
//      qDebug("    '%s'", DBG(d));

    cbDevice->clearEditText();
    cbDevice->clear();
    listDevices->clear();

    if (devices.contains(_("#EDIT#"))) {
        devices.removeAll(_("#EDIT#"));
        cbDevice->setEditable(true);
    } else {
        cbDevice->setEditable(false);
    }

    if (devices.contains(_("#SELECT#"))) {
        devices.removeAll(_("#SELECT#"));
        btSelectDevice->setEnabled(true);
        btSelectDevice->show();
    } else {
        btSelectDevice->setEnabled(false);
        btSelectDevice->hide();
    }

    if (devices.contains(_("#TREE#"))) {
        // treeview mode
        KIconLoader *icon_loader = KIconLoader::global();

        devices.removeAll((_("#TREE#")));
        listDevices->setEnabled(true);
        cbDevice->setEnabled(false);
        cbDevice->hide();
        m_devices_list_map.clear();

        // build a tree with all nodes in the list
        foreach (const QString &dev_id, devices) {
            QTreeWidgetItem *parent = nullptr;

            QStringList list = dev_id.split(_("||"), Qt::KeepEmptyParts);
            foreach (const QString &t, list) {
                QString token(t);
                QTreeWidgetItem *item = nullptr;

                // split the icon name from the token
                QString icon_name;
                int pos = token.indexOf(QLatin1Char('|'));
                if (pos > 0) {
                    icon_name = token.mid(pos+1);
                    token     = token.left(pos);
                }

                // find the first item with the same text
                // and the same root
                if (parent) {
                    for (int i = 0; i < parent->childCount(); i++) {
                        QTreeWidgetItem *node = parent->child(i);
                        if (node && node->text(0) == token) {
                            item = node;
                            break;
                        }
                    }
                } else {
                    QList<QTreeWidgetItem *> matches =
                        listDevices->findItems(token, Qt::MatchExactly);
                    if (matches.count())
                        item = matches.takeFirst();
                }

                if (item) {
                    // already in the list
                    parent = item;
                } else if (parent) {
                    // new leaf, add to the parent
                    item = new(std::nothrow) QTreeWidgetItem(parent);
                    Q_ASSERT(item);
                    if (item) {
                        item->setText(0, token);
                        m_devices_list_map.insert(item, dev_id);
                    }

                    parent->setExpanded(true);
                    parent->setFlags(parent->flags() &
                        ~(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable));
                    if (m_devices_list_map.contains(parent)) {
                        // make the parent not selectable
                        m_devices_list_map.remove(parent);
                    }
                } else {
                    // new root node
                    item = new(std::nothrow) QTreeWidgetItem(listDevices);
                    Q_ASSERT(item);
                    if (item) {
                        item->setText(0, token);
                        m_devices_list_map.insert(item, dev_id);
                    }
                }

                if (item && icon_name.length() && icon_loader) {
                    QIcon icon = icon_loader->loadIcon(
                        icon_name, KIconLoader::User);
                    item->setIcon(0, icon);
                }

                // use the current item as parent for the next pass
                parent = item;
            }
        }
    } else {
        // combo box mode
        cbDevice->addItems(devices);
        cbDevice->show();
        listDevices->setEnabled(false);

        if (devices.contains(current_device)) {
            // current device is in the list
            cbDevice->setCurrentIndex(cbDevice->findText(current_device));
        } else {
            if (cbDevice->isEditable() && current_device.length()) {
                // user defined device name
                cbDevice->setEditText(current_device);
            } else if (devices.count()) {
                // one or more other possibilities -> take the first one
                cbDevice->setCurrentIndex(0);
            } else {
                // empty list of possibilities
                cbDevice->clearEditText();
                cbDevice->clear();
            }
        }
        cbDevice->setEnabled(devices.count() > 1);
    }

    // enable changes in the device controls again
    m_enable_setDevice = true;
}

//***************************************************************************
void Kwave::PlayBackDialog::listEntrySelected(QTreeWidgetItem *current,
                                              QTreeWidgetItem *previous)
{
    Q_ASSERT(listDevices);
    Q_UNUSED(previous)
    if (!current || !listDevices) return;

    if (m_devices_list_map.contains(current))
        setDevice(m_devices_list_map[current]);
}

//***************************************************************************
void Kwave::PlayBackDialog::listItemExpanded(QTreeWidgetItem *item)
{
    Q_UNUSED(item)
    updateListSelection();
}

//***************************************************************************
void Kwave::PlayBackDialog::updateListSelection()
{
    // set the current device again, otherwise nothing will be selected
    setDevice(m_playback_params.device);
}

//***************************************************************************
void Kwave::PlayBackDialog::setDevice(const QString &device)
{
    Q_ASSERT(cbDevice);
    Q_ASSERT(listDevices);
    if (!cbDevice || !listDevices) return;

    if (!m_enable_setDevice) return;

    qDebug("PlayBackDialog::setDevice(): '%s' -> '%s'",
       DBG(m_playback_params.device.split(_("|")).at(0)),
       DBG(device.split(_("|")).at(0)));

    if (listDevices->isEnabled()) {
        // treeview mode
        QTreeWidgetItem *node = m_devices_list_map.key(device, nullptr);
        if (node) {
            node->setSelected(true);
            listDevices->scrollToItem(node);
            listDevices->setCurrentItem(node);
        }
    } else if (cbDevice->isEditable() && device.length()) {
        // user defined device name
        if (!device.isEmpty() && (cbDevice->currentText() != device)) {
            cbDevice->setCurrentIndex(cbDevice->findText(device));
            cbDevice->setEditText(device);
        }
    } else {
        // just take one from the list
        if (cbDevice->findText(device) >= 0) {
            cbDevice->setCurrentIndex(cbDevice->findText(device));
        } else if (cbDevice->count()) {
            cbDevice->setCurrentIndex(0);
        }
    }

    // select the default device if new one is not supported
    QString dev = device;
    if (m_device) {
        QStringList supported = m_device->supportedDevices();
        supported.removeAll(_("#EDIT#"));
        supported.removeAll(_("#SELECT#"));
        supported.removeAll(_("#TREE#"));
        if (!supported.isEmpty() && !supported.contains(device)) {
            // use the first entry as default
            dev = supported.first();
            qDebug("PlayBackPlugin::setDevice(%s) -> fallback to '%s'",
                DBG(device.split(_("|")).at(0)),
                DBG(dev.split(_("|")).at(0)));
        }
    }

    // take over the device, please note that this one might differ from
    // the device we got as parameter, maybe it is a fallback
    m_playback_params.device = dev;

    QList<unsigned int> supported_bits;
    if (m_device) supported_bits = m_device->supportedBits(dev);
    setSupportedBits(supported_bits);

    unsigned int min = 0;
    unsigned int max = 0;
    if (m_device) m_device->detectChannels(dev, min, max);
    setSupportedChannels(min, max);
}

//***************************************************************************
void Kwave::PlayBackDialog::setBufferSize(int exp)
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
        text = i18n("%1 Bytes", buffer_size);
    } else {
        text = i18n("%1 kB", buffer_size >> 10);
    }
    txtBufferSize->setText(text);
}

//***************************************************************************
void Kwave::PlayBackDialog::setSupportedBits(const QList<unsigned int> &bits)
{
    Q_ASSERT(cbBitsPerSample);
    if (!cbBitsPerSample) return;

    int current_bits = m_playback_params.bits_per_sample;
    cbBitsPerSample->clear();
    QString txt;
    foreach (unsigned int b, bits) {
        txt.setNum(b);
        cbBitsPerSample->addItem(txt);
    }

    // if possibilities are "unknown" -> use last known setting
    if (!bits.count()) {
        txt.setNum(current_bits);
        cbBitsPerSample->addItem(txt);
    }

    if (!bits.contains(current_bits) && bits.count())
        current_bits = bits.last();

    setBitsPerSample(current_bits);
    cbBitsPerSample->setEnabled(bits.count() > 0);
}

//***************************************************************************
void Kwave::PlayBackDialog::bitsPerSampleSelected(const QString &text)
{
    bool ok = false;
    unsigned int bits = text.toUInt(&ok);
    if (!ok) bits = 0;

    setBitsPerSample(bits);
}

//***************************************************************************
void Kwave::PlayBackDialog::setBitsPerSample(unsigned int bits)
{
    Q_ASSERT(cbBitsPerSample);
    if (!cbBitsPerSample) return;

    qDebug("PlayBackDialog::setBitsPerSample(): %u -> %u",
           m_playback_params.bits_per_sample, bits);

    QString txt;
    txt.setNum(bits);
    if (cbBitsPerSample->findText(txt) >= 0) {
        cbBitsPerSample->setCurrentIndex(cbBitsPerSample->findText(txt));
        m_playback_params.bits_per_sample = bits;
    }
}

//***************************************************************************
void Kwave::PlayBackDialog::setSupportedChannels(unsigned int min,
                                                 unsigned int max)
{
    Q_ASSERT(sbChannels);
    if (!sbChannels) return;

    int current_channels = m_playback_params.channels;

    // if possibilities are "unknown" -> use last known setting
    if (!min && !max && current_channels)
        min = max = current_channels;

    sbChannels->setMinimum(min);
    sbChannels->setMaximum(max);
    setChannels(current_channels);
    sbChannels->setEnabled(min != max);
}

//***************************************************************************
void Kwave::PlayBackDialog::setChannels(int channels)
{
    Q_ASSERT(sbChannels);
    if (!sbChannels) return;

    if ((sbChannels->value() != channels) &&
        (sbChannels->minimum() != sbChannels->maximum()) &&
        (sbChannels->maximum() > 0))
    {
        sbChannels->setValue(channels);
        channels = sbChannels->value();
    }

    qDebug("PlayBackDialog::setChannels(): %d -> %d",
           m_playback_params.channels, channels);
    m_playback_params.channels = channels;

    QString txt;
    switch (channels) {
        case 1: txt = i18n("(mono)");   break;
        case 2: txt = i18n("(stereo)"); break;
        case 4: txt = i18n("(quadro)"); break;
        default: txt = _("");
    }
    lblChannels->setText(txt);
}

//***************************************************************************
const Kwave::PlayBackParam &Kwave::PlayBackDialog::params()
{
    return m_playback_params;
}

//***************************************************************************
void Kwave::PlayBackDialog::setFileFilter(const QString &filter)
{
    m_file_filter = filter;
    if (btSelectDevice) btSelectDevice->setEnabled(m_file_filter.length());
}

//***************************************************************************
void Kwave::PlayBackDialog::selectPlaybackDevice()
{
    QString filter = m_file_filter;

    QPointer<Kwave::FileDialog> dlg = new(std::nothrow) Kwave::FileDialog(
        _("kfiledialog:///kwave_playback_device"),
        Kwave::FileDialog::OpenFile, filter, this,
        QUrl(_("file:/dev"))
    );
    if (!dlg) return;
    dlg->setWindowTitle(i18n("Select Playback Device"));
    if (!m_playback_params.device.startsWith(_("#")))
        dlg->selectUrl(QUrl(_("file:") + m_playback_params.device));
    else
        dlg->selectUrl(QUrl(_("file:/dev/*")));
    if (dlg->exec() == QDialog::Accepted) {
        QString new_device = dlg->selectedUrl().fileName();
        // selected new device
        if (cbDevice) cbDevice->setEditText(new_device);
    }
    delete dlg;
}

//***************************************************************************
void Kwave::PlayBackDialog::invokeHelp()
{
    KHelpClient::invokeHelp(_("playback"));
}

//***************************************************************************
//***************************************************************************

#include "moc_PlayBackDialog.cpp"
