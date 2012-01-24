/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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
#include <qcontactdetails.h>
#include <QtDeclarative/qdeclarativeinfo.h>

#include "qdeclarativecontactmodel_p.h"
#include "qcontactmanager.h"
#include "qcontactdetailfilter.h"
#include "qcontactidfilter.h"
#include "qversitreader.h"
#include "qversitwriter.h"
#include "qversitcontactimporter.h"
#include "qversitcontactexporter.h"
#include <QColor>
#include <QHash>
#include <QDebug>
#include <QPixmap>
#include <QFile>
#include <QMap>

#include "qcontactrequests.h"

/*!
    \qmlclass ContactModel QDeclarativeContactModel
    \brief The ContactModel element provides access to contacts from the contacts store.
    \ingroup qml-contacts-main
    \inqmlmodule QtContacts

    This element is part of the \bold{QtContacts} module.

    ContactModel provides a model of contacts from the contacts store.
    The contents of the model can be specified with \l filter, \l sortOrders and \l fetchHint properties.
    Whether the model is automatically updated when the store or \l contacts changes, can be
    controlled with \l ContactModel::autoUpdate property.

    There are two ways of accessing the contact data: via model by using views and delegates,
    or alternatively via \l contacts list property. Of the two, the model access is preferred.
    Direct list access (i.e. non-model) is not guaranteed to be in order set by \l sortOrder.

    At the moment the model roles provided by ContactModel are display, decoration and \c contact.
    Through the \c contact role can access any data provided by the Contact element.

    \sa RelationshipModel, Contact, {QContactManager}
*/

QTCONTACTS_BEGIN_NAMESPACE

class QDeclarativeContactModelPrivate
{
public:
    QDeclarativeContactModelPrivate()
        :m_manager(0),
        m_fetchHint(0),
        m_filter(0),
        m_fetchRequest(0),
        m_error(QContactManager::NoError),
        m_autoUpdate(true),
        m_updatePending(false),
        m_componentCompleted(false)
    {
    }
    ~QDeclarativeContactModelPrivate()
    {
        if (m_manager)
            delete m_manager;
    }

    QList<QDeclarativeContact*> m_contacts;
    QMap<QContactId, QDeclarativeContact*> m_contactMap;
    QContactManager* m_manager;
    QDeclarativeContactFetchHint* m_fetchHint;
    QList<QDeclarativeContactSortOrder*> m_sortOrders;
    QDeclarativeContactFilter* m_filter;

    QContactFetchRequest* m_fetchRequest;
    QList<QContactId> m_updatedContactIds;
    QVersitReader m_reader;
    QVersitWriter m_writer;
    QStringList m_importProfiles;

    QContactManager::Error m_error;

    bool m_autoUpdate;
    bool m_updatePending;
    bool m_componentCompleted;
};

QDeclarativeContactModel::QDeclarativeContactModel(QObject *parent) :
    QAbstractListModel(parent),
    d(new QDeclarativeContactModelPrivate)
{
    QHash<int, QByteArray> roleNames;
    roleNames = QAbstractItemModel::roleNames();
    roleNames.insert(ContactRole, "contact");
    setRoleNames(roleNames);

    connect(this, SIGNAL(managerChanged()), SLOT(update()));
    connect(this, SIGNAL(filterChanged()), SLOT(update()));
    connect(this, SIGNAL(fetchHintChanged()), SLOT(update()));
    connect(this, SIGNAL(sortOrdersChanged()), SLOT(update()));
    
    //import vcard
    connect(&d->m_reader, SIGNAL(stateChanged(QVersitReader::State)), this, SLOT(startImport(QVersitReader::State)));
    connect(&d->m_writer, SIGNAL(stateChanged(QVersitWriter::State)), this, SLOT(contactsExported(QVersitWriter::State)));
}

/*!
  \qmlproperty string ContactModel::manager

  This property holds the manager uri of the contact backend engine.
  */
