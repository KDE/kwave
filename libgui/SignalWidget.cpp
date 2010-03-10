/***************************************************************************
             SignalWidget.cpp  -  Widget for displaying the signal
			     -------------------
    begin                : 1999
    copyright            : (C) 1999 by Martin Wilz
    email                : Martin Wilz <mwilz@ernie.mi.uni-koeln.de>
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
#include <stdlib.h>
#include <sys/time.h>

#include <QBitmap>
#include <QContextMenuEvent>
#include <QDragLeaveEvent>
#include <QEvent>
#include <QFrame>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QTime>
#include <QToolTip>

#include <kcursor.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kiconloader.h>

#include "libkwave/ApplicationContext.h"
#include "libkwave/ClipBoard.h"
#include "libkwave/CodecManager.h"
#include "libkwave/FileInfo.h"
#include "libkwave/KwaveDrag.h"
#include "libkwave/Label.h"
#include "libkwave/LabelList.h"
#include "libkwave/MessageBox.h"
#include "libkwave/MultiTrackReader.h"
#include "libkwave/Parser.h"
#include "libkwave/SampleReader.h"
#include "libkwave/SignalManager.h"
#include "libkwave/Track.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/LabelPropertiesWidget.h"
#include "libgui/MenuManager.h"
#include "libgui/MultiStateWidget.h"
// #include "libgui/ShortcutWrapper.h"
#include "libgui/SignalView.h"
#include "libgui/SignalWidget.h"
#include "libgui/TrackPixmap.h"
#include "libgui/TrackView.h"

// /** table of keyboard shortcuts 0...9 */
// static const int tbl_keys[10] = {
//     Qt::Key_1,
//     Qt::Key_2,
//     Qt::Key_3,
//     Qt::Key_4,
//     Qt::Key_5,
//     Qt::Key_6,
//     Qt::Key_7,
//     Qt::Key_8,
//     Qt::Key_9,
//     Qt::Key_0
// };

/**
 * useful macro for command parsing
 */
#define CASE_COMMAND(x) } else if (parser.command() == x) {

/** number of milliseconds between repaints */
// #define REPAINT_INTERVAL 50

/** number of milliseconds until the position widget disappears */
// #define POSITION_WIDGET_TIME 5000

/** vertical zoom factor: minimum value */
// #define VERTICAL_ZOOM_MIN 1.0

/** vertical zoom factor: maximum value */
// #define VERTICAL_ZOOM_MAX 100.0

/** vertical zoom factor: increment/decrement factor */
// #define VERTICAL_ZOOM_STEP_FACTOR 1.5

//***************************************************************************
//***************************************************************************
// namespace KwaveFileDrag
// {
//     static bool canDecode(const QMimeData *source) {
// 	if (!source) return false;
//
// 	if (source->hasUrls()) {
// 	    // dropping URLs
// 	    foreach (QUrl url, source->urls()) {
// 		QString filename = url.toLocalFile();
// 		QString mimetype = CodecManager::whatContains(filename);
// 		if (CodecManager::canDecode(mimetype)) {
// 		    return true;
// 		}
// 	    }
// 	}
//
// 	foreach (QString format, source->formats()) {
// 	    // dropping known mime type
// 	    if (CodecManager::canDecode(format)) {
// 		qDebug("KwaveFileDrag::canDecode(%s)",
// 		       format.toLocal8Bit().data());
// 		return true;
// 	    }
// 	}
// 	return false;
//     }
// }

//***************************************************************************
//***************************************************************************
SignalWidget::SignalWidget(QWidget *parent, Kwave::ApplicationContext &context,
                           QVBoxLayout *upper_dock, QVBoxLayout *lower_dock)
    :QWidget(parent),
     m_context(context),
     m_views(),
     m_layout(this),
     m_upper_dock(upper_dock),
     m_lower_dock(lower_dock),
     m_offset(0),
     m_zoom(1.0)
//     m_offset(0),
//     m_zoom(0.0),
//     m_vertical_zoom(1.0),
//     m_playpointer(-1),
//     m_last_playpointer(-1),
//     m_redraw(false),
//     m_inhibit_repaint(0),
//     m_track_pixmaps(),
//     m_repaint_timer(this),
//     m_position_widget(this),
//     m_position_widget_timer(this)
{
//    qDebug("SignalWidget::SignalWidget()");

    // connect to the signal manager's signals
    SignalManager *sig = m_context.signalManager();

    connect(sig,  SIGNAL(sigTrackInserted(unsigned int, Track *)),
            this, SLOT( slotTrackInserted(unsigned int, Track *)));
    connect(sig,  SIGNAL(sigTrackDeleted(unsigned int)),
            this, SLOT( slotTrackDeleted(unsigned int)));

//     connect(sig, SIGNAL(sigSamplesDeleted(unsigned int, unsigned int,
// 	unsigned int)),
// 	this, SLOT(slotSamplesDeleted(unsigned int, unsigned int,
// 	unsigned int)));
//     connect(sig, SIGNAL(sigSamplesInserted(unsigned int, unsigned int,
// 	unsigned int)),
// 	this, SLOT(slotSamplesInserted(unsigned int, unsigned int,
// 	unsigned int)));
//     connect(sig, SIGNAL(sigSamplesModified(unsigned int, unsigned int,
// 	unsigned int)),
// 	this, SLOT(slotSamplesModified(unsigned int, unsigned int,
// 	unsigned int)));
//     connect(sig, SIGNAL(sigLabelCountChanged()),
//             this, SLOT(hidePosition()),
//             Qt::QueuedConnection);
//
//     // connect repaint timer
//     connect(&m_repaint_timer, SIGNAL(timeout()),
//             this, SLOT(timedRepaint()));
//
//     // connect the timer of the position widget
//     connect(&m_position_widget_timer, SIGNAL(timeout()),
//             &m_position_widget, SLOT(hide()));
//
// //    m_menu_manager.clearNumberedMenu("ID_LABELS_TYPE");
// //    for (LabelType *tmp = globals.markertypes.first(); tmp;
// //         tmp = globals.markertypes.next())
// //    {
// //	m_menu_manager.addNumberedMenuEntry("ID_LABELS_TYPE", (char *)tmp->name);
// //    }
// //
// //    markertype = globals.markertypes.first();

//     // -- accelerator keys for 1...9 --
//     for (int i = 0; i < 10; i++) {
// 	Kwave::ShortcutWrapper *shortcut =
// 	    new Kwave::ShortcutWrapper(this, tbl_keys[i], i);
// 	connect(shortcut, SIGNAL(activated(int)),
// 	        this, SLOT(parseKey(int)));
//     }

    m_layout.setColumnStretch(0,   0);
    m_layout.setColumnStretch(1, 100);
    m_layout.setMargin(3);
    m_layout.setSpacing(3);
    setLayout(&m_layout);

    setMinimumHeight(200);

//    qDebug("SignalWidget::SignalWidget(): done.");
}

//***************************************************************************
bool SignalWidget::isOK()
{
    return true;
}

//***************************************************************************
SignalWidget::~SignalWidget()
{
}

//***************************************************************************
void SignalWidget::setZoomAndOffset(double zoom, sample_index_t offset)
{
    foreach (QPointer<Kwave::SignalView> view, m_views)
	view->setZoomAndOffset(zoom, offset);
}

