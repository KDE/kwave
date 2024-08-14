/***************************************************************************
                Label.h  -  representation of a label within a signal
                             -------------------
    begin                : Mon Jul 31 2006
    copyright            : (C) 2006 by Thomas Eschenbacher
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
#ifndef LABEL_H
#define LABEL_H

#include "config.h"
#include "libkwave_export.h"

#include <QtGlobal>
#include <QString>

#include <KLocalizedString>

#include "libkwave/MetaData.h"
#include "libkwave/Sample.h"
#include "libkwave/String.h"

namespace Kwave
{

    class LIBKWAVE_EXPORT Label: public Kwave::MetaData
    {
    public:

        /** Default constructor, creates an empty label */
        Label();

        /**
         * Constructor, takes over the identity of a MetaData object
         * @param meta_data reference to a meta data object
         */
        explicit Label(const Kwave::MetaData &meta_data);

        /**
         * Constructor
         *
         * @param position the label position [samples]
         * @param name the name of the label, user defined
         */
        Label(sample_index_t position, const QString &name);

        /** destructor */
        virtual ~Label();

        /** returns the identifier of the "type" of this meta data object */
        static QString metaDataType() { return _("Label"); }

        /**
         * Set a new position of the label
         * @param position the new position [samples]
         */
        virtual void moveTo(sample_index_t position);

        /** Returns the label's position [samples] */
        virtual sample_index_t pos() const;

        /**
         * change the name of the label
         * @param name the new name, user defined
         */
        virtual void rename(const QString &name);

        /** returns the name of the string */
        virtual QString name() const;

        /** less-than operator, needed for sorting the list */
        inline bool operator < (const Kwave::Label &other) const {
            return (pos() < other.pos());
        }

        /** equal operator */
        inline bool operator == (const Kwave::Label &other) const {
            return ((pos() == other.pos()) && (name() == other.name()));
        }

    };
}

#endif /* LABEL_H */

//***************************************************************************
//***************************************************************************