QString QDeclarativeContactModel::manager() const
{
    if (d->m_manager)
    	return d->m_manager->managerName();
    return QString();
}
void QDeclarativeContactModel::setManager(const QString& managerName)
{
    if (d->m_manager)
        delete d->m_manager;


    d->m_manager = new QContactManager(managerName);

    connect(d->m_manager, SIGNAL(dataChanged()), this, SLOT(update()));
    connect(d->m_manager, SIGNAL(contactsAdded(QList<QContactId>)), this, SLOT(onContactsAdded(QList<QContactId>)));
    connect(d->m_manager, SIGNAL(contactsRemoved(QList<QContactId>)), this, SLOT(onContactsRemoved(QList<QContactId>)));
    connect(d->m_manager, SIGNAL(contactsChanged(QList<QContactId>)), this, SLOT(onContactsChanged(QList<QContactId>)));

    if (d->m_error != QContactManager::NoError) {
        d->m_error = QContactManager::NoError;
        emit errorChanged();
    }

    emit managerChanged();
}
void QDeclarativeContactModel::componentComplete()
{
    d->m_componentCompleted = true;
    if (!d->m_manager)
        setManager(QString());

    if (d->m_autoUpdate)
        update();
}
/*!
  \qmlproperty bool ContactModel::autoUpdate

  This property indicates whether or not the contact model should be updated automatically, default value is true.
  */
void QDeclarativeContactModel::setAutoUpdate(bool autoUpdate)
{
    if (autoUpdate == d->m_autoUpdate)
        return;
    d->m_autoUpdate = autoUpdate;
    emit autoUpdateChanged();
}

bool QDeclarativeContactModel::autoUpdate() const
{
    return d->m_autoUpdate;
}

void QDeclarativeContactModel::update()
{
    if (!d->m_componentCompleted || d->m_updatePending)
        return;
    d->m_updatePending = true; // Disallow possible duplicate request triggering
    QMetaObject::invokeMethod(this, "fetchAgain", Qt::QueuedConnection);
}

/*!
  \qmlmethod ContactModel::cancelUpdate()

  Cancel any unfinished requests to update the contacts in the model.

  \sa ContactModel::autoUpdate  ContactModel::update
  */
void QDeclarativeContactModel::cancelUpdate()
{
    if (d->m_fetchRequest) {
        d->m_fetchRequest->cancel();
        d->m_fetchRequest->deleteLater();
        d->m_fetchRequest = 0;
        d->m_updatePending = false;
    }
}

/*!
  \qmlproperty string ContactModel::error

  This property holds the latest error code returned by the contact manager.

  This property is read only.
  */
QString QDeclarativeContactModel::error() const
{
    if (d->m_manager) {
        switch (d->m_error) {
        case QContactManager::DoesNotExistError:
            return QLatin1String("DoesNotExist");
        case QContactManager::AlreadyExistsError:
            return QLatin1String("AlreadyExists");
        case QContactManager::InvalidDetailError:
            return QLatin1String("InvalidDetail");
        case QContactManager::InvalidRelationshipError:
            return QLatin1String("InvalidRelationship");
        case QContactManager::LockedError:
            return QLatin1String("LockedError");
        case QContactManager::DetailAccessError:
            return QLatin1String("DetailAccessError");
        case QContactManager::PermissionsError:
            return QLatin1String("PermissionsError");
        case QContactManager::OutOfMemoryError:
            return QLatin1String("OutOfMemory");
        case QContactManager::NotSupportedError:
            return QLatin1String("NotSupported");
        case QContactManager::BadArgumentError:
            return QLatin1String("BadArgument");
        case QContactManager::UnspecifiedError:
            return QLatin1String("UnspecifiedError");
        case QContactManager::VersionMismatchError:
            return QLatin1String("VersionMismatch");
        case QContactManager::LimitReachedError:
            return QLatin1String("LimitReached");
        case QContactManager::InvalidContactTypeError:
            return QLatin1String("InvalidContactType");
        default:
            break;
        }
    }
    return QLatin1String("NoError");
}


/*!
  \qmlproperty list<string> ContactModel::availableManagers

  This property holds the list of available manager names.
  This property is read only.
  */
QStringList QDeclarativeContactModel::availableManagers() const
{
    return QContactManager::availableManagers();
}
static QString urlToLocalFileName(const QUrl& url)
{
   if (!url.isValid()) {
      return url.toString();
   } else if (url.scheme() == "qrc") {
      return url.toString().remove(0, 5).prepend(':');
   } else {
      return url.toLocalFile();
   }

}

