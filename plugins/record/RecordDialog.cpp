/*************************************************************************
       RecordDialog.cpp  -  dialog window for controlling audio recording
                             -------------------
    begin                : Wed Aug 20 2003
    copyright            : (C) 2003 by Thomas Eschenbacher
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

#include <new>

#include <QCheckBox>
#include <QDateTimeEdit>
#include <QGroupBox>
#include <QIcon>
#include <QLabel>
#include <QLatin1Char>
#include <QPixmap>
#include <QPointer>
#include <QProgressBar>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QStatusBar>
#include <QTabWidget>
#include <QTreeView>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVector>

#include <KComboBox>
#include <KHelpClient>
#include <KLocalizedString>
#include <KIconLoader>
#include <KIconTheme>

#include "libkwave/Compression.h"
#include "libkwave/SampleFormat.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"

#include "libgui/FileDialog.h"
#include "libgui/HMSTimeWidget.h"

#include "LevelMeter.h"
#include "RecordDevice.h"
#include "RecordDialog.h"
#include "RecordParams.h"
#include "RecordState.h"

// status bar icons
#include "ledgreen.xpm"
#include "ledlightgreen.xpm"
#include "ledred.xpm"
#include "ledyellow.xpm"
#include "ok.xpm"
#include "stop_hand.xpm"
#include "walk_r1.xpm"
#include "walk_r2.xpm"
#include "walk_r3.xpm"
#include "walk_r4.xpm"
#include "walk_r5.xpm"
#include "walk_r6.xpm"
#include "walk_r7.xpm"
#include "walk_r8.xpm"

/* some macros, for laziness ;-) */
#define SETUP(enabled,property,check,value) \
        check->setChecked(m_params.enabled); \
        value->setValue(m_params.property); \