// //***************************************************************************
int SignalWidget::executeCommand(const QString &command)
{
//     InhibitRepaintGuard inhibit(*this); ### TODO ###
    Parser parser(command);
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return false;

    if (!command.length()) return true;

    if (false) {
    // label commands
    CASE_COMMAND("label")
	unsigned int pos = parser.toUInt();
	addLabel(pos);
    CASE_COMMAND("deletelabel")
	int index = parser.toInt();
	signal_manager->deleteLabel(index, true);
//    CASE_COMMAND("chooselabel")
//	Parser parser(command);
//	markertype = globals.markertypes.at(parser.toInt());
//    CASE_COMMAND("amptolabel")
//	markSignal(command);
//    CASE_COMMAND("pitch")
//	markPeriods(command);
//    CASE_COMMAND("labeltopitch")
//      convertMarkstoPitch(command);
//    CASE_COMMAND("loadlabel")  -> plugin
//	loadLabel();
//    CASE_COMMAND("savelabel")  -> plugin
//	saveLabel(command);
    CASE_COMMAND("expandtolabel")
	UndoTransactionGuard undo(*signal_manager,
	    i18n("Expand Selection to Label"));
	unsigned int selection_left  = signal_manager->selection().first();
	unsigned int selection_right = signal_manager->selection().last();
	LabelList &labels = signal_manager->labels();
	if (labels.isEmpty()) return false; // we need labels for this
	Label label_left  = Label();
	Label label_right = Label();
	// the last label <= selection start -> label_left
	// the first label >= selection end  -> label_right
	foreach (Label label, labels) {
	    unsigned int lp = label.pos();
	    if (lp <= selection_left)
		label_left = label;
	    if ((lp >= selection_right) && (label_right.isNull())) {
		label_right = label;
		break; // done
	    }
	}
	// default left label = start of file
	selection_left = (label_left.isNull()) ? 0 :
	    label_left.pos();
	// default right label = end of file
	selection_right = (label_right.isNull()) ?
	    signal_manager->length() - 1 : label_right.pos();
	unsigned int length = selection_right - selection_left + 1;
	signal_manager->selectRange(selection_left, length);

    CASE_COMMAND("selectnextlabels")
	UndoTransactionGuard undo(*signal_manager,
	    i18n("Select Next Labels"));
	unsigned int selection_left;
	unsigned int selection_right = signal_manager->selection().last();
	Label label_left  = Label();
	Label label_right = Label();
	LabelList &labels = signal_manager->labels();
	if (labels.isEmpty()) return false; // we need labels for this

	// special case: nothing selected -> select up to the first label
	if (selection_right == 0) {
	    label_right = labels.first();
	    selection_left = 0;
	} else {
	    // find the first label starting after the current selection
	    LabelListIterator it(labels);
	    while (it.hasNext()) {
		Label label = it.next();
		if (label.pos() >= selection_right) {
		    // take it as selection start
		    label_left  = label;
		    // and it's next one as selection end (might be null)
		    label_right = it.hasNext() ? it.next() : Label();
		    break;
		}
	    }
	    // default selection start = last label
	    if (label_left.isNull()) label_left = labels.last();
	    if (label_left.isNull()) return false; // no labels at all !?
	    selection_left = label_left.pos();
	}
	// default selection end = end of the file
	selection_right = (label_right.isNull()) ?
	    signal_manager->length() - 1 : label_right.pos();
	unsigned int length = selection_right - selection_left + 1;
	signal_manager->selectRange(selection_left, length);

    CASE_COMMAND("selectprevlabels")
	UndoTransactionGuard undo(*signal_manager,
	    i18n("Select Previous Labels"));
	unsigned int selection_left  = signal_manager->selection().first();
	unsigned int selection_right = signal_manager->selection().last();
	Label label_left  = Label();
	Label label_right = Label();
	LabelList &labels = signal_manager->labels();
	if (labels.isEmpty()) return false; // we need labels for this

	// find the last label before the start of the selection
	foreach (Label label, labels) {
	    if (label.pos() > selection_left)
		break; // done
	    label_left  = label_right;
	    label_right = label;
	}
	// default selection start = start of file
	selection_left = (label_left.isNull()) ? 0 :
	    label_left.pos();
	// default selection end = first label
	if (label_right.isNull()) label_right = labels.first();
	if (label_right.isNull()) return false; // no labels at all !?
	selection_right = label_right.pos();
	unsigned int length = selection_right - selection_left + 1;
	signal_manager->selectRange(selection_left, length);

//    CASE_COMMAND("markperiod")
//	markPeriods(command);
//    CASE_COMMAND("saveperiods")
//	savePeriods();
    }

    return true;
}

//***************************************************************************
void SignalWidget::forwardCommand(const QString &command)
{
    emit sigCommand(command);
}

// //***************************************************************************
// void SignalWidget::setMouseMode(MouseMode mode)
// {
//     if (mode == m_mouse_mode) return;
//
//     m_mouse_mode = mode;
//     switch (mode) {
// 	case MouseNormal:
// 	    setCursor(Qt::ArrowCursor);
// 	    break;
// 	case MouseAtSelectionBorder:
// 	    setCursor(Qt::SizeHorCursor);
// 	    break;
// 	case MouseInSelection:
// 	    setCursor(Qt::ArrowCursor);
// 	    break;
// 	case MouseSelect:
// 	    setCursor(Qt::SizeHorCursor);
// 	    break;
//     }
//
//     emit sigMouseChanged(static_cast<int>(mode));
// }


// //***************************************************************************
// void SignalWidget::mousePressEvent(QMouseEvent *e)
// {
//     Q_ASSERT(e);
//     if (!e) return;
//
//     SignalManager *signal_manager = m_context.signalManager();
//     Q_ASSERT(signal_manager);
//     if (!signal_manager) return;
//
//     // abort if no signal is loaded
//     if (!signal_manager->length()) return;
//
//     // ignore all mouse press events in playback mode
//     if (signal_manager->playbackController().running()) return;
//
//     if (e->button() == Qt::LeftButton) {
// 	int mx = e->pos().x();
// 	if (mx < 0) mx = 0;
// 	if (mx >= m_width) mx = m_width-1;
// 	unsigned int x = m_offset + pixels2samples(mx);
// 	unsigned int len = signal_manager->selection().length();
// 	switch (e->modifiers()) {
// 	    case Qt::ShiftModifier: {
// 		// expand the selection to "here"
// 		m_selection->set(signal_manager->selection().first(),
// 		                 signal_manager->selection().last());
// 		m_selection->grep(x);
// 		selectRange(m_selection->left(), m_selection->length());
// 		setMouseMode(MouseSelect);
// 		break;
// 	    }
// 	    case Qt::ControlModifier: {
// 		if (isInSelection(e->pos().x()) && (len > 1)) {
// 		    // start a drag&drop operation in "copy" mode
// 		    startDragging();
// 		}
// 		break;
// 	    }
// 	    case 0: {
// 		if (isSelectionBorder(e->pos().x())) {
// 		    // modify selection border
// 		    m_selection->set(signal_manager->selection().first(),
// 			             signal_manager->selection().last());
// 		    m_selection->grep(x);
// 		    selectRange(m_selection->left(), m_selection->length());
// 		    setMouseMode(MouseSelect);
// 		} else if (isInSelection(e->pos().x()) && (len > 1)) {
// 		    // store the x position for later drag&drop
// 		    m_mouse_down_x = e->pos().x();
// 		} else {
// 		    // start a new selection
// 		    m_selection->set(x, x);
// 		    selectRange(x, 0);
// 		    setMouseMode(MouseSelect);
// 		}
// 		break;
// 	    }
// 	}
//     }
// }