/*!
  \qmlmethod ContactModel::importContacts(url url, list<string> profiles)

  Import contacts from a vcard by the given \a url and optional \a profiles.
  Supported profiles are:
  \list
  \o "Sync"  Imports contacts in sync mode, currently, this is the same as passing in an empty list, and is generally what you want.
  \o "Backup" imports contacts in backup mode, use this mode if the vCard was generated by exporting in backup mode.

  \endlist

  \sa QVersitContactHandlerFactory
  \sa QVersitContactHandlerFactory::ProfileSync
  \sa QVersitContactHandlerFactory::ProfileBackup

  */
void QDeclarativeContactModel::importContacts(const QUrl& url, const QStringList& profiles)
{
   d->m_importProfiles = profiles;

   //TODO: need to allow download vcard from network
   QFile*  file = new QFile(urlToLocalFileName(url));
   bool ok = file->open(QIODevice::ReadOnly);
   if (ok) {
      d->m_reader.setDevice(file);
      d->m_reader.startReading();
   }
}

/*!
  \qmlmethod ContactModel::exportContacts(url url, list<string> profiles)

  Export contacts into a vcard file to the given \a url by optional \a profiles.
  At the moment only the local file url is supported in export method.
  Supported profiles are:
  \list
  \o "Sync"  exports contacts in sync mode, currently, this is the same as passing in an empty list, and is generally what you want.
  \o "Backup" exports contacts in backup mode, this will add non-standard properties to the generated vCard
              to try to save every detail of the contacts. Only use this if the vCard is going to be imported using the backup profile.
#include "moc_qdeclarativecontactmodel_p.cpp"
  \endlist

  \sa QVersitContactHandlerFactory
  \sa QVersitContactHandlerFactory::ProfileSync
  \sa QVersitContactHandlerFactory::ProfileBackup
  */
void QDeclarativeContactModel::exportContacts(const QUrl& url, const QStringList& profiles)
{

   QString profile = profiles.isEmpty()? QString() : profiles.at(0);
    //only one profile string supported now
   QVersitContactExporter exporter(profile);

   QList<QContact> contacts;
   foreach (QDeclarativeContact* dc, d->m_contacts) {
       contacts.append(dc->contact());
   }

   exporter.exportContacts(contacts, QVersitDocument::VCard30Type);
   QList<QVersitDocument> documents = exporter.documents();
   QFile* file = new QFile(urlToLocalFileName(url));
   bool ok = file->open(QIODevice::WriteOnly);
   if (ok) {
      d->m_writer.setDevice(file);
      d->m_writer.startWriting(documents);
   }
}

void QDeclarativeContactModel::contactsExported(QVersitWriter::State state)
{
    if (state == QVersitWriter::FinishedState || state == QVersitWriter::CanceledState) {
         delete d->m_writer.device();
         d->m_writer.setDevice(0);
    }
}

int QDeclarativeContactModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->m_contacts.count();
}



/*!
  \qmlproperty Filter ContactModel::filter

  This property holds the filter instance used by the contact model.

  \sa Filter
  */
QDeclarativeContactFilter* QDeclarativeContactModel::filter() const
{
    return d->m_filter;
}

void QDeclarativeContactModel::setFilter(QDeclarativeContactFilter* filter)
{
    if (d->m_filter != filter) {
        if (d->m_filter) {
            disconnect(d->m_filter, SIGNAL(filterChanged()), this, SLOT(update()));
        }
        d->m_filter = filter;
        if (d->m_filter) {
            connect(d->m_filter, SIGNAL(filterChanged()), SLOT(update()), Qt::UniqueConnection);
        }
        emit filterChanged();
    }
}

/*!
  \qmlproperty FetchHint ContactModel::fetchHint

  This property holds the fetch hint instance used by the contact model.

  \sa FetchHint
  */
QDeclarativeContactFetchHint* QDeclarativeContactModel::fetchHint() const
{
    return d->m_fetchHint;
}
void QDeclarativeContactModel::setFetchHint(QDeclarativeContactFetchHint* fetchHint)
{
    if (fetchHint && fetchHint != d->m_fetchHint) {
        if (d->m_fetchHint)
            delete d->m_fetchHint;
        d->m_fetchHint = fetchHint;
        emit fetchHintChanged();
    }
}

/*!
  \qmlproperty list<Contact> ContactModel::contacts

  This property holds the list of contacts.

  \sa Contact
  */