#define STD_SETUP(enabled,property,control) \
        SETUP(enabled,property,chk##control,sb##control); \
        sb##control->setEnabled(chk##control->isEnabled() && \
                                chk##control->isChecked());

#define STD_SETUP_SLIDER(enabled,property,control) \
        STD_SETUP(enabled,property,control); \
        sl##control->setEnabled(sb##control->isEnabled());

//***************************************************************************
Kwave::RecordDialog::RecordDialog(QWidget *parent, QStringList &params,
                                  Kwave::RecordController *controller,
                                  Kwave::RecordDialog::Mode mode)
    :QDialog(parent), Ui::RecordDlg(), m_methods_map(),
     m_file_filter(), m_devices_list_map(),
     m_state(Kwave::REC_EMPTY), m_params(),
     m_supported_resolutions(), m_buffer_progress_count(0),
     m_buffer_progress_total(0), m_buffer_progress_timer(this),
     m_record_enabled(true), m_samples_recorded(0),
     m_enable_setDevice(true), m_state_icon_widget(Q_NULLPTR)
{
    m_status_bar.m_state           = Q_NULLPTR;
    m_status_bar.m_time            = Q_NULLPTR;
    m_status_bar.m_sample_rate     = Q_NULLPTR;
    m_status_bar.m_bits_per_sample = Q_NULLPTR;
    m_status_bar.m_tracks          = Q_NULLPTR;

    setupUi(this);

    /* get initial parameters */
    m_params.fromList(params);

    /* set the icons of the record control buttons */
    btNew->setIcon(   QIcon::fromTheme(_("document-new")));
    btStop->setIcon(  QIcon::fromTheme(_("kwave_player_stop")));
    btPause->setIcon( QIcon::fromTheme(_("kwave_player_pause")));
    btRecord->setIcon(QIcon::fromTheme(_("kwave_player_record")));

    // fill the combo box with playback methods
    unsigned int index=0;
    for (index = 0; index < m_methods_map.count(); ++index) {
        cbMethod->addItem(m_methods_map.description(index, true));
    }
    cbMethod->setEnabled(cbMethod->count() > 1);

    /* --- set up all controls with their default/startup values --- */

    // pre-record
    STD_SETUP_SLIDER(pre_record_enabled, pre_record_time, RecordPre)
    connect(chkRecordPre, SIGNAL(toggled(bool)),
            this,         SLOT(preRecordingChecked(bool)));
    connect(sbRecordPre,  SIGNAL(valueChanged(int)),
            this,         SLOT(preRecordingTimeChanged(int)));

    // record time (duration)
    STD_SETUP(record_time_limited, record_time, RecordTime)

    // start time (date & time)
    chkRecordStartTime->setChecked(m_params.start_time_enabled);
    startTime->setDateTime(m_params.start_time);

    // record trigger
    STD_SETUP_SLIDER(record_trigger_enabled, record_trigger, RecordTrigger)

    // amplification
    STD_SETUP_SLIDER(amplification_enabled, amplification, LevelAmplify)

    // AGC
    STD_SETUP_SLIDER(agc_enabled, agc_decay, LevelAGC)

    // fade in
    STD_SETUP(fade_in_enabled, fade_in_time, LevelFadeIn)

    // fade out
    STD_SETUP(fade_out_enabled, fade_out_time, LevelFadeOut)

    // sample rate, bits per sample, track
    // -> will be initialized later, by the plugin

    // number of buffers
    slSourceBufferCount->setValue(m_params.buffer_count);

    // power of buffer size
    slSourceBufferSize->setValue(m_params.buffer_size);
    sourceBufferSizeChanged(m_params.buffer_size);

    // after this point all controls have their initial values

    /* --- connect some missing low level GUI functionality --- */

    connect(cbMethod, SIGNAL(activated(int)),
            this, SLOT(methodSelected(int)));

    // record buffer size and count
    slSourceBufferCount->setValue(m_params.buffer_count);
    slSourceBufferSize->setValue(m_params.buffer_size);
    connect(slSourceBufferSize, SIGNAL(valueChanged(int)),
            this, SLOT(sourceBufferSizeChanged(int)));
    connect(slSourceBufferCount, SIGNAL(valueChanged(int)),
            this, SLOT(sourceBufferCountChanged(int)));

    // device treeview
    connect(listDevices,
            SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            SLOT(listEntrySelected(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(listDevices, SIGNAL(itemExpanded(QTreeWidgetItem*)),
            SLOT(listItemExpanded(QTreeWidgetItem*)));
    connect(listDevices, SIGNAL(focusLost()),
            SLOT(updateListSelection()));

    // "select device..." button
    connect(btSourceSelect, SIGNAL(clicked()),
            this, SLOT(selectRecordDevice()));
    connect(cbDevice, SIGNAL(textActivated(QString)),
            this, SLOT(setDevice(QString)));

    // setup controls
    connect(chkRecordTime, SIGNAL(toggled(bool)),
            this, SLOT(recordTimeChecked(bool)));
    connect(sbRecordTime, SIGNAL(valueChanged(int)),
            this, SLOT(recordTimeChanged(int)));

    connect(chkRecordStartTime, SIGNAL(toggled(bool)),
            this, SLOT(startTimeChecked(bool)));
    connect(startTime, SIGNAL(dateTimeChanged(QDateTime)),
            this, SLOT(startTimeChanged(QDateTime)));

    connect(chkRecordTrigger, SIGNAL(toggled(bool)),
            this, SLOT(triggerChecked(bool)));
    connect(sbRecordTrigger, SIGNAL(valueChanged(int)),
            this, SLOT(triggerChanged(int)));

    connect(cbFormatSampleRate, SIGNAL(editTextChanged(QString)),
            this, SLOT(sampleRateChanged(QString)));
    connect(cbFormatSampleRate, SIGNAL(textActivated(QString)),
            this, SLOT(sampleRateChanged(QString)));

    connect(sbFormatTracks, SIGNAL(valueChanged(int)),
            this, SLOT(tracksChanged(int)));

    connect(cbFormatCompression, SIGNAL(activated(int)),
            this, SLOT(compressionChanged(int)));

    connect(sbFormatResolution, SIGNAL(valueChanged(int)),
            this, SLOT(bitsPerSampleChanged(int)));

    connect(cbFormatSampleFormat, SIGNAL(activated(int)),
            this, SLOT(sampleFormatChanged(int)));

    // connect the buttons to the record controller
    connect(btNew, SIGNAL(clicked()),
            controller, SLOT(actionReset()));
    connect(btStop, SIGNAL(clicked()),
            controller, SLOT(actionStop()));
    connect(btPause, SIGNAL(clicked()),
            controller, SLOT(actionPause()));
    connect(btRecord, SIGNAL(clicked()),
            controller, SLOT(actionStart()));

    // stop recording when the window gets closed
    connect(this, SIGNAL(rejected()), controller, SLOT(actionStop()));
    connect(this, SIGNAL(accepted()), controller, SLOT(actionStop()));

    // connect the notifications/commands of the record controller
    connect(controller, SIGNAL(stateChanged(Kwave::RecordState)),
            this, SLOT(setState(Kwave::RecordState)));

    // timer for updating the buffer progress bar
    connect(&m_buffer_progress_timer, SIGNAL(timeout()),
            this, SLOT(updateBufferProgressBar()));

    // help button
    connect(buttonBox->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this,   SLOT(invokeHelp()));

    // status bar
    m_state_icon_widget = new(std::nothrow) Kwave::StatusWidget(this);
    Q_ASSERT(m_state_icon_widget);
    if (!m_state_icon_widget) return;

    m_state_icon_widget->setFixedSize(16, 16);
    lbl_state->addWidget(m_state_icon_widget);

    m_status_bar.m_state = new(std::nothrow) QLabel(_(" "));
    Q_ASSERT(m_status_bar.m_state);
    if (!m_status_bar.m_state) return;
    m_status_bar.m_state->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lbl_state->addWidget(m_status_bar.m_state);

    m_status_bar.m_time = new(std::nothrow) QLabel(_(" "));
    Q_ASSERT(m_status_bar.m_time);
    if (!m_status_bar.m_time) return;
    m_status_bar.m_time->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lbl_state->addWidget(m_status_bar.m_time);

    m_status_bar.m_sample_rate = new(std::nothrow) QLabel(_(" "));
    Q_ASSERT(m_status_bar.m_sample_rate);
    if (!m_status_bar.m_sample_rate) return;
    m_status_bar.m_sample_rate->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lbl_state->addWidget(m_status_bar.m_sample_rate);

    m_status_bar.m_bits_per_sample = new(std::nothrow) QLabel(_(" "));
    Q_ASSERT(m_status_bar.m_bits_per_sample);
    if (!m_status_bar.m_bits_per_sample) return;
    m_status_bar.m_bits_per_sample->setAlignment(
        Qt::AlignRight | Qt::AlignVCenter);
    lbl_state->addWidget(m_status_bar.m_bits_per_sample);

    m_status_bar.m_tracks = new(std::nothrow) QLabel(_(" "));
    Q_ASSERT(m_status_bar.m_tracks);
    if (!m_status_bar.m_tracks) return;
    m_status_bar.m_tracks->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    lbl_state->addWidget(m_status_bar.m_tracks);

    m_state_icon_widget->setFixedSize(16, lbl_state->childrenRect().height());

    // set the initial state of the dialog to "Reset/Empty"
    setState(Kwave::REC_EMPTY);

    // disable the "level" tab, it is not implemented yet
    tabRecord->setCurrentIndex(1);
    QWidget *page = tabRecord->currentWidget();
    tabRecord->setCurrentIndex(0);
    if (page) delete page;

    // add the "Done" button manually, otherwise it would have "Cancel" semantic
    QPushButton *bt_done =
        buttonBox->addButton(i18n("&Done"), QDialogButtonBox::AcceptRole);
    Q_ASSERT(bt_done);
    if (!bt_done) return;
    connect(bt_done, SIGNAL(clicked(bool)), this, SLOT(accept()));

    switch (mode)
    {
        case Kwave::RecordDialog::SETTINGS_FORMAT:
            tabRecord->setCurrentIndex(1);
            break;
        case Kwave::RecordDialog::SETTINGS_SOURCE:
            tabRecord->setCurrentIndex(2);
            break;
        case Kwave::RecordDialog::START_RECORDING:  /* FALLTHROUGH */
        case Kwave::RecordDialog::SETTINGS_DEFAULT: /* FALLTHROUGH */
        default:
            tabRecord->setCurrentIndex(0);
            // set the focus onto the "Record" button
            btRecord->setFocus();
            break;
    }
}

//***************************************************************************
Kwave::RecordDialog::~RecordDialog()
{
    updateBufferState(0,0);
}

//***************************************************************************
Kwave::RecordParams &Kwave::RecordDialog::params()
{
    return m_params;
}

//***************************************************************************
void Kwave::RecordDialog::setMethod(Kwave::record_method_t method)
{
    m_params.method = method;
    cbMethod->setCurrentIndex(m_methods_map.findFromData(
        m_params.method));
}

//***************************************************************************
void Kwave::RecordDialog::methodSelected(int index)
{
    Kwave::record_method_t method = m_methods_map.data(index);
//     qDebug("RecordDialog::methodSelected(%d) - %d", index, (int)method);

    Q_ASSERT(method > Kwave::RECORD_NONE);
    Q_ASSERT(method < Kwave::RECORD_INVALID);
    if (method <= Kwave::RECORD_NONE) return;
    if (method >= Kwave::RECORD_INVALID) return;

    if (method != m_params.method) {
        setMethod(method);
        emit sigMethodChanged(method);
    }
}

//***************************************************************************
void Kwave::RecordDialog::setSupportedDevices(QStringList devices)
{
//     qDebug("RecordDialog::setSupportedDevices(QStringList devices)");
    Q_ASSERT(cbDevice);
    Q_ASSERT(listDevices);
    if (!cbDevice || !listDevices) return;
    QString current_device = m_params.device_name;

    // disable all that noisy stuff that comes from modifying the
    // device controls...
    m_enable_setDevice = false;

    KIconLoader *icon_loader = KIconLoader::global();

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
        btSourceSelect->setEnabled(true);
        btSourceSelect->show();
    } else {
        btSourceSelect->setEnabled(false);
        btSourceSelect->hide();
    }

    if (devices.contains(_("#TREE#"))) {
        // treeview mode
        devices.removeAll(_("#TREE#"));
        listDevices->setEnabled(true);
        cbDevice->setEnabled(false);
        cbDevice->hide();
        m_devices_list_map.clear();

        // build a tree with all nodes in the list
        foreach (QString dev_id, devices) {
            QTreeWidgetItem *parent = Q_NULLPTR;

            QStringList list = dev_id.split(_("||"), Qt::KeepEmptyParts);
            foreach (QString token, list) {
                QTreeWidgetItem *item = Q_NULLPTR;

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
                    /* parent = item; */
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
void Kwave::RecordDialog::listEntrySelected(QTreeWidgetItem *current,
                                            QTreeWidgetItem *previous)
{
    Q_ASSERT(listDevices);
    Q_UNUSED(previous)
    if (!current || !listDevices) return;

    if (m_devices_list_map.contains(current))
        setDevice(m_devices_list_map[current]);
}

//***************************************************************************
void Kwave::RecordDialog::listItemExpanded(QTreeWidgetItem *item)
{
    Q_UNUSED(item)
    updateListSelection();
}

//***************************************************************************
void Kwave::RecordDialog::updateListSelection()
{
    // set the current device again, otherwise nothing will be selected
    setDevice(m_params.device_name);
}

//***************************************************************************
void Kwave::RecordDialog::setDevice(const QString &device)
{
    Q_ASSERT(cbDevice);
    Q_ASSERT(listDevices);
    if (!cbDevice || !listDevices) return;

    bool device_changed = (device != m_params.device_name);
    m_params.device_name = device;
//     qDebug("RecordDialog::setDevice(%s)", device.local8Bit().data());

    if (listDevices->isEnabled()) {
        // treeview mode
        QTreeWidgetItem *node = m_devices_list_map.key(device, Q_NULLPTR);
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

    if (device_changed) emit sigDeviceChanged(device);
}

//***************************************************************************
void Kwave::RecordDialog::sourceBufferCountChanged(int value)
{
    Q_ASSERT(value >=  4);
    Q_ASSERT(value <= 64);
    if (value < 4)  value = 4;
    if (value > 64) value = 64;

    // take the value into our struct
    m_params.buffer_count = value;
    emit sigBuffersChanged();
}

//***************************************************************************
void Kwave::RecordDialog::sourceBufferSizeChanged(int value)
{
    Q_ASSERT(value >= 10);
    Q_ASSERT(value <= 18);
    if (value < 10) value = 10;
    if (value > 18) value = 18;

    // take the value into our struct
    m_params.buffer_size = value;

    // update the text
    unsigned int buffer_size = (1 << value);
    txtSourceBuffer->setText(i18n("%1 samples", buffer_size));

    emit sigBuffersChanged();
}

//***************************************************************************
void Kwave::RecordDialog::setFileFilter(const QString &filter)
{
    m_file_filter = filter;
    if (btSourceSelect) btSourceSelect->setEnabled(m_file_filter.length());
}

//***************************************************************************
void Kwave::RecordDialog::selectRecordDevice()
{
    if (!m_enable_setDevice) return;

    QString filter;
    filter += _("dsp*|")    + i18n("OSS record device (dsp*)");
    filter += _("\nadsp*|") + i18n("ALSA record device (adsp*)");
    filter += _("\n*|")     + i18n("Any device (*)");

    QPointer<Kwave::FileDialog> dlg = new(std::nothrow) Kwave::FileDialog(
        _("kfiledialog:///kwave_record_device"),
        Kwave::FileDialog::OpenFile, filter, this,
        QUrl(_("file:/dev"))
    );
    if (!dlg) return;
    dlg->setWindowTitle(i18n("Select Record Device"));
    if (!m_params.device_name.startsWith(_("#")))
        dlg->selectUrl(QUrl(_("file:") + m_params.device_name));
    else
        dlg->selectUrl(QUrl(_("file:/dev/*")));
    if (dlg->exec() == QDialog::Accepted) {
        // selected new device
        QString new_device = dlg->selectedUrl().path();
        if (new_device != m_params.device_name)
            emit sigDeviceChanged(new_device);
    }
    delete dlg;
}

//***************************************************************************
QString Kwave::RecordDialog::rate2string(double rate) const
{
    QLocale locale;
    const QString dot  = locale.decimalPoint();
    const QString tsep = locale.groupSeparator();

    // format number with 3 digits
    QString s = locale.toString(rate, 'f', 3);

    // remove thousands separator (looks ugly)
    s.remove(tsep);

    // remove trailing zeroes
    while (s.endsWith(_("0"))) s.remove(s.length()-1, 1);

    // remove decimal point if necessary
    if (s.endsWith(dot)) s.remove(s.length()-1, 1);

    return s;
}

//***************************************************************************
double Kwave::RecordDialog::string2rate(const QString &rate) const
{
    QLocale locale;
    const QString s = rate;
    double r;
    bool ok;
    r = locale.toDouble(rate, &ok);
    Q_ASSERT(ok);
    if (!ok) return s.toDouble();

    return r;
}

//***************************************************************************
void Kwave::RecordDialog::setSupportedTracks(unsigned int min,
                                             unsigned int max)
{
    Q_ASSERT(sbFormatTracks);
    if (!sbFormatTracks) return;

    if ((min == max) || (!max)) {
        sbFormatTracks->setEnabled(false);
        return;
    } else
        sbFormatTracks->setEnabled(true);

    if (sbFormatTracks->value() < sbFormatTracks->minimum()) {
        sbFormatTracks->setMaximum(max);
        sbFormatTracks->setMinimum(min);
    } else {
        sbFormatTracks->setMinimum(min);
        sbFormatTracks->setMaximum(max);
    }

}

//***************************************************************************
void Kwave::RecordDialog::setTracks(unsigned int tracks)
{
//     qDebug("+++ RecordDialog::setTracks(%u)", tracks);
    Q_ASSERT(sbFormatTracks);
    Q_ASSERT(m_status_bar.m_tracks);
    if (!sbFormatTracks || !m_status_bar.m_tracks) return;
    if (!tracks) return;

    m_params.tracks = tracks;
    QString tracks_str;

    switch (tracks) {
        case 1:
            tracks_str = i18n("Mono");
            break;
        case 2:
            tracks_str = i18n("Stereo");
            break;
        case 4:
            tracks_str = i18n("Quadro");
            break;
        default:
            tracks_str = _("");
    }

    if (tracks_str.length()) {
        lblTracksVerbose->setText(_("(") + tracks_str + _(")"));
        m_status_bar.m_tracks->setText(tracks_str);
    } else {
        lblTracksVerbose->setText(_(""));
        m_status_bar.m_tracks->setText(i18n("%1 tracks", tracks));
    }

    sbFormatTracks->setValue(tracks);
}

//***************************************************************************
void Kwave::RecordDialog::tracksChanged(int tracks)
{
    if (tracks < 1) return; // no device
    if (tracks == Kwave::toInt(m_params.tracks)) return;

    m_params.tracks = tracks;
    emit sigTracksChanged(tracks);
}

//***************************************************************************
void Kwave::RecordDialog::setSupportedSampleRates(const QList<double> &rates)
{
    Q_ASSERT(cbFormatSampleRate);
    if (!cbFormatSampleRate) return;

    cbFormatSampleRate->clearEditText();
    cbFormatSampleRate->setEditable(false);
    cbFormatSampleRate->clear();

    foreach (double r, rates) {
        QString rate = rate2string(r);
        Q_ASSERT(rate.length());
        if (!rate.length()) continue; // string was zero?
        cbFormatSampleRate->addItem(rate);
    }

    bool have_choice = (cbFormatSampleRate->count() > 1);
    cbFormatSampleRate->setEnabled(have_choice);
}

//***************************************************************************
void Kwave::RecordDialog::setSampleRate(double new_rate)
{
    Q_ASSERT(cbFormatSampleRate);
    Q_ASSERT(m_status_bar.m_sample_rate);
    if (!cbFormatSampleRate || !m_status_bar.m_sample_rate) return;

    if (new_rate <= 0) {
        cbFormatSampleRate->setEnabled(false);
        return;
    } else {
        bool have_choice = (cbFormatSampleRate->count() > 1);
        cbFormatSampleRate->setEnabled(have_choice);
        m_params.sample_rate = new_rate;
    }

    QString rate;
    rate = rate2string(new_rate);
    cbFormatSampleRate->setCurrentItem(rate, true);
    m_status_bar.m_sample_rate->setText(i18n("%1 Hz", rate));
}

//***************************************************************************
void Kwave::RecordDialog::sampleRateChanged(const QString &rate)
{
    if (!rate.length()) return; // no rate selected, combo box clear
    double sample_rate = string2rate(rate);
    if (qFuzzyCompare(sample_rate, m_params.sample_rate)) return;

    m_params.sample_rate = sample_rate;
    emit sampleRateChanged(sample_rate);
}

//***************************************************************************
void Kwave::RecordDialog::setSupportedCompressions(
    const QList<Kwave::Compression::Type> &comps
)
{
    Q_ASSERT(cbFormatCompression);
    if (!cbFormatCompression) return;

    cbFormatCompression->clear();

    if (comps.isEmpty()) {
        // no compressions -> add "none" manually
        const Kwave::Compression comp(Kwave::Compression::NONE);
        cbFormatCompression->addItem(comp.name());
    } else {
        foreach (Kwave::Compression::Type c, comps) {
            const Kwave::Compression comp(c);
            cbFormatCompression->addItem(comp.name(), comp.toInt());
        }
    }

    bool have_choice = (cbFormatCompression->count() > 1);
    cbFormatCompression->setEnabled(have_choice);
}

//***************************************************************************
void Kwave::RecordDialog::setCompression(int compression)
{
    Q_ASSERT(cbFormatCompression);
    if (!cbFormatCompression) return;

    if (compression < 0) {
        cbFormatCompression->setEnabled(false);
        return;
    } else {
        bool have_choice = (cbFormatCompression->count() > 1);
        cbFormatCompression->setEnabled(have_choice);
        m_params.compression = Kwave::Compression::fromInt(compression);
    }

    const Kwave::Compression comp(Kwave::Compression::fromInt(compression));
    cbFormatCompression->setCurrentItem(comp.name(), true);
}

//***************************************************************************
void Kwave::RecordDialog::compressionChanged(int index)
{
    Kwave::Compression::Type compression = Kwave::Compression::fromInt(
        cbFormatCompression->itemData(index).toInt());
    if (compression != m_params.compression)
        emit sigCompressionChanged(compression);
}

//***************************************************************************
void Kwave::RecordDialog::setSupportedBits(const QList<unsigned int> &bits)
{
    Q_ASSERT(sbFormatResolution);
    if (!sbFormatResolution) return;

    m_supported_resolutions = bits;
    if (bits.count()) {
        sbFormatResolution->setMinimum(bits.first());
        sbFormatResolution->setMaximum(bits.last());
    }

    // enable only if there is a choice
    sbFormatResolution->setEnabled(bits.count() > 1);
}

//***************************************************************************
void Kwave::RecordDialog::setBitsPerSample(unsigned int bits)
{
    Q_ASSERT(sbFormatResolution);
    Q_ASSERT(m_status_bar.m_bits_per_sample);
    if (!sbFormatResolution || !m_status_bar.m_bits_per_sample) return;

    if (!bits ) {
        sbFormatResolution->setEnabled(false);
        return;
    } else {
        sbFormatResolution->setEnabled(m_supported_resolutions.count() > 1);
        m_params.bits_per_sample = bits;
    }

    m_status_bar.m_bits_per_sample->setText(i18n("%1 bit", bits));
    sbFormatResolution->setValue(bits);
}

//***************************************************************************
void Kwave::RecordDialog::bitsPerSampleChanged(int bits)
{
    if (bits < 1) return; // no device
    int last = m_params.bits_per_sample;
    if (bits == last) return;

    // round up or down to the next supported resolution in bits per sample
    if (!m_supported_resolutions.isEmpty()) {
        if (bits > last) {
            // step up to the next supported value
            QListIterator<unsigned int> it(m_supported_resolutions);
            while (it.hasNext()) {
                bits = it.next();
                if (bits > last) break;
            }
            if (bits < last) bits = m_supported_resolutions.last();
        } else {
            // step down to the next supported value
            QListIterator<unsigned int> it(m_supported_resolutions);
            it.toBack();
            while (it.hasPrevious()) {
                bits = it.previous();
                if (bits < last) break;
            }
            if (bits > last) bits = m_supported_resolutions.first();
        }
    }

    m_params.bits_per_sample = bits;

    if (sbFormatResolution && (bits != sbFormatResolution->value()))
        sbFormatResolution->setValue(bits);

    emit sigBitsPerSampleChanged(bits);
}

//***************************************************************************
void Kwave::RecordDialog::setSupportedSampleFormats(
    const QList<Kwave::SampleFormat::Format> &formats)
{
    Q_ASSERT(cbFormatSampleFormat);
    if (!cbFormatSampleFormat) return;

    cbFormatSampleFormat->clear();
    Kwave::SampleFormat::Map types;
    foreach (Kwave::SampleFormat::Format format, formats) {
        int index = types.findFromData(format);
        cbFormatSampleFormat->addItem(
            types.description(index, true),
            Kwave::SampleFormat(format).toInt()
        );
    }

    bool have_choice = (cbFormatSampleFormat->count() > 1);
    cbFormatSampleFormat->setEnabled(have_choice);
}

//***************************************************************************
void Kwave::RecordDialog::setSampleFormat(
    Kwave::SampleFormat::Format sample_format)
{
    Q_ASSERT(cbFormatSampleFormat);
    if (!cbFormatSampleFormat) return;

    if (sample_format == Kwave::SampleFormat::Unknown) {
        cbFormatSampleFormat->setEnabled(false);
        return;
    } else {
        bool have_choice = (cbFormatSampleFormat->count() > 1);
        cbFormatSampleFormat->setEnabled(have_choice);
        m_params.sample_format = sample_format;
    }

    int cb_index = cbFormatSampleFormat->findData(
        Kwave::SampleFormat(sample_format).toInt());
    cbFormatSampleFormat->setCurrentIndex(cb_index);
}

//***************************************************************************
void Kwave::RecordDialog::sampleFormatChanged(int index)
{
    Q_ASSERT(cbFormatSampleFormat);
    if (!cbFormatSampleFormat) return;

    Kwave::SampleFormat format;
    format.fromInt(cbFormatSampleFormat->itemData(index).toInt());

    if (format == m_params.sample_format) return;

    emit sigSampleFormatChanged(format);
}

//***************************************************************************
void Kwave::RecordDialog::setState(Kwave::RecordState state)
{
    Q_ASSERT(m_status_bar.m_state);
    if (!m_status_bar.m_state) return;

    bool enable_new = false;
    bool enable_pause = false;
    bool enable_stop = false;
    bool enable_record = false;
    bool enable_settings = false;
    bool enable_trigger = false;
    QString state_text = _("");
    QVector<QPixmap> pixmaps;
    unsigned int animation_time = 500;

    m_state = state;
    switch (state) {
        case Kwave::REC_UNINITIALIZED:
            state_text = i18n("Please check the source device settings...");
            enable_new      = true;
            enable_pause    = false;
            enable_stop     = false;
            enable_record   = false;
            enable_settings = true;
            enable_trigger  = true;
            pixmaps.push_back(QPixmap(stop_hand_xpm));
            pixmaps.push_back(QPixmap(ledred_xpm));
            m_status_bar.m_time->setText(_(""));
            break;
        case Kwave::REC_EMPTY:
            state_text = i18n("(empty)");
            enable_new      = true;
            enable_pause    = false;
            enable_stop     = false;
            enable_record   = m_params.device_name.length();
            enable_settings = true;
            enable_trigger  = true;
            pixmaps.push_back(QPixmap(ledgreen_xpm));
            m_status_bar.m_time->setText(_(""));
            break;
        case Kwave::REC_BUFFERING:
            state_text = i18n("Buffering...");
            enable_new      = true; /* throw away current FIFO content */
            enable_pause    = false;
            enable_stop     = true;
            enable_record   = true; /* acts as "trigger now" */
            enable_settings = false;
            enable_trigger  = true;
            pixmaps.push_back(QPixmap(ledgreen_xpm));
            pixmaps.push_back(QPixmap(ledlightgreen_xpm));
            break;
        case Kwave::REC_PRERECORDING:
            state_text = i18n("Prerecording...");
            enable_new      = false;
            enable_pause    = false;
            enable_stop     = true;
            enable_record   = true;
            enable_settings = false;
            enable_trigger  = true;
            pixmaps.push_back(QPixmap(ledgreen_xpm));
            pixmaps.push_back(QPixmap(ledlightgreen_xpm));
            break;
        case Kwave::REC_WAITING_FOR_TRIGGER:
            state_text = i18n("Waiting for trigger...");
            enable_new      = false;
            enable_pause    = false;
            enable_stop     = true;
            enable_record   = true; /* acts as "trigger now" */
            enable_settings = false;
            enable_trigger  = true;
            pixmaps.push_back(QPixmap(ledgreen_xpm));
            pixmaps.push_back(QPixmap(ledlightgreen_xpm));
            break;
        case Kwave::REC_RECORDING:
            state_text = i18n("Recording...");
            enable_new      = false;
            enable_pause    = true;
            enable_stop     = true;
            enable_record   = false;
            enable_settings = false;
            enable_trigger  = false;
            pixmaps.push_back(QPixmap(walk_r1_xpm));
            pixmaps.push_back(QPixmap(walk_r2_xpm));
            pixmaps.push_back(QPixmap(walk_r3_xpm));
            pixmaps.push_back(QPixmap(walk_r4_xpm));
            pixmaps.push_back(QPixmap(walk_r5_xpm));
            pixmaps.push_back(QPixmap(walk_r6_xpm));
            pixmaps.push_back(QPixmap(walk_r7_xpm));
            pixmaps.push_back(QPixmap(walk_r8_xpm));
            animation_time = 100;
            break;
        case Kwave::REC_PAUSED:
            state_text = i18n("Paused");
            enable_new      = true; /* start again */
            enable_pause    = true; /* used for "continue" */
            enable_stop     = true;
            enable_record   = true; /* used for "continue" */
            enable_settings = false;
            enable_trigger  = false;
            pixmaps.push_back(QPixmap(ledgreen_xpm));
            pixmaps.push_back(QPixmap(ledyellow_xpm));
            break;
        case Kwave::REC_DONE:
            state_text = i18n("Done");
            enable_new      = true;
            enable_pause    = false;
            enable_stop     = false;
            enable_record   = true;
            enable_settings = true;
            enable_trigger  = true;
            pixmaps.push_back(QPixmap(ok_xpm));
            break;
    }
    m_status_bar.m_state->setText(state_text);
    m_state_icon_widget->setPixmaps(pixmaps, animation_time);

    // enable/disable the record control buttons
    btNew->setEnabled(enable_new);
    btPause->setEnabled(enable_pause);
    btStop->setEnabled(enable_stop);
    m_record_enabled = enable_record;
    updateRecordButton();

    // enable disable all controls (groups) for setup
    chkRecordPre->setEnabled(enable_settings);
    sbRecordPre->setEnabled(enable_settings &&
                            chkRecordPre->isChecked());
    slRecordPre->setEnabled(enable_settings &&
                            chkRecordPre->isChecked());

    chkRecordStartTime->setEnabled(enable_settings);
    chkRecordTime->setEnabled(enable_settings);
    sbRecordTime->setEnabled(enable_settings &&
                             chkRecordTime->isChecked());
    chkRecordTrigger->setEnabled(enable_settings);

    // it is not really necessary to disable these ;-)
    sbRecordTrigger->setEnabled(enable_trigger &&
                                chkRecordTrigger->isChecked());
    slRecordTrigger->setEnabled(enable_trigger &&
                                chkRecordTrigger->isChecked());
    startTime->setEnabled(enable_settings &&
                          chkRecordStartTime->isChecked());

    grpFormat->setEnabled(enable_settings);
    grpSource->setEnabled(enable_settings);

}

//***************************************************************************
void Kwave::RecordDialog::updateBufferState(unsigned int count,
                                            unsigned int total)
{
    Q_ASSERT(progress_bar);
    Q_ASSERT(m_status_bar.m_state);
    if (!progress_bar || !m_status_bar.m_state) return;

    if (total == 0) {
        // we are done: stop update timer and reset buffer percentage
        m_buffer_progress_timer.stop();
        m_buffer_progress_count = 0;
        m_buffer_progress_total = 0;
        progress_bar->setTextVisible(false);
        progress_bar->setMinimum(0);
        progress_bar->setMaximum(100);
        progress_bar->setValue(0);
        progress_bar->reset();
    } else {
        m_buffer_progress_count = count;
        m_buffer_progress_total = total;

        if (!m_buffer_progress_timer.isActive())
            updateBufferProgressBar();
    }

    // update recording time
    QString txt;
    switch (m_state) {
        case Kwave::REC_UNINITIALIZED:
        case Kwave::REC_EMPTY:
        case Kwave::REC_BUFFERING:
        case Kwave::REC_PRERECORDING:
            txt = _("");
            break;
        case Kwave::REC_WAITING_FOR_TRIGGER: {
            txt = _("");
            QString state_text;
            QDateTime now     = QDateTime::currentDateTime();
            QDateTime t_start = m_params.start_time;

            if (m_params.start_time_enabled && (now < t_start)) {
                // waiting for start time to come...

                int s = Kwave::toInt(now.secsTo(t_start));
                int m = s / 60;
                s %= 60;
                int h = m / 60;
                m %= 60;
                int d = h / 24;
                h %= 24;

                QString days    = (d) ?
                    i18np("one day ",    "%1 days ",    d) : _("");
                QString hours   = (h) ?
                    i18np("one hour ",   "%1 hours ",   h) : _("");
                QString minutes = (m) ?
                    i18np("one minute ", "%1 minutes ", m) : _("");
                QString seconds =
                    (d | h | m) ?
                    i18np("and %1 second", "and %1 seconds", s) :
                    i18np("%1 second", "%1 seconds", s);

                state_text = i18nc(
                    "%1=days; %2=hours; %3=minutes; %4=seconds",
                    "Waiting for start in %1%2%3%4...",
                    days, hours, minutes, seconds);
            } else {
                // waiting for trigger...
                state_text = i18n("Waiting for trigger...");
            }
            m_status_bar.m_state->setText(state_text);

            break;
        }
        case Kwave::REC_RECORDING:
        case Kwave::REC_PAUSED:
        case Kwave::REC_DONE: {
            if (m_samples_recorded > 1) {
                double rate = m_params.sample_rate;
                double ms = (rate > 0) ?
                    ((static_cast<double>(m_samples_recorded) / rate) * 1E3)
                    : 0;
                txt = _(" ") +
                    i18n("Length: %1", Kwave::ms2string(ms)) +
                    _(" ") + i18n("(%1 samples)",
                    Kwave::samples2string(m_samples_recorded));
            } else txt = _("");
            break;
        }
    }
    m_status_bar.m_time->setText(txt);
}

//***************************************************************************
void Kwave::RecordDialog::preRecordingChecked(bool enabled)
{
    m_params.pre_record_enabled = enabled;
    emit sigPreRecordingChanged(enabled);
}

//***************************************************************************
void Kwave::RecordDialog::preRecordingTimeChanged(int time)
{
    m_params.pre_record_time = time;
}

//***************************************************************************
void Kwave::RecordDialog::recordTimeChecked(bool limited)
{
    m_params.record_time_limited = limited;
    emit sigRecordTimeChanged(limited ? sbRecordTime->value() : -1);
}

//***************************************************************************
void Kwave::RecordDialog::recordTimeChanged(int limit)
{
    m_params.record_time = limit;
    emit sigRecordTimeChanged(chkRecordTime->isChecked() ?
                              limit : -1);
    updateRecordButton();
}

//***************************************************************************
void Kwave::RecordDialog::startTimeChecked(bool enabled)
{
    m_params.start_time_enabled = enabled;
    emit sigTriggerChanged(enabled || m_params.record_trigger_enabled);
}

//***************************************************************************
void Kwave::RecordDialog::startTimeChanged(const QDateTime &datetime)
{
    m_params.start_time = datetime;

    // force seconds to zero
    QTime t = m_params.start_time.time();
    t.setHMS(t.hour(), t.minute(), 0, 0);
    m_params.start_time.setTime(t);
}

//***************************************************************************
void Kwave::RecordDialog::triggerChecked(bool enabled)
{
    m_params.record_trigger_enabled = enabled;
    emit sigTriggerChanged(enabled || m_params.start_time_enabled);
}

//***************************************************************************
void Kwave::RecordDialog::triggerChanged(int trigger)
{
    m_params.record_trigger = trigger;
}

//***************************************************************************
void Kwave::RecordDialog::updateBufferProgressBar()
{
    unsigned int count = m_buffer_progress_count;
    unsigned int total = m_buffer_progress_total;
//     qDebug("RecordDialog::updateBufferProgressBar(): %u/%u",
//            count, total);

    /*
     * @note: QProgressBar has a bug when handling small numbers,
     *        therefore we multiply everything by 100
     */
    progress_bar->setTextVisible(true);
    progress_bar->setMinimum(0);
    progress_bar->setMaximum(100 * total);
    progress_bar->setValue(100 * count);

    m_buffer_progress_timer.setSingleShot(true);
    m_buffer_progress_timer.setInterval(100);
    m_buffer_progress_timer.start();
}

//***************************************************************************
void Kwave::RecordDialog::updateEffects(unsigned int track,
                                        Kwave::SampleArray &buffer)
{
    if (!buffer.size()) return;

    if (level_meter) {
        level_meter->setTracks(m_params.tracks);
        level_meter->setSampleRate(m_params.sample_rate);
        level_meter->updateTrack(track, buffer);
    }

}

//***************************************************************************
void Kwave::RecordDialog::setRecordedSamples(sample_index_t samples_recorded)
{
    // if (!m_params.record_time_limited) return; // not of interest
    m_samples_recorded = samples_recorded;
    updateRecordButton();
}

//***************************************************************************
void Kwave::RecordDialog::updateRecordButton()
{
    bool old_enable = btRecord->isEnabled();
    bool new_enable;

    // enabled if not disabled by status and also not limited or
    // less than the limit has been recorded
    new_enable = m_record_enabled && (!m_params.record_time_limited ||
        (static_cast<double>(m_samples_recorded) <
         m_params.record_time * m_params.sample_rate));

    if (new_enable != old_enable) btRecord->setEnabled(new_enable);
}

//***************************************************************************
void Kwave::RecordDialog::invokeHelp()
{
    KHelpClient::invokeHelp(_("recording"));
}

//***************************************************************************
void Kwave::RecordDialog::message(const QString &message)
{
    if (lbl_state) lbl_state->showMessage(message, 3000);
}

//***************************************************************************
void Kwave::RecordDialog::showDevicePage()
{
    if (tabRecord) tabRecord->setCurrentIndex(2);
}

//***************************************************************************
//***************************************************************************