//***************************************************************************
void SignalWidget::contextMenuEvent(QContextMenuEvent *e)
{
    Q_ASSERT(e);

    SignalManager *manager = m_context.signalManager();
    bool have_signal = manager && !manager->isEmpty();
    if (!have_signal)return;
    bool have_selection = manager && (manager->selection().length() > 1);
    bool have_labels = !manager && manager->labels().isEmpty();

    QMenu *context_menu = new QMenu(this);
    Q_ASSERT(context_menu);
    if (!context_menu) return;

    QMenu *submenu_select = context_menu->addMenu(i18n("&Selection"));
    Q_ASSERT(submenu_select);
    if (!submenu_select) return;

    QMenu *submenu_label = context_menu->addMenu(i18n("&Label"));
    Q_ASSERT(submenu_label);
    if (!submenu_label) return;

    KIconLoader icon_loader;

    /* menu items common to all cases */

    // undo
    QAction *action;
    action = context_menu->addAction(
	icon_loader.loadIcon("edit-undo", KIconLoader::Toolbar),
	i18n("&Undo"), this, SLOT(contextMenuEditUndo()),
	Qt::CTRL + Qt::Key_Z);
    Q_ASSERT(action);
    if (!action) return;
    if (!manager || !manager->canUndo())
	action->setEnabled(false);

    // redo
    action = context_menu->addAction(
	icon_loader.loadIcon("edit-redo", KIconLoader::Toolbar),
	i18n("&Redo"), this, SLOT(contextMenuEditRedo()),
	Qt::CTRL + Qt::Key_Y);
    Q_ASSERT(action);
    if (!action) return;
    if (!manager || !manager->canRedo())
	action->setEnabled(false);
    context_menu->addSeparator();

    // cut/copy/paste
    QAction *action_cut = context_menu->addAction(
	icon_loader.loadIcon("edit-cut", KIconLoader::Toolbar),
	i18n("Cu&t"), this, SLOT(contextMenuEditCut()),
	Qt::CTRL + Qt::Key_X);
    QAction *action_copy = context_menu->addAction(
	icon_loader.loadIcon("edit-copy", KIconLoader::Toolbar),
	i18n("&Copy"), this, SLOT(contextMenuEditCopy()),
	Qt::CTRL + Qt::Key_C);
    QAction *action_paste = context_menu->addAction(
	icon_loader.loadIcon("edit-paste", KIconLoader::Toolbar),
	i18n("&Paste"), this, SLOT(contextMenuEditPaste()),
	Qt::CTRL + Qt::Key_V);
    context_menu->addSeparator();
    if (action_cut)   action_cut->setEnabled(have_selection);
    if (action_copy)  action_copy->setEnabled(have_selection);
    if (action_paste)
	action_paste->setEnabled(!ClipBoard::instance().isEmpty());

    int mouse_x = mapFromGlobal(e->globalPos()).x();
    if (mouse_x < 0) mouse_x = 0;
    if (mouse_x >= width())  mouse_x = width()  - 1;

    // Selection / &Save
    QAction *action_select_save = submenu_select->addAction(
	icon_loader.loadIcon("document-save", KIconLoader::Toolbar),
	i18n("&Save..."), this, SLOT(contextMenuSaveSelection()));
    Q_ASSERT(action_select_save);
    if (!action_select_save) return;
    action_select_save->setEnabled(have_selection);

    // Selection / &Expand to labels
    QAction *action_select_expand_to_labels = submenu_select->addAction(
	i18n("&Expand to Labels"), this,
	SLOT(contextMenuSelectionExpandToLabels()), Qt::Key_E);
    Q_ASSERT(action_select_expand_to_labels);
    if (!action_select_expand_to_labels) return;
    action_select_expand_to_labels->setEnabled(have_labels);

    // Selection / to next labels
    QAction *action_select_next_labels = submenu_select->addAction(
	i18n("To Next Labels"), this,
	SLOT(contextMenuSelectionNextLabels()),
	Qt::SHIFT + Qt::CTRL + Qt::Key_N);
    Q_ASSERT(action_select_next_labels);
    if (!action_select_next_labels) return;
    action_select_next_labels->setEnabled(have_labels);

    // Selection / to previous labels
    QAction *action_select_prev_labels = submenu_select->addAction(
	i18n("To Previous Labels"), this,
	SLOT(contextMenuSelectionPrevLabels()),
	Qt::SHIFT + Qt::CTRL + Qt::Key_P);
    Q_ASSERT(action_select_prev_labels);
    if (!action_select_prev_labels) return;
    action_select_prev_labels->setEnabled(have_labels);

    // label handling
    QAction *action_label_new = submenu_label->addAction(
	icon_loader.loadIcon("list-add", KIconLoader::Toolbar),
	i18n("&New"), this, SLOT(contextMenuLabelNew()));
    Q_ASSERT(action_label_new);
    if (!action_label_new) return;
    action_label_new->setEnabled(have_signal);

    QAction *action_label_delete = submenu_label->addAction(
	icon_loader.loadIcon("list-remove", KIconLoader::Toolbar),
	i18n("&Delete"), this, SLOT(contextMenuLabelDelete()));
    Q_ASSERT(action_label_delete);
    if (!action_label_delete) return;
    action_label_delete->setEnabled(false);

    QAction *action_label_properties = submenu_label->addAction(
	icon_loader.loadIcon("configure", KIconLoader::Toolbar),
	i18n("&Properties..."), this, SLOT(contextMenuLabelProperties()));
    Q_ASSERT(action_label_properties);
    if (!action_label_properties) return;
    action_label_properties->setEnabled(false);

//     // store the menu position in the mouse selection
//     unsigned int pos = m_offset + pixels2samples(mouse_x);
//     m_selection->set(pos, pos);
//
//     Label label;
//     if (!((label = findLabelNearMouse(mouse_x)).isNull())) {
// 	// delete label ?
// 	// label properties ?
// 	action_label_new->setEnabled(false);
// 	action_label_properties->setEnabled(true);
// 	action_label_delete->setEnabled(true);
//
// 	pos = label.pos();
// 	m_selection->set(pos, pos);
//     }
//
//     if (isSelectionBorder(mouse_x)) {
// 	// context menu: do something with the selection border
//
// 	// expand to next marker (right) ?
// 	// expand to next marker (left) ?
//     }
//
//     if (isInSelection(mouse_x) && have_selection) {
// 	// context menu: do something with the selection
//     }

    context_menu->exec(QCursor::pos());
    delete context_menu;
}

//***************************************************************************
void SignalWidget::contextMenuLabelNew()
{
//     forwardCommand(QString("label(%1)").arg(m_selection->left()));
}

//***************************************************************************
void SignalWidget::contextMenuLabelDelete()
{
//     SignalManager *signal_manager = m_context.signalManager();
//     Q_ASSERT(signal_manager);
//     if (!signal_manager) return;
//
//     Label label = signal_manager->findLabel(m_selection->left());
//     if (label.isNull()) return;
//     int index = signal_manager->labelIndex(label);
//     forwardCommand(QString("deletelabel(%1)").arg(index));
}

//***************************************************************************
void SignalWidget::contextMenuLabelProperties()
{
//     SignalManager *signal_manager = m_context.signalManager();
//     Q_ASSERT(signal_manager);
//     if (!signal_manager) return;
//
//     Label label = signal_manager->findLabel(m_selection->left());
//     if (label.isNull()) return;
//
//     labelProperties(label);
}

// //***************************************************************************
// void SignalWidget::mouseReleaseEvent(QMouseEvent *e)
// {
//     Q_ASSERT(e);
//     Q_ASSERT(m_selection);
//     if (!e) return;
//     if (!m_selection) return;
//
//     SignalManager *signal_manager = m_context.signalManager();
//     Q_ASSERT(signal_manager);
//     if (!signal_manager) return;
//
//     // abort if no signal is loaded
//     if (!signal_manager->length()) return;
//
//     // ignore all mouse release events in playback mode
//     if (signal_manager->playbackController().running()) return;
//
//     switch (m_mouse_mode) {
// 	case MouseSelect: {
// 	    unsigned int x = m_offset + pixels2samples(e->pos().x());
// 	    m_selection->update(x);
// 	    selectRange(m_selection->left(), m_selection->length());
// 	    setMouseMode(MouseNormal);
// 	    hidePosition();
// 	    break;
// 	}
// 	case MouseInSelection: {
// 	    int dmin = KGlobalSettings::dndEventDelay();
// 	    if ((e->button() & Qt::LeftButton) &&
// 		    ((e->pos().x() >= m_mouse_down_x - dmin) ||
// 		     (e->pos().x() <= m_mouse_down_x + dmin)) )
// 	    {
// 		// deselect if only clicked without moving
// 		unsigned int pos = m_offset + pixels2samples(e->pos().x());
// 		selectRange(pos, 0);
// 		setMouseMode(MouseNormal);
// 		hidePosition();
// 	    }
// 	    break;
// 	}
// 	default: ;
//     }
//
// }

// //***************************************************************************
// SignalWidget::PositionWidget::PositionWidget(QWidget *parent)
//     :QWidget(parent), m_label(0), m_alignment(0),
//      m_radius(10), m_arrow_length(30), m_last_alignment(Qt::AlignHCenter),
//      m_last_size(QSize(0,0)), m_polygon()
// {
//     hide();
//
//     m_label = new QLabel(this);
//     Q_ASSERT(m_label);
//     if (!m_label) return;
//
//     m_label->setFrameStyle(QFrame::Panel | QFrame::Plain);
//     m_label->setPalette(QToolTip::palette()); // use same colors as a QToolTip
//     m_label->setFocusPolicy(Qt::NoFocus);
//     m_label->setMouseTracking(true);
//     m_label->setLineWidth(0);
//
//     setPalette(QToolTip::palette()); // use same colors as a QToolTip
//     setFocusPolicy(Qt::NoFocus);
//     setMouseTracking(true);
// }

// //***************************************************************************
// SignalWidget::PositionWidget::~PositionWidget()
// {
//     if (m_label) delete m_label;
//     m_label = 0;
// }

