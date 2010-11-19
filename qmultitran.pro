#-------------------------------------------------
#
# Project created by QtCreator 2010-11-15T20:01:09
#
#-------------------------------------------------

QT       += core gui webkit network

TARGET = qmultitran
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    settingsdialog.cpp

HEADERS  += mainwindow.h \
    settingsdialog.h \
    constants.h

FORMS    += mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    locale/locale.qrc \
    icons/icons.qrc

TRANSLATIONS = \
    locale/ru_RU.ts

OTHER_FILES = \
    COPYING \
    GPLHEADER \
    README \
    TODO
