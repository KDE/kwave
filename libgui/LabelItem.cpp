/***************************************************************************
           LabelItem.cpp -  label item within a SignalView
                             -------------------
    begin                : Sat Mar 26 2011
    copyright            : (C) 2011 by Thomas Eschenbacher
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

#include <QAction>
#include <QIcon>
#include <QMenu>

#include <KLocalizedString>

#include "libkwave/Label.h"
#include "libkwave/SignalManager.h"
#include "libkwave/String.h"
#include "libkwave/Utils.h"
#include "libkwave/undo/UndoModifyMetaDataAction.h"
#include "libkwave/undo/UndoTransactionGuard.h"

#include "libgui/LabelItem.h"
#include "libgui/SignalView.h"

//***************************************************************************
Kwave::LabelItem::LabelItem(Kwave::SignalView &view,
                            Kwave::SignalManager &signal_manager,
                            unsigned int index,
                            const Kwave::Label &label)
    :Kwave::ViewItem(view, signal_manager),
     m_index(index),
     m_initial_pos(label.pos()),
     m_current_pos(label.pos()),
     m_description(label.name()),
     m_undo_transaction(Q_NULLPTR)
{
}

//***************************************************************************
Kwave::LabelItem::~LabelItem()
{
    if (m_undo_transaction) {
        // restore the previous data
        qDebug("Kwave::LabelItem::~LabelItem() -> aborted -> reverting!");
        m_undo_transaction->abort();
        delete m_undo_transaction;
        m_undo_transaction = Q_NULLPTR;
    }
}

//***************************************************************************
Kwave::ViewItem::Flags Kwave::LabelItem::flags() const
{
    return Kwave::ViewItem::CanGrabAndMove;
}

//***************************************************************************
QString Kwave::LabelItem::toolTip(sample_index_t &ofs)
{
    Q_UNUSED(ofs)

    QString description = (m_description.length()) ?
        i18nc("tooltip of a label, %1=index, %2=description/name",
                "Label #%1 (%2)", m_index, m_description) :
        i18nc("tooltip of a label, without description, %1=index",
                "Label #%1", m_index);

    QString hms = Kwave::ms2hms(m_view.samples2ms(m_current_pos));
    QString tip = _("%1\n%2\n%3").arg(description).arg(m_current_pos).arg(hms);

    return tip;
}

//***************************************************************************
void Kwave::LabelItem::appendContextMenu(QMenu *parent)
{
    Q_ASSERT(parent);
    if (!parent) return;

    // locate the "label" menu
    QMenu *label_menu = Q_NULLPTR;
    foreach (const QAction *action, parent->actions()) {
        if (action->text() == i18n("Label")) {
            label_menu = action->menu();
            break;
        }
    }

    // the context menu of a label has been activated
    if (label_menu) {

        // find the "New" action and disable it
        foreach (QAction *action, label_menu->actions()) {
            if (action->text() == i18n("New")) {
                action->setEnabled(false);
                break;
            }
        }

        QAction *action_label_delete = label_menu->addAction(
            QIcon::fromTheme(_("list-remove")),
            i18n("&Delete"), this, SLOT(contextMenuLabelDelete()));
        Q_ASSERT(action_label_delete);
        if (!action_label_delete) return;

        QAction *action_label_properties = label_menu->addAction(
            QIcon::fromTheme(_("configure")),
            i18n("&Properties..."), this, SLOT(contextMenuLabelProperties()));
        Q_ASSERT(action_label_properties);
        if (!action_label_properties) return;
    }

}

//***************************************************************************
void Kwave::LabelItem::contextMenuLabelDelete()
{
    emit sigCommand(_("label:delete(%1)").arg(m_index));
}

//***************************************************************************
void Kwave::LabelItem::contextMenuLabelProperties()
{
    emit sigCommand(_("nomacro:label:edit(%1)").arg(m_index));
}

//***************************************************************************
QCursor Kwave::LabelItem::mouseCursor() const
{
    return Qt::SizeHorCursor;
}

//***************************************************************************
void Kwave::LabelItem::moveTo(const QPoint &mouse_pos)
{
    const sample_index_t new_pos = m_view.offset() +
                                   m_view.pixels2samples(mouse_pos.x());

    Kwave::Label label = m_signal_manager.findLabel(new_pos);
    if (label.isNull()) {

        // this is the first move ?
        if (!m_undo_transaction) {
            // there probably will be something to undo later
            // create an undo transaction guard
            m_undo_transaction = new(std::nothrow)
                Kwave::UndoTransactionGuard(
                    m_signal_manager, i18n("Move Label"));
            Q_ASSERT(m_undo_transaction);
            if (!m_undo_transaction) return;

            // save the previous label data for undo
            Kwave::Label lbl = m_signal_manager.findLabel(m_initial_pos);
            if (!m_undo_transaction->registerUndoAction(new(std::nothrow)
                UndoModifyMetaDataAction(Kwave::MetaDataList(lbl)))) {
                qWarning("Kwave::LabelItem::done(): saving undo data failed!");
                return;
            }
        }

        if (m_signal_manager.modifyLabel(m_index, new_pos,
                                         m_description, false)) {
            Kwave::Label lbl = m_signal_manager.findLabel(new_pos);
            if (!lbl.isNull()) {
                m_index       = m_signal_manager.labelIndex(lbl);
                m_current_pos = lbl.pos();
            }
        }
    }
}

//***************************************************************************
void Kwave::LabelItem::done()
{
    if (m_undo_transaction) {
        // close the undo transaction (regularly, with undo action)
        if (m_current_pos == m_initial_pos)
            m_undo_transaction->abort();
        delete m_undo_transaction;
        m_undo_transaction = Q_NULLPTR;
    }
}

//***************************************************************************
//***************************************************************************

#include "moc_LabelItem.cpp"