// //***************************************************************************
// void SignalWidget::PositionWidget::setText(const QString &text,
//                                            Qt::Alignment alignment)
// {
//     if (!m_label) return;
//
//     m_alignment = alignment;
//
//     m_label->setText(text);
//     m_label->setAlignment(m_alignment);
//     m_label->resize(m_label->sizeHint());
//
//     switch (m_alignment) {
// 	case Qt::AlignLeft:
// 	    resize(m_arrow_length + m_radius + m_label->width() + m_radius,
// 	           m_radius + m_label->height() + m_radius);
// 	    m_label->move(m_arrow_length + m_radius, m_radius);
// 	    break;
// 	case Qt::AlignRight:
// 	    resize(m_radius + m_label->width() + m_radius + m_arrow_length,
// 	           m_radius + m_label->height() + m_radius);
// 	    m_label->move(m_radius, m_radius);
// 	    break;
// 	case Qt::AlignHCenter:
// 	    resize(m_radius + m_label->width() + m_radius,
// 	           m_arrow_length + m_radius + m_label->height() + m_radius);
// 	    m_label->move(m_radius, m_arrow_length + m_radius);
// 	    break;
// 	default:
// 	    ;
//     }
//
//     updateMask();
// }

// //***************************************************************************
// bool SignalWidget::PositionWidget::event(QEvent *e)
// {
//     if (!e) return false;
//
//     // ignore any kind of event that might be of interest
//     // for the parent widget (in our case the SignalWidget)
//     switch (e->type()) {
// 	case QEvent::MouseButtonPress:
// 	case QEvent::MouseButtonRelease:
// 	case QEvent::MouseButtonDblClick:
// 	case QEvent::MouseMove:
// 	case QEvent::KeyPress:
// 	case QEvent::KeyRelease:
// 	case QEvent::Shortcut:
// 	case QEvent::Wheel:
// 	case QEvent::Clipboard:
// 	case QEvent::Speech:
// 	case QEvent::DragEnter:
// 	case QEvent::DragMove:
// 	case QEvent::DragLeave:
// 	case QEvent::Drop:
// 	case QEvent::DragResponse:
// 	case QEvent::TabletMove:
// 	case QEvent::TabletPress:
// 	case QEvent::TabletRelease:
// 	    return false;
// 	default:
// 	    ;
//     }
//
//     // everything else: let it be handled by QLabel
//     return QWidget::event(e);
// }

// //***************************************************************************
// void SignalWidget::PositionWidget::updateMask()
// {
//     // bail out if nothing has changed
//     if ((size() == m_last_size) && (m_alignment == m_last_alignment))
// 	return;
//
//     QPainter p;
//     QBitmap bmp(size());
//     bmp.fill(Qt::color0);
//     p.begin(&bmp);
//
//     QBrush brush(Qt::color1);
//     p.setBrush(brush);
//     p.setPen(Qt::color1);
//
//     const int h = height();
//     const int w = width();
//
//     // re-create the polygon, depending on alignment
//     switch (m_alignment) {
// 	case Qt::AlignLeft:
// 	    m_polygon.setPoints(8,
// 		m_arrow_length, 0,
// 		w-1, 0,
// 		w-1, h-1,
// 		m_arrow_length, h-1,
// 		m_arrow_length, 2*h/3,
// 		0, h/2,
// 		m_arrow_length, h/3,
// 		m_arrow_length, 0
// 	    );
// 	    break;
// 	case Qt::AlignRight:
// 	    m_polygon.setPoints(8,
// 		0, 0,
// 		w-1-m_arrow_length, 0,
// 		w-1-m_arrow_length, h/3,
// 		w-1, h/2,
// 		w-1-m_arrow_length, 2*h/3,
// 		w-1-m_arrow_length, h-1,
// 		0, h-1,
// 		0, 0
// 	    );
// 	    break;
// 	case Qt::AlignHCenter:
// 	    break;
// 	default:
// 	    ;
//     }
//
//     p.drawPolygon(m_polygon);
//     p.end();
//
//     // activate the new widget mask
//     clearMask();
//     setMask(bmp);
//
//     // remember size/alignment for detecing changes
//     m_last_alignment = m_alignment;
//     m_last_size      = size();
// }

// //***************************************************************************
// void SignalWidget::PositionWidget::paintEvent(QPaintEvent *)
// {
//     QPainter p(this);
//     p.setBrush(palette().background().color());
//     p.drawPolygon(m_polygon);
// }

// //***************************************************************************
// void SignalWidget::showPosition(const QString &text, unsigned int pos,
//                                 double ms, const QPoint &mouse)
// {
//     int x = mouse.x();
//     int y = mouse.y();
//
//     // x/y == -1/-1 -> reset/hide the position
//     if ((x < 0) && (y < 0)) {
// 	m_position_widget_timer.stop();
// 	m_position_widget.hide();
// 	return;
//     }
//
//     setUpdatesEnabled(false);
//     m_position_widget.hide();
//
//     unsigned int t, h, m, s, tms;
//     t = static_cast<unsigned int>(rint(ms * 10.0));
//     tms = t % 10000;
//     t /= 10000;
//     s = t % 60;
//     t /= 60;
//     m = t % 60;
//     t /= 60;
//     h = t;
//     QString hms;
//     hms.sprintf("%02u:%02u:%02u.%04u", h, m, s, tms);
//     QString txt = QString("%1\n%2\n%3").arg(text).arg(pos).arg(hms);
//
//     switch (selectionPosition(mouse.x()) & ~Selection) {
// 	case LeftBorder:
// 	    m_position_widget.setText(txt, Qt::AlignRight);
// 	    x = samples2pixels(pos - m_offset) - m_position_widget.width();
// 	    if (x < 0) {
// 		// switch to left aligned mode
// 		m_position_widget.setText(txt, Qt::AlignLeft);
// 		x = samples2pixels(pos - m_offset);
// 	    }
// 	    break;
// 	case RightBorder:
// 	default:
// 	    m_position_widget.setText(txt, Qt::AlignLeft);
// 	    x = samples2pixels(pos - m_offset);
// 	    if (x + m_position_widget.width() > width()) {
// 		// switch to right aligned mode
// 		m_position_widget.setText(txt, Qt::AlignRight);
// 		x = samples2pixels(pos - m_offset) - m_position_widget.width();
// 	    }
// 	    break;
//     }
//
//     // adjust the position to avoid vertical clipping
//     int lh = m_position_widget.height();
//     if (y - lh/2 < 0) {
// 	y = 0;
//     } else if (y + lh/2 > height()) {
// 	y = height() - lh;
//     } else {
// 	y -= lh/2;
//     }
//
//     m_position_widget.move(x, y);
//
//     if (!m_position_widget.isVisible())
// 	m_position_widget.show();
//
//     m_position_widget_timer.stop();
//     m_position_widget_timer.setSingleShot(true);
//     m_position_widget_timer.start(POSITION_WIDGET_TIME);
//
//     setUpdatesEnabled(true);
// }

// //***************************************************************************
// void SignalWidget::wheelEvent(QWheelEvent *event)
// {
//     if (!event) return;
//
//     // we currently are only interested in <Alt> + <WheelUp/Down>
//     if (event->modifiers() != Qt::AltModifier) {
// 	event->ignore();
// 	return;
//     }
//
//     if ((event->delta() > 0) && (m_vertical_zoom < VERTICAL_ZOOM_MAX)) {
// 	// zom in
// 	m_vertical_zoom *= VERTICAL_ZOOM_STEP_FACTOR;
// 	if (m_vertical_zoom > VERTICAL_ZOOM_MAX)
// 	    m_vertical_zoom = VERTICAL_ZOOM_MAX;
// 	event->accept();
// 	refreshSignalLayer();
//     } else if ((event->delta() < 0) && (m_vertical_zoom > VERTICAL_ZOOM_MIN)) {
// 	// zoom out
// 	m_vertical_zoom /= VERTICAL_ZOOM_STEP_FACTOR;
// 	if (m_vertical_zoom < VERTICAL_ZOOM_MIN)
// 	    m_vertical_zoom = VERTICAL_ZOOM_MIN;
// 	event->accept();
// 	refreshSignalLayer();
//     } else {
// 	// no change
// 	event->ignore();
//     }
// }

