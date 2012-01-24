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

#ifndef QVERSITCONTACTEXPORTER_H
#define QVERSITCONTACTEXPORTER_H

#include <qversitresourcehandler.h>
#include <qversitdocument.h>

#include <qcontact.h>

#include <QImage>

QTVERSIT_BEGIN_NAMESPACE

class QVersitContactExporterPrivate;

class Q_VERSIT_EXPORT QVersitContactExporterDetailHandler
{
public:
    virtual ~QVersitContactExporterDetailHandler() {}
    virtual bool preProcessDetail(const QTCONTACTS_PREPEND_NAMESPACE(QContact)& contact,
                                  const QTCONTACTS_PREPEND_NAMESPACE(QContactDetail)& detail,
                                  QVersitDocument* document) = 0;
    virtual bool postProcessDetail(const QTCONTACTS_PREPEND_NAMESPACE(QContact)& contact,
                                   const QTCONTACTS_PREPEND_NAMESPACE(QContactDetail)& detail,
                                   bool alreadyProcessed,
                                   QVersitDocument* document) = 0;

};

class Q_VERSIT_EXPORT QVersitContactExporterDetailHandlerV2
{
public:
    virtual ~QVersitContactExporterDetailHandlerV2() {}

    virtual void detailProcessed(const QTCONTACTS_PREPEND_NAMESPACE(QContact)& contact,
                                 const QTCONTACTS_PREPEND_NAMESPACE(QContactDetail)& detail,
                                 const QVersitDocument& document,
                                 QSet<QString>* processedFields,
                                 QList<QVersitProperty>* toBeRemoved,
                                 QList<QVersitProperty>* toBeAdded) = 0;
    virtual void contactProcessed(const QTCONTACTS_PREPEND_NAMESPACE(QContact)& contact,
                                  QVersitDocument* document) = 0;
};

class Q_VERSIT_EXPORT QVersitContactExporter
{
public:
    enum Error {
        NoError = 0,
        EmptyContactError,
        NoNameError
    };

    QVersitContactExporter();
    explicit QVersitContactExporter(const QString& profile);
    explicit QVersitContactExporter(const QStringList& profile);
    ~QVersitContactExporter();

    bool exportContacts(const QList<QTCONTACTS_PREPEND_NAMESPACE(QContact)>& contacts,
            QVersitDocument::VersitType versitType = QVersitDocument::VCard30Type);
    QList<QVersitDocument> documents() const;
    QMap<int, Error> errorMap() const;

    void setDetailHandler(QVersitContactExporterDetailHandlerV2* handler);

    void setResourceHandler(QVersitResourceHandler* handler);
    QVersitResourceHandler* resourceHandler() const;

    /* deprecated */
    QMap<int, Error> errors() const;
    void setDetailHandler(QVersitContactExporterDetailHandler* handler);
    QVersitContactExporterDetailHandler* detailHandler() const;

private:
    QVersitContactExporterPrivate* d;
};

QTVERSIT_END_NAMESPACE

#endif // QVERSITCONTACTEXPORTER_H