QDeclarativeListProperty<QDeclarativeContact> QDeclarativeContactModel::contacts()
{
    return QDeclarativeListProperty<QDeclarativeContact>(this,
                                                         0,
                                                         contacts_append,
                                                         contacts_count,
                                                         contacts_at,
                                                         contacts_clear);
}



void QDeclarativeContactModel::contacts_append(QDeclarativeListProperty<QDeclarativeContact>* prop, QDeclarativeContact* contact)
{
    Q_UNUSED(prop);
    Q_UNUSED(contact);
    qmlInfo(0) << tr("ContactModel: appending contacts is not currently supported");
}

int QDeclarativeContactModel::contacts_count(QDeclarativeListProperty<QDeclarativeContact>* prop)
{
    return static_cast<QDeclarativeContactModel*>(prop->object)->d->m_contacts.count();
}

QDeclarativeContact* QDeclarativeContactModel::contacts_at(QDeclarativeListProperty<QDeclarativeContact>* prop, int index)
{
    return static_cast<QDeclarativeContactModel*>(prop->object)->d->m_contacts.at(index);
}

void QDeclarativeContactModel::contacts_clear(QDeclarativeListProperty<QDeclarativeContact>* prop)
{
    QDeclarativeContactModel* model = static_cast<QDeclarativeContactModel*>(prop->object);
    model->clearContacts();
    emit model->contactsChanged();
}


/*!
  \qmlproperty list<SortOrder> ContactModel::sortOrders

  This property holds a list of sort orders used by the organizer model.

  \sa SortOrder
  */
QDeclarativeListProperty<QDeclarativeContactSortOrder> QDeclarativeContactModel::sortOrders()
{
    return QDeclarativeListProperty<QDeclarativeContactSortOrder>(this,
                                                                  0,
                                                                  sortOrder_append,
                                                                  sortOrder_count,
                                                                  sortOrder_at,
                                                                  sortOrder_clear);
}

void QDeclarativeContactModel::startImport(QVersitReader::State state)
{
    if (state == QVersitReader::FinishedState || state == QVersitReader::CanceledState) {
        QVersitContactImporter importer(d->m_importProfiles);
        importer.importDocuments(d->m_reader.results());
        QList<QContact> contacts = importer.contacts();

        delete d->m_reader.device();
        d->m_reader.setDevice(0);

        if (d->m_manager) {
            for (int i = 0; i < contacts.size(); i++) {
                contacts[i] = d->m_manager->compatibleContact(contacts[i]);
            }
            if (d->m_manager->saveContacts(&contacts)) {
                qmlInfo(this) << tr("contacts imported.");
                update();
            } else {
                if (d->m_error != d->m_manager->error()) {
                    d->m_error = d->m_manager->error();
                    emit errorChanged();
                }
            }
        }
    }
}

/*!
  \qmlmethod ContactModel::fetchContacts(list<string> contactIds)
  Fetch a list of contacts from the contacts store by given \a contactIds.

  \sa Contact::contactId
  */
void QDeclarativeContactModel::fetchContacts(const QStringList &contactIds)
{
    QList<QContactId> ids;
    foreach (const QString &contactIdString, contactIds){
        QContactId contactId = QContactId::fromString(contactIdString);
        if (!contactId.isNull())
            ids.append(contactId);
    }

    d->m_updatedContactIds = ids;
    d->m_updatePending = true;
    QMetaObject::invokeMethod(this, "fetchAgain", Qt::QueuedConnection);
}
void QDeclarativeContactModel::clearContacts()
{
    qDeleteAll(d->m_contacts);
    d->m_contacts.clear();
    d->m_contactMap.clear();
}

void QDeclarativeContactModel::fetchAgain()
{
    cancelUpdate();

    QList<QContactSortOrder> sortOrders;
    foreach (QDeclarativeContactSortOrder* so, d->m_sortOrders) {
        sortOrders.append(so->sortOrder());
    }
    d->m_fetchRequest = new QContactFetchRequest(this);
    d->m_fetchRequest->setManager(d->m_manager);
    d->m_fetchRequest->setSorting(sortOrders);
    // ids are ignored and all contacts (matching the filter) are always fetched
    if (d->m_filter){
        d->m_fetchRequest->setFilter(d->m_filter->filter());
    } else {
        d->m_fetchRequest->setFilter(QContactFilter());
    }

    d->m_fetchRequest->setFetchHint(d->m_fetchHint ? d->m_fetchHint->fetchHint() : QContactFetchHint());

    connect(d->m_fetchRequest,SIGNAL(stateChanged(QContactAbstractRequest::State)), this, SLOT(requestUpdated()));

    d->m_fetchRequest->start();
}