// //***************************************************************************
// // Label SignalWidget::findLabelNearMouse(int x) const
// // {
// //     const int tol = SELECTION_TOLERANCE;
// //     unsigned int pos = m_offset + pixels2samples(x);
// //     Label nearest;
// //     unsigned int dmin = pixels2samples(SELECTION_TOLERANCE) + 1;
// //
// //     foreach (Label label, labels()) {
// // 	unsigned int lp = label.pos();
// // 	if (lp < m_offset) continue; // outside left
// // 	int lx = samples2pixels(lp - m_offset);
// // 	if (lx >= m_width) continue; // outside right
// //
// // 	if ((lx + tol < x) || (lx > x + tol))
// // 	    continue; // out of tolerance
// //
// // 	unsigned int dist = (pos > lp) ? (pos - lp) : (lp - pos);
// // 	if (dist < dmin) {
// // 	    // found a new "nearest" label
// // 	    dmin = dist;
// // 	    nearest = label;
// // 	}
// //     }
// //
// //     return nearest;
// // }

// //***************************************************************************
// // LabelList &SignalWidget::labels()
// // {
// //     return m_context.signalManager()->fileInfo().labels();
// // }

// //***************************************************************************
// // const LabelList &SignalWidget::labels() const
// // {
// //     return m_context.signalManager()->fileInfo().labels();
// // }

//***************************************************************************
void SignalWidget::addLabel(unsigned int pos)
{
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return;

    UndoTransactionGuard undo(*signal_manager, i18n("Add Label"));

    // add a new label, with undo
    if (!signal_manager->addLabel(pos)) {
	signal_manager->abortUndoTransaction();
	return;
    }

    // find the new label
    Label label = signal_manager->findLabel(pos);
    if (label.isNull()) return;

    // edit the properties of the new label
    if (!labelProperties(label)) {
	// aborted or failed -> delete (without undo)
	int index = signal_manager->labelIndex(label);
	signal_manager->deleteLabel(index, false);
	signal_manager->abortUndoTransaction();
    }

//     refreshMarkersLayer(); ### TODO ###
//     hidePosition(); ### TODO ###
}

// ////****************************************************************************
// //void SignalWidget::loadLabel()
// //{
// //    labels().clear();    //remove old Labels...
// //    appendLabel ();
// //}

// ////****************************************************************************
// //void SignalWidget::saveLabel (const char *typestring)
// //{
// //    QString name = KFileDialog::getSaveFileName (0, "*.label", this);
// //    if (!name.isNull()) {
// //	FILE *out;
// //	out = fopen (name.local8Bit(), "w");
// //
// //	Parser parser (typestring);
// //	Label *tmp;
// //	LabelType *act;
// //
// //	const char *actstring = parser.getFirstParam();
// //
// //	while (actstring) {
// //	    printf ("selecting %s\n", actstring);
// //	    for (act = globals.markertypes.first(); act; act = globals.markertypes.next())
// //		if (strcmp(act->name, actstring) == 0) {
// //		    printf ("selected\n");
// //		    act->selected = true;
// //		    break;
// //		}
// //	    actstring = parser.getNextParam();
// //	}
// //
// //	for (act = globals.markertypes.first(); act; act = globals.markertypes.next())
// //	    //write out all selected label types
// //	    if (act->selected)
// //		fprintf (out, "%s\n", act->getCommand());
// //
// //	//ended writing of types, so go on with the labels...
// //
// //	for (tmp = labels().first(); tmp; tmp = labels().next())  //write out labels
// //	{
// //	    fprintf (out, tmp->getCommand());
// //	    fprintf (out, "\n");
// //	}
// //
// //	fclose (out);
// //    }
// //}

//***************************************************************************
bool SignalWidget::labelProperties(Label &label)
{
    SignalManager *signal_manager = m_context.signalManager();
    Q_ASSERT(signal_manager);
    if (!signal_manager) return false;

    if (label.isNull()) return false;
    int index = signal_manager->labelIndex(label);
    Q_ASSERT(index >= 0);
    if (index < 0) return false;

    // try to modify the label. just in case that the user moves it
    // to a position where we already have one, catch this situation
    // and ask if he wants to abort, re-enter the label properties
    // dialog or just replace (remove) the label at the target position
    bool accepted;
    unsigned int new_pos  = label.pos();
    QString      new_name = label.name();
    int old_index = -1;
    while (true) {
	// create and prepare the dialog
	LabelPropertiesWidget *dlg = new LabelPropertiesWidget(this);
	Q_ASSERT(dlg);
	if (!dlg) return false;
	dlg->setLabelIndex(index);
	dlg->setLabelPosition(new_pos, signal_manager->length(),
	    signal_manager->rate());
	dlg->setLabelName(new_name);

	// execute the dialog
	accepted = (dlg->exec() == QDialog::Accepted);
	if (!accepted) {
	    // user pressed "cancel"
	    delete dlg;
	    break;
	}

	// if we get here the user pressed "OK"
	new_pos  = dlg->labelPosition();
	new_name = dlg->labelName();
	dlg->saveSettings();
	delete dlg;

	// check: if there already is a label at the new position
	// -> ask the user if he wants to overwrite that one
	if ((new_pos != label.pos()) &&
	    !signal_manager->findLabel(new_pos).isNull())
	{
	    int res = Kwave::MessageBox::warningYesNoCancel(this, i18n(
		"There already is a label at the position "\
		"you have chosen.\n"\
		"Do you want to replace it?"));
	    if (res == KMessageBox::Yes) {
		// delete the label at the target position (with undo)
		Label old = signal_manager->findLabel(new_pos);
		old_index = signal_manager->labelIndex(old);
		break;
	    }
	    if (res == KMessageBox::No) {
		// make another try -> re-enter the dialog
		continue;
	    }

	    // cancel -> abort the whole action
	    accepted = false;
	    break;
	} else {
	    // ok, we can put it there
	    break;
	}
    }

    if (accepted) {
	// shortcut: abort if nothing has changed
	if ((new_name == label.name()) && (new_pos == label.pos()))
	    return false;

	UndoTransactionGuard undo(*signal_manager, i18n("Modify Label"));

	// if there is a label at the target position, remove it first
	if (old_index >= 0) {
	    signal_manager->deleteLabel(old_index, true);
	    // this might have changed the current index!
	    index = signal_manager->labelIndex(label);
	}

	// modify the label through the signal manager
	if (!signal_manager->modifyLabel(index, new_pos, new_name)) {
	    // position is already occupied
	    signal_manager->abortUndoTransaction();
	    return false;
	}

	// reflect the change in the passed label
	label.moveTo(new_pos);
	label.rename(new_name);

	// NOTE: moving might also change the index, so the complete
	//       markers layer has to be refreshed
// 	refreshMarkersLayer(); ### TODO ###
    }
    else
	signal_manager->abortUndoTransaction();

    return accepted;
}

// ////****************************************************************************
// //void SignalWidget::jumptoLabel ()
// // another fine function contributed by Gerhard Zintel
// // if lmarker == rmarker (no range selected) cursor jumps to the nearest label
// // if lmarker <  rmarker (range is selected) lmarker jumps to next lower label or zero
// // rmarker jumps to next higher label or end
// //{
// //    if (signalmanage) {
// //	int lmarker = signalmanage->getLMarker();
// //	int rmarker = signalmanage->getRMarker();
// //	bool RangeSelected = (rmarker - lmarker) > 0;
// //	if (!labels().isEmpty()) {
// //	    Label *tmp;
// //	    int position = 0;
// //	    for (tmp = labels->first(); tmp; tmp = labels->next())
// //		if (RangeSelected) {
// //		    if (tmp->pos < lmarker)
// //			if (qAbs(lmarker - position) >
// //			    qAbs(lmarker - ms2samples(tmp->pos)))
// //			    position = ms2samples(tmp->pos);
// //		} else if (qAbs(lmarker - position) >
// //			   qAbs(lmarker - ms2samples(tmp->pos)))
// //		    position = ms2samples(tmp->pos);
// //
// //	    lmarker = position;
// //	    position = signalmanage->getLength();
// //	    for (tmp = labels->first(); tmp; tmp = labels->next())
// //		if (tmp->pos > rmarker)
// //		    if (qAbs(rmarker - position) >
// //			qAbs(rmarker - ms2samples(tmp->pos)))
// //			position = ms2samples(tmp->pos);
// //	    rmarker = position;
// //	    if (RangeSelected) setRange(lmarker, rmarker);
// //	    else setRange (lmarker, lmarker);
// //	    refresh ();
// //	}
// //    }
// //}

