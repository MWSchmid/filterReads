#-------------------------------------------------
#
# Project created by QtCreator 2014-11-05T07:50:28
#
#-------------------------------------------------

QT       += core gui

TARGET = p502filterReads
TEMPLATE = app

SOURCES += ../p502_SOURCE/fastaHandler/fastahandler.cpp \
    ../p502_SOURCE/readsHandlerIllumina/readshandlerillumina.cpp \
    ../p502_SOURCE/readsHandlerIllumina/illuminareader.cpp \
    ../p502_SOURCE/readsHandlerIllumina/illuminawriter.cpp \
    main.cpp\
    widget.cpp \
    readsfilter.cpp \
    coordinator.cpp

HEADERS += ../p502_SOURCE/fastaHandler/fastahandler.h \
    ../p502_SOURCE/readsHandlerIllumina/readshandlerillumina.h \
    ../p502_SOURCE/readsHandlerIllumina/readshandlerilluminahelper.h \
    ../p502_SOURCE/readsHandlerIllumina/illuminareader.h \
    ../p502_SOURCE/readsHandlerIllumina/illuminawriter.h \
    widget.h \
    readsfilter.h \
    consoleRunner.h \
    coordinator.h

include(../qtiocompressor-2.3_1-opensource/src/qtiocompressor.pri)

FORMS    += widget.ui
