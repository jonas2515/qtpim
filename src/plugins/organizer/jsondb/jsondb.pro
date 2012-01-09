load(qt_module)

QT += organizer network jsondb

TARGET = qtorganizer_jsondb
target.path += $$[QT_INSTALL_PLUGINS]/organizer
INSTALLS += target

load(qt_plugin)

DESTDIR = $$QT.organizer.plugins/organizer

HEADERS += \
    qorganizerjsondbengine.h \
    qorganizerjsondbrequestthread.h \
    qorganizerjsondbenginefactory.h \
    qorganizerjsondbid.h \
    qorganizerjsondbrequestmanager.h \
    qorganizerjsondbstring.h \
    qorganizerjsondbconverter.h \
    qorganizerjsondbdatastorage.h

SOURCES += \
    qorganizerjsondbengine.cpp \
    qorganizerjsondbrequestthread.cpp \
    qorganizerjsondbenginefactory.cpp \
    qorganizerjsondbid.cpp \
    qorganizerjsondbrequestmanager.cpp \
    qorganizerjsondbstring.cpp \
    qorganizerjsondbconverter.cpp \
    qorganizerjsondbdatastorage.cpp
