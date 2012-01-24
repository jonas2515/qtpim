/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#ifndef QDECLARATIVECONTACTDISPLAYLABEL_H
#define QDECLARATIVECONTACTDISPLAYLABEL_H

#include "qdeclarativecontactdetail_p.h"
#include "qcontactdisplaylabel.h"
#include <QDebug>

QTCONTACTS_BEGIN_NAMESPACE

class QDeclarativeContactDisplayLabel : public QDeclarativeContactDetail
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label NOTIFY valueChanged)
    Q_ENUMS(FieldType)
    Q_CLASSINFO("DefaultProperty", "label")
public:
    enum FieldType {
        Label = 0
    };
    QDeclarativeContactDisplayLabel(QObject* parent = 0)
        :QDeclarativeContactDetail(parent)
    {
        setDetail(QContactDisplayLabel());
        connect(this, SIGNAL(valueChanged()), SIGNAL(detailChanged()));
    }

    ContactDetailType detailType() const
    {
        return QDeclarativeContactDetail::DisplayLabel;
    }

    static QString fieldNameFromFieldType(int fieldType)
    {
        switch (fieldType) {
        case Label:
            return QContactDisplayLabel::FieldLabel;
        default:
            break;
        }
        qmlInfo(0) << tr("Unknown field type.");
        return QString();
    }

    QString label() const {
        return detail().value(QContactDisplayLabel::FieldLabel).toString();
    }
signals:
    void valueChanged();
};

QTCONTACTS_END_NAMESPACE

QML_DECLARE_TYPE(QTCONTACTS_PREPEND_NAMESPACE(QDeclarativeContactDisplayLabel))

#endif