void QDeclarativeContactModel::requestUpdated()
{
    //Don't use d->m_fetchRequest, this pointer might be invalid if cancelUpdate() was called, use QObject::sender() instead.
    QContactFetchRequest* req = qobject_cast<QContactFetchRequest*>(QObject::sender());
    if (req && req->isFinished()) {
        QList<QContact> contacts = req->contacts();
        QList<QDeclarativeContact*> dcs;
        foreach (const QContact &c, contacts) {
            if (d->m_contactMap.contains(c.id())) {
                QDeclarativeContact* dc = d->m_contactMap.value(c.id());
                dc->setContact(c);
                dcs.append(dc);
            } else {
                QDeclarativeContact* dc = new QDeclarativeContact(this);
                if (dc) {
                    d->m_contactMap.insert(c.id(), dc);
                    dc->setContact(c);
                    dcs.append(dc);
                }
            }
        }

        if (d->m_contacts.isEmpty()) {
            reset();
            if (contacts.count()>0) {
                beginInsertRows(QModelIndex(), 0, contacts.count() - 1);
                d->m_contacts = dcs;
                endInsertRows();
            }
        } else {
            //Partial updating, insert the fetched contacts into the the exist contact list.
            beginResetModel();
            d->m_contacts.clear();
            if (dcs.count() > 0) {
                d->m_contacts = dcs;
            }
            endResetModel();
        }
        emit contactsChanged();
        checkError(req);
        req->deleteLater();
        d->m_fetchRequest = 0;
        d->m_updatePending = false;
    }
}

/*!
  \qmlmethod ContactModel::saveContact(Contact contact)
  Save the given \a contact into the contacts store. Once saved successfully, the dirty flags of this contact will be reset.

  \sa Contact::modified
  */
void QDeclarativeContactModel::saveContact(QDeclarativeContact* dc)
{
    if (dc) {
        QContact c = d->m_manager->compatibleContact(dc->contact());
        QContactSaveRequest* req = new QContactSaveRequest(this);
        req->setManager(d->m_manager);
        req->setContact(c);
        connect(req,SIGNAL(stateChanged(QContactAbstractRequest::State)), this, SLOT(onRequestStateChanged(QContactAbstractRequest::State)));
        req->start();
    }
}

void QDeclarativeContactModel::onRequestStateChanged(QContactAbstractRequest::State newState)
{
    if (newState != QContactAbstractRequest::FinishedState) {
        return;
    }
    if (!sender()) {
        qWarning() << Q_FUNC_INFO << "Called without a sender.";
        return;
    }

    QContactAbstractRequest *request = qobject_cast<QContactAbstractRequest *>(sender());
    if (!request)
        return;
    checkError(request);
    request->deleteLater();
}

void QDeclarativeContactModel::checkError(const QContactAbstractRequest *request)
{
    if (request && d->m_error != request->error()) {
        d->m_error = request->error();
        emit errorChanged();
    }
}

void QDeclarativeContactModel::onContactsAdded(const QList<QContactId>& ids)
{
    Q_UNUSED(ids)
    if (d->m_autoUpdate) {
        QStringList empty;
        fetchContacts(empty);
    }
}

/*!
  \qmlmethod ContactModel::removeContact(string contactId)
  Remove the contact from the contacts store by given \a contactId.
  \sa Contact::contactId
  */
void QDeclarativeContactModel::removeContact(QString id)
{
    QList<QString> ids;
    ids << id;
    removeContacts(ids);
}

/*!
  \qmlmethod ContactModel::removeContacts(list<string> contactIds)
  Remove the list of contacts from the contacts store by given \a contactIds.
  \sa Contact::contactId
  */

void QDeclarativeContactModel::removeContacts(const QStringList &ids)
{
    QContactRemoveRequest* req = new QContactRemoveRequest(this);
    QList<QContactId> contactIdsAsList;
    req->setManager(d->m_manager);

    foreach (const QString& id, ids) {
        QContactId contactId = QContactId::fromString(id);
        if (!contactId.isNull())
            contactIdsAsList.append(contactId);
    }
    req->setContactIds(contactIdsAsList);

    connect(req,SIGNAL(stateChanged(QContactAbstractRequest::State)), this, SLOT(onRequestStateChanged(QContactAbstractRequest::State)));

    req->start();
}