// ////****************************************************************************
// //void SignalWidget::savePeriods ()
// //{
// //    if (signalmanage) {
// //	Dialog *dialog =
// //	    DynamicLoader::getDialog ("marksave", new DialogOperation(&globals, signalmanage->getRate(), 0, 0));
// //
// //	if ((dialog) && (dialog->exec())) {
// //	    selectMarkers (dialog->getCommand());
// //
// //	    LabelType *act;
// //	    Label *tmp;
// //	    int last = 0;
// //	    int rate = signalmanage->getRate ();
// //
// //	    QString name = KFileDialog::getSaveFileName (0, "*.dat", this);
// //	    if (!name.isNull()) {
// //		QFile out(name.local8Bit());
// //		char buf[160];
// //		float freq = 0, time, lastfreq = 0;
// //		out.open (QIODevice::WriteOnly);
// //		int first = true;
// //
// //		for (act = globals.markertypes.first(); act; act = globals.markertypes.next())
// //		    //write only selected label type
// //		    if (act->selected)
// //			//traverse list of all labels
// //			for (tmp = labels->first(); tmp; tmp = labels->next()) {
// //			    if (tmp->getType() == act) {
// //				freq = tmp->pos - last;
// //				time = last * 1000 / rate;
// //
// //				if ((!first) && (freq != lastfreq)) {
// //				    lastfreq = freq;
// //				    freq = 1 / (freq / rate);
// //				    snprintf (buf, sizeof(buf), "%f %f\n",
// //					time, freq);
// //				    out.writeBlock (&buf[0], strlen(buf));
// //				} else lastfreq = freq;
// //				first = false;
// //				last = ms2samples(tmp->pos);
// //			    }
// //			}
// //
// //		if (!first) //make sure last tone gets its length
// //		{
// //		    time = last * 1000 / rate;
// //		    snprintf (buf, sizeof(buf), "%f %f\n", time, freq);
// //		    out.writeBlock (&buf[0], strlen(buf));
// //		}
// //
// //		out.close ();
// //	    }
// //	}
// //    }
// //}

// ////****************************************************************************
// //void SignalWidget::markSignal (const char *str)
// //{
// //    if (signalmanage) {
// //	Label *newmark;
// //
// //	Parser parser (str);
// //
// //	int level = (int) (parser.toDouble() / 100 * (1 << 23));
// //
// //	int len = signalmanage->getLength();
// //	int *sam = signalmanage->getSignal(0)->getSample();    // ### @@@ ###
// //	LabelType *start = findMarkerType(parser.getNextParam());
// //	LabelType *stop = findMarkerType (parser.getNextParam());
// //	int time = (int) (parser.toDouble () * signalmanage->getRate() / 1000);
// //
// //	printf ("%d %d\n", level, time);
// //	printf ("%s %s\n", start->name, stop->name);
// //
// //	ProgressDialog *dialog =
// //	    new ProgressDialog (len, "Searching for Signal portions...");
// //
// //	if (dialog && start && stop) {
// //	    dialog->show();
// //
// //	    newmark = new Label(0, start);     //generate initial Label
// //
// //	    labels->inSort (newmark);
// //
// //	    for (int i = 0; i < len; i++) {
// //		if (qAbs(sam[i]) < level) {
// //		    int j = i;
// //		    while ((i < len) && (qAbs(sam[i]) < level)) i++;
// //
// //		    if (i - j > time) {
// //			//insert labels...
// //			newmark = new Label(i, start);
// //			labels->inSort (newmark);
// //
// //			if (start != stop) {
// //			    newmark = new Label(j, stop);
// //			    labels->inSort (newmark);
// //			}
// //		    }
// //		}
// //		dialog->setProgress (i);
// //	    }
// //
// //	    newmark = new Label(len - 1, stop);
// //	    labels->inSort (newmark);
// //
// //	    refresh ();
// //	    delete dialog;
// //	}
// //    }
// //}

// ////****************************************************************************
// //void SignalWidget::markPeriods (const char *str)
// //{
// //    if (signalmanage) {
// //	Parser parser (str);
// //
// //	int high = signalmanage->getRate() / parser.toInt();
// //	int low = signalmanage->getRate() / parser.toInt();
// //	int octave = parser.toBool ("true");
// //	double adjust = parser.toDouble ();
// //
// //	for (int i = 0; i < AUTOKORRWIN; i++)
// //	    autotable[i] = 1 - (((double)i * i * i) / (AUTOKORRWIN * AUTOKORRWIN * AUTOKORRWIN));    //generate static weighting function
// //
// //	if (octave) for (int i = 0; i < AUTOKORRWIN; i++) weighttable[i] = 1;    //initialise moving weight table
// //
// //	Label *newmark;
// //	int next;
// //	int len = signalmanage->getLength();
// //	int *sam = signalmanage->getSignal(0)->getSample();    // ### @@@ ###
// //	LabelType *start = markertype;
// //	int cnt = findFirstMark (sam, len);
// //
// //	ProgressDialog *dialog = new ProgressDialog (len - AUTOKORRWIN, "Correlating Signal to find Periods:");
// //	if (dialog) {
// //	    dialog->show();
// //
// //	    newmark = new Label(cnt, start);
// //	    labels->inSort (newmark);
// //
// //	    while (cnt < len - 2*AUTOKORRWIN) {
// //		if (octave)
// //		    next = findNextRepeatOctave (&sam[cnt], high, adjust);
// //		else
// //		    next = findNextRepeat (&sam[cnt], high);
// //
// //		if ((next < low) && (next > high)) {
// //		    newmark = new Label(cnt, start);
// //
// //		    labels->inSort (newmark);
// //		}
// //		if (next < AUTOKORRWIN) cnt += next;
// //		else
// //		    if (cnt < len - AUTOKORRWIN) {
// //			int a = findFirstMark (&sam[cnt], len - cnt);
// //			if (a > 0) cnt += a;
// //			else cnt += high;
// //		    } else cnt = len;
// //
// //		dialog->setProgress (cnt);
// //	    }
// //
// //	    delete dialog;
// //
// //	    refresh ();
// //	}
// //    }
// //}

// ////*****************************************************************************
// //int findNextRepeat (int *sample, int high)
// ////autocorellation of a windowed part of the sample
// ////returns length of period, if found
// //{
// //    int i, j;
// //    double gmax = 0, max, c;
// //    int maxpos = AUTOKORRWIN;
// //    int down, up;         //flags
// //
// //    max = 0;
// //    for (j = 0; j < AUTOKORRWIN; j++)
// //	gmax += ((double)sample[j]) * sample [j];
// //
// //    //correlate signal with itself for finding maximum integral
// //
// //    down = 0;
// //    up = 0;
// //    i = high;
// //    max = 0;
// //    while (i < AUTOKORRWIN) {
// //	c = 0;
// //	for (j = 0; j < AUTOKORRWIN; j++) c += ((double)sample[j]) * sample [i + j];
// //	c = c * autotable[i];    //multiply window with weight for preference of high frequencies
// //	if (c > max) max = c, maxpos = i;
// //	i++;
// //    }
// //    return maxpos;
// //}

// ////*****************************************************************************
// //int findNextRepeatOctave (int *sample, int high, double adjust = 1.005)
// ////autocorellation of a windowed part of the sample
// ////same as above only with an adaptive weighting to decrease fast period changes
// //{
// //    int i, j;
// //    double gmax = 0, max, c;
// //    int maxpos = AUTOKORRWIN;
// //    int down, up;         //flags
// //
// //    max = 0;
// //    for (j = 0; j < AUTOKORRWIN; j++)
// //	gmax += ((double)sample[j]) * sample [j];
// //
// //    //correlate signal with itself for finding maximum integral
// //
// //    down = 0;
// //    up = 0;
// //    i = high;
// //    max = 0;
// //    while (i < AUTOKORRWIN) {
// //	c = 0;
// //	for (j = 0; j < AUTOKORRWIN; j++) c += ((double)sample[j]) * sample [i + j];
// //	c = c * autotable[i] * weighttable[i];
// //	//multiply window with weight for preference of high frequencies
// //	if (c > max) max = c, maxpos = i;
// //	i++;
// //    }
// //
// //    for (int i = 0; i < AUTOKORRWIN; i++) weighttable[i] /= adjust;
// //
// //    weighttable[maxpos] = 1;
// //    weighttable[maxpos + 1] = .9;
// //    weighttable[maxpos - 1] = .9;
// //    weighttable[maxpos + 2] = .8;
// //    weighttable[maxpos - 2] = .8;
// //
// //    float buf[7];
// //
// //    for (int i = 0; i < 7; buf[i++] = .1)
// //
// //	//low pass filter
// //	for (int i = high; i < AUTOKORRWIN - 3; i++) {
// //	    buf[i % 7] = weighttable[i + 3];
// //	    weighttable[i] = (buf[0] + buf[1] + buf[2] + buf[3] + buf[4] + buf[5] + buf[6]) / 7;
// //	}
// //
// //    return maxpos;
// //}

