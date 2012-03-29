/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the test suite of the Qt Toolkit.
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

#ifndef JSONDBPROCESS_H
#define JSONDBPROCESS_H

#include <QtTest/QtTest>
#include <QProcess>
#include <QTemporaryDir>
#include <QCoreApplication>
#include <QLibraryInfo>

#if !defined(QT_NO_JSONDB)
#include <QtJsonDb>
QT_USE_NAMESPACE_JSONDB
#endif//QT_NO_JSONDB

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

class JsonDbProcess : public QObject
{
    Q_OBJECT
public:
    JsonDbProcess();

    bool start(bool forceNewProcess = false, QStringList &wantedPartitions = QStringList()<<"com.nokia.mt.User"<<"com.nokia.mt.System");
    void terminate();

#if !defined(QT_NO_JSONDB)
signals:
    void quitEventLoop();

private:
    void checkPartitions();

private slots:
    void onConnection(QtJsonDb::QJsonDbConnection::ErrorCode error = QJsonDbConnection::NoError , const QString &message = QString());
    void onPartitionsChecked(QtJsonDb::QJsonDbRequest::ErrorCode error = QJsonDbRequest::NoError , const QString &message = QString());
    void onPartitionsCreated(QtJsonDb::QJsonDbRequest::ErrorCode error = QJsonDbRequest::NoError , const QString &message = QString());
#endif//QT_NO_JSONDB

private:
    QProcess m_process;
    QTemporaryDir jsondbWorkingDirectory;
#if !defined(QT_NO_JSONDB)
    int m_CreationRequestAmount;
    QStringList m_wantedPartitions;
    QJsonDbConnection m_JsonDbConnection;
#endif//QT_NO_JSONDB
};

#endif // JSONDBPROCESS_H