void QDeclarativeContactModel::onContactsRemoved(const QList<QContactId> &ids)
{
    if (!d->m_autoUpdate)
        return;

    bool emitSignal = false;
    foreach (const QContactId &id, ids) {
        if (d->m_contactMap.contains(id)) {
            int row = 0;
            //TODO:need a fast lookup
            for (; row < d->m_contacts.count(); row++) {
                if (d->m_contacts.at(row)->contactId() == id.toString())
                    break;
            }

            if (row < d->m_contacts.count()) {
                beginRemoveRows(QModelIndex(), row, row);
                d->m_contacts.removeAt(row);
                d->m_contactMap.remove(id);
                endRemoveRows();
                emitSignal = true;
            }
        }
    }
    if (emitSignal)
        emit contactsChanged();
}

void QDeclarativeContactModel::onContactsChanged(const QList<QContactId> &ids)
{
    if (d->m_autoUpdate) {
        QStringList updatedIds;
        foreach (const QContactId& id, ids)
            if (d->m_contactMap.contains(id)) {
                updatedIds.append(id.toString());
            }

        if (updatedIds.count() > 0) {
            QStringList empty;
            fetchContacts(empty);
        }
    }
}

QVariant QDeclarativeContactModel::data(const QModelIndex &index, int role) const
{
    //Check if QList itme's index is valid before access it, index should be between 0 and count - 1
    if (index.row() < 0 || index.row() >= d->m_contacts.count()) {
        return QVariant();
    }

    QDeclarativeContact* dc = d->m_contacts.value(index.row());
    Q_ASSERT(dc);
    QContact c = dc->contact();

    switch(role) {
        case Qt::DisplayRole:
            return c.displayLabel();
        case Qt::DecorationRole:
            {
                QContactThumbnail t = c.detail<QContactThumbnail>();
                if (!t.thumbnail().isNull())
                    return QPixmap::fromImage(t.thumbnail());
                return QPixmap();
            }
        case ContactRole:
            return QVariant::fromValue(dc);
    }
    return QVariant();
}


void QDeclarativeContactModel::sortOrder_append(QDeclarativeListProperty<QDeclarativeContactSortOrder> *p, QDeclarativeContactSortOrder *sortOrder)
{
    QDeclarativeContactModel* model = qobject_cast<QDeclarativeContactModel*>(p->object);
    if (model && sortOrder) {
        QObject::connect(sortOrder, SIGNAL(sortOrderChanged()), model, SIGNAL(sortOrdersChanged()));
        model->d->m_sortOrders.append(sortOrder);
        emit model->sortOrdersChanged();
    }
}

int  QDeclarativeContactModel::sortOrder_count(QDeclarativeListProperty<QDeclarativeContactSortOrder> *p)
{
    QDeclarativeContactModel* model = qobject_cast<QDeclarativeContactModel*>(p->object);
    if (model)
        return model->d->m_sortOrders.size();
    return 0;
}
QDeclarativeContactSortOrder * QDeclarativeContactModel::sortOrder_at(QDeclarativeListProperty<QDeclarativeContactSortOrder> *p, int idx)
{
    QDeclarativeContactModel* model = qobject_cast<QDeclarativeContactModel*>(p->object);

    QDeclarativeContactSortOrder* sortOrder = 0;
    if (model) {
        int i = 0;
        foreach (QDeclarativeContactSortOrder* s, model->d->m_sortOrders) {
            if (i == idx) {
                sortOrder = s;
                break;
            } else {
                i++;
            }
        }
    }
    return sortOrder;
}
void  QDeclarativeContactModel::sortOrder_clear(QDeclarativeListProperty<QDeclarativeContactSortOrder> *p)
{
    QDeclarativeContactModel* model = qobject_cast<QDeclarativeContactModel*>(p->object);

    if (model) {
        model->d->m_sortOrders.clear();
        emit model->sortOrdersChanged();
    }
}


#include "moc_qdeclarativecontactmodel_p.cpp"

QTCONTACTS_END_NAMESPACE