// //*****************************************************************************
// //int findFirstMark (int *sample, int len)
// ////finds first sample that is non-zero, or one that preceeds a zero crossing
// //{
// //    int i = 1;
// //    int last = sample[0];
// //    int act = last;
// //    if ((last < 100) && (last > -100)) i = 0;
// //    else
// //	while (i < len) {
// //	    act = sample[i];
// //	    if ((act < 0) && (last >= 0)) break;
// //	    if ((act > 0) && (last <= 0)) break;
// //	    last = act;
// //	    i++;
// //	}
// //    return i;
// //}

// //***************************************************************************
// void SignalWidget::playbackStopped()
// {
//     InhibitRepaintGuard inhibit(*this);
// }

// //***************************************************************************
// void SignalWidget::updatePlaybackPointer(unsigned int)
// {
//     InhibitRepaintGuard inhibit(*this);
// }

//***************************************************************************
int SignalWidget::viewPortWidth()
{
    if (m_views.isEmpty()) return width(); // if empty
    return m_layout.cellRect(0, 1).width();
}

//***************************************************************************
void SignalWidget::insertView(Kwave::SignalView *view, QWidget *controls)
{
    Q_ASSERT(m_upper_dock);
    Q_ASSERT(m_lower_dock);
    Q_ASSERT(view);
    if (!m_upper_dock || !m_lower_dock) return;
    if (!view) return;

    // find the proper row to insert the track view
    int index = 0;
    int track = (view) ? view->track() : -1;
    const Kwave::SignalView::Location where = view->preferredLocation();
    switch (where) {
	case Kwave::SignalView::UpperDockTop: {
	    // upper dock area, top
	    index = 0;
	    m_upper_dock->insertWidget(0, view);
	    Q_ASSERT(!controls);
	    break;
	}
	case Kwave::SignalView::UpperDockBottom: {
	    // upper dock area, bottom
	    index = m_upper_dock->count();
	    m_upper_dock->addWidget(view);
	    Q_ASSERT(!controls);
	    break;
	}
	case Kwave::SignalView::Top: {
	    // central layout, above all others
	    index = m_upper_dock->count();
	    int row = 0;
	    m_layout.addWidget(view, row, 1);
	    if (controls) m_layout.addWidget(controls, row, 0);
	    break;
	}
	case Kwave::SignalView::AboveTrackTop: {
	    // above the corresponding track, start of group
	    index = m_upper_dock->count();
	    int row = 0;
	    for (;index < m_views.count(); ++row, ++index) {
		if (m_views[index]->track() >= track) break; // reached top
		if (m_views[index]->preferredLocation() >=
		        Kwave::SignalView::Bottom) break;
	    }
	    m_layout.addWidget(view, row, 1);
	    if (controls) m_layout.addWidget(controls, row, 0);
	    break;
	}
	case Kwave::SignalView::AboveTrackBottom: {
	    // above the corresponding track, end of group
	    index = m_upper_dock->count();
	    int row = 0;
	    for (;index < m_views.count(); ++row, ++index) {
		if (m_views[index]->track() < track) continue; // too early
		if (m_views[index]->track() != track) break; // next track
		if (m_views[index]->preferredLocation() !=
		        Kwave::SignalView::AboveTrackTop) break;
	    }
	    m_layout.addWidget(view, row, 1);
	    if (controls) m_layout.addWidget(controls, row, 0);
	    break;
	}
	case Kwave::SignalView::BelowTrackTop: {
	    // below the corresponding track, start of group
	    index = m_upper_dock->count();
	    int row = 0;
	    for (;index < m_views.count(); ++row, ++index) {
		if (m_views[index]->track() < track) continue; // too early
		if (m_views[index]->track() != track) break; // next track
		if (m_views[index]->preferredLocation() >=
		        Kwave::SignalView::BelowTrackTop) break;
	    }
	    m_layout.addWidget(view, row, 1);
	    if (controls) m_layout.addWidget(controls, row, 0);
	    break;
	}
	case Kwave::SignalView::BelowTrackBottom: {
	    // below the corresponding track, end of group
	    index = m_upper_dock->count();
	    int row = 0;
	    for (;index < m_views.count(); ++row, ++index) {
		if (m_views[index]->track() < track) continue; // too early
		if (m_views[index]->track() != track) break; // next track
		if (m_views[index]->preferredLocation() >=
		        Kwave::SignalView::Bottom) break;
	    }
	    m_layout.addWidget(view, row, 1);
	    if (controls) m_layout.addWidget(controls, row, 0);
	    break;
	}
	case Kwave::SignalView::Bottom: {
	    // below all others
	    int row = m_layout.rowCount();
	    index = m_upper_dock->count() + row;
	    m_layout.addWidget(view, row, 1);
	    if (controls) m_layout.addWidget(controls, row, 0);
	    break;
	}
	case Kwave::SignalView::LowerDockTop: {
	    // lower dock area, top
	    index = m_upper_dock->count() + m_layout.rowCount();
	    m_lower_dock->insertWidget(0, view);
	    Q_ASSERT(!controls);
	    break;
	}
	case Kwave::SignalView::LowerDockBottom:
	    // lower dock area, bottom
	    index = m_upper_dock->count() + m_layout.rowCount() +
	            m_lower_dock->count();
	    m_lower_dock->addWidget(view);
	    Q_ASSERT(!controls);
	    break;
    }

    // insert the view into the list of views
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < m_upper_dock->count() + m_layout.rowCount() +
             m_lower_dock->count());
    m_views.insert(index, view);
    qDebug("SignalWidget::insertView(...), view count = %d", m_views.count());

    // initially set the current view info
    view->setZoomAndOffset(m_zoom, m_offset);

    // connect all signals

//     connect(pix, SIGNAL(sigModified()),
//             this, SLOT(refreshSignalLayer()));

}

//***************************************************************************
void SignalWidget::slotTrackInserted(unsigned int index, Track *track)
{
    Q_ASSERT(track);
    if (!track) return;

    // create a container widget for the track controls
    QWidget *controls = new QWidget(this);
    Q_ASSERT(controls);
    if (!controls) return;

    // create a new view for the track's signal
    Kwave::SignalView *view = new Kwave::TrackView(
	this, controls, m_context.signalManager(), track);
    Q_ASSERT(view);
    if (!view) {
	if (controls) delete controls;
	return;
    }

    // loop over all views and adjust the track index of the following ones
    foreach (QPointer<Kwave::SignalView> view, m_views) {
	if (view->track() > static_cast<int>(index))
	    view->setTrack(view->track() + 1);
    }

    // assign the view to the new track
    view->setTrack(index);

// ### TODO ###
//     connect(track, SIGNAL(sigSelectionChanged()),
// 	    this, SIGNAL(sigTrackSelectionChanged()));
//     connect(track, SIGNAL(sigSelectionChanged()),
//             this, SLOT(refreshSignalLayer()));

    connect(view, SIGNAL(destroyed()), controls, SLOT(deleteLater()));

    insertView(view, controls);
}

//***************************************************************************
void SignalWidget::slotTrackDeleted(unsigned int index)
{
    // loop over all views, delete those that are bound to this track
    // and adjust the index of the following ones
    bool empty = true;
    foreach (QPointer<Kwave::SignalView> view, m_views) {
	if (view->track() == static_cast<int>(index)) {
	    m_views.removeAll(view);
	    delete view;
	} else if (view->track() > static_cast<int>(index)) {
	    view->setTrack(view->track() - 1);
	    empty = false;
	} else if (view->track() != -1)
	    empty = false;
    }

    // if there are only views with track() == -1, we are empty,
    // in that case delete the rest (all views)
    if (empty) {
	while (!m_views.isEmpty())
	    delete m_views.takeFirst();
    }
}

// //***************************************************************************
// void SignalWidget::slotSamplesInserted(unsigned int track,
//     unsigned int offset, unsigned int length)
// {
// //     qDebug("SignalWidget(): slotSamplesInserted(%u, %u,%u)", track,
// // 	offset, length);
//     Q_UNUSED(track);
//     Q_UNUSED(offset);
//     Q_UNUSED(length);
//
//     SignalManager *signal_manager = m_context.signalManager();
//     Q_ASSERT(signal_manager);
//     if (!signal_manager) return;
//
//     // length has changed -> update overview
//     emit viewInfo(m_offset, pixels2samples(QWidget::width()-1)+1,
//                   signal_manager->length());
//
//     refreshMarkersLayer();
// }

// //***************************************************************************
// void SignalWidget::slotSamplesDeleted(unsigned int track,
//     unsigned int offset, unsigned int length)
// {
// //    qDebug("SignalWidget(): slotSamplesDeleted(%u, %u...%u)", track,
// // 	offset, offset+length-1);
//     Q_UNUSED(track);
//     Q_UNUSED(offset);
//     Q_UNUSED(length);
//
//     SignalManager *signal_manager = m_context.signalManager();
//     Q_ASSERT(signal_manager);
//     if (!signal_manager) return;
//
//     // length has changed -> update overview
//     emit viewInfo(m_offset, pixels2samples(QWidget::width()-1)+1,
//                   signal_manager->length());
//
//     refreshMarkersLayer();
// }

// //***************************************************************************
// void SignalWidget::slotSamplesModified(unsigned int /*track*/,
//     unsigned int /*offset*/, unsigned int /*length*/)
// {
// //    qDebug("SignalWidget(): slotSamplesModified(%u, %u,%u)", track,
// //	offset, length);
// }

// //***************************************************************************
// void SignalWidget::startDragging()
// {
//     SignalManager *signal_manager = m_context.signalManager();
//     Q_ASSERT(signal_manager);
//     if (!signal_manager) return;
//
//     const unsigned int length = signal_manager->selection().length();
//     if (!length) return;
//
//     KwaveDrag *d = new KwaveDrag(this);
//     Q_ASSERT(d);
//     if (!d) return;
//
//     const unsigned int first = signal_manager->selection().first();
//     const unsigned int last  = signal_manager->selection().last();
//     const double       rate  = signal_manager->rate();
//     const unsigned int bits  = signal_manager->bits();
//
//     MultiTrackReader src(Kwave::SinglePassForward, *signal_manager,
// 	signal_manager->selectedTracks(), first, last);
//
//     // create the file info
//     FileInfo info = signal_manager->fileInfo();
//     info.setLength(last - first + 1);
//     info.setRate(rate);
//     info.setBits(bits);
//     info.setTracks(src.tracks());
//
//     if (!d->encode(this, src, info)) {
// 	delete d;
// 	return;
//     }
//
//     // start drag&drop, mode is determined automatically
//     InhibitRepaintGuard inhibit(*this);
//     UndoTransactionGuard undo(*signal_manager, i18n("Drag and Drop"));
//     Qt::DropAction drop = d->exec(Qt::CopyAction | Qt::MoveAction);
//
//     if (drop == Qt::MoveAction) {
// 	// deleting also affects the selection !
// 	const unsigned int f = signal_manager->selection().first();
// 	const unsigned int l = signal_manager->selection().last();
// 	const unsigned int len = l-f+1;
//
// 	// special case: when dropping into the same widget, before
// 	// the previous selection, the previous range has already
// 	// been moved to the right !
// 	unsigned int src = first;
// 	if ((d->target() == this) && (f < src)) src += len;
//
// 	signal_manager->deleteRange(src, len, signal_manager->selectedTracks());
//
// 	// restore the new selection
// 	selectRange((first < f) ? f-len : f, len);
//     }
// }

// //***************************************************************************
// void SignalWidget::dragEnterEvent(QDragEnterEvent *event)
// {
//     if (!event) return;
//     if ((event->proposedAction() != Qt::MoveAction) &&
//         (event->proposedAction() != Qt::CopyAction))
//         return; /* unsupported action */
//
//     if (KwaveFileDrag::canDecode(event->mimeData()))
// 	event->acceptProposedAction();
// }

// //***************************************************************************
// void SignalWidget::dragLeaveEvent(QDragLeaveEvent *)
// {
//     setMouseMode(MouseNormal);
// }

// //***************************************************************************
// void SignalWidget::dropEvent(QDropEvent *event)
// {
//     if (!event) return;
//     if (!event->mimeData()) return;
//
//     SignalManager *signal_manager = m_context.signalManager();
//     Q_ASSERT(signal_manager);
//     if (!signal_manager) return;
//
//     if (KwaveDrag::canDecode(event->mimeData())) {
// 	UndoTransactionGuard undo(*signal_manager, i18n("Drag and Drop"));
// 	InhibitRepaintGuard inhibit(*this);
// 	unsigned int pos = m_offset + pixels2samples(event->pos().x());
// 	unsigned int len = 0;
//
// 	if ((len = KwaveDrag::decode(this, event->mimeData(),
// 	    *signal_manager, pos)))
// 	{
// 	    // set selection to the new area where the drop was done
// 	    selectRange(pos, len);
// 	    event->acceptProposedAction();
// 	} else {
// 	    qWarning("SignalWidget::dropEvent(%s): failed !", event->format(0));
// 	    event->ignore();
// 	}
//     } else if (event->mimeData()->hasUrls()) {
// 	bool first = true;
// 	foreach (QUrl url, event->mimeData()->urls()) {
// 	    QString filename = url.toLocalFile();
// 	    QString mimetype = CodecManager::whatContains(filename);
// 	    if (CodecManager::canDecode(mimetype)) {
// 		if (first) {
// 		    // first dropped URL -> open in this window
// 		    emit sigCommand("open(" + filename + ")");
// 		    first = false;
// 		} else {
// 		    // all others -> open a new window
// 		    emit sigCommand("newwindow(" + filename + ")");
// 		}
// 	    }
// 	}
//     }
//
//     qDebug("SignalWidget::dropEvent(): done");
//     setMouseMode(MouseNormal);
// }

// //***************************************************************************
// void SignalWidget::dragMoveEvent(QDragMoveEvent* event)
// {
//     if (!event) return;
//
//     const int x = event->pos().x();
//
//     if ((event->source() == this) && isInSelection(x)) {
// 	// disable drag&drop into the selection itself
// 	// this would be nonsense
//
// 	SignalManager *signal_manager = m_context.signalManager();
// 	Q_ASSERT(signal_manager);
// 	if (!signal_manager) {
// 	    event->ignore();
// 	    return;
// 	}
//
// 	unsigned int left  = signal_manager->selection().first();
// 	unsigned int right = signal_manager->selection().last();
// 	const unsigned int w = pixels2samples(m_width);
// 	QRect r(this->rect());
//
// 	// crop selection to widget borders
// 	if (left < m_offset) left = m_offset;
// 	if (right > m_offset+w) right = m_offset+w-1;
//
// 	// transform to pixel coordinates
// 	left  = samples2pixels(left - m_offset);
// 	right = samples2pixels(right - m_offset);
// 	if (right >= static_cast<unsigned int>(m_width))
// 	    right=m_width-1;
// 	if (left > right)
// 	    left = right;
//
// 	r.setLeft(left);
// 	r.setRight(right);
// 	event->ignore(r);
//     } else if (KwaveDrag::canDecode(event->mimeData())) {
// 	// accept if it is decodeable within the
// 	// current range (if it's outside our own selection)
// 	event->acceptProposedAction();
//     } else if (KwaveFileDrag::canDecode(event->mimeData())) {
// 	// file drag
// 	event->accept();
//     } else event->ignore();
// }

//***************************************************************************
// void SignalWidget::parseKey(int key)
// {
//     if ((key < 0) || (key >= m_lamps.count()))
// 	return;
//     if (m_lamps.at(key)) m_lamps.at(key)->nextState();
// }

//***************************************************************************
#include "SignalWidget.moc"
//***************************************************************************
//***************************************************************************
