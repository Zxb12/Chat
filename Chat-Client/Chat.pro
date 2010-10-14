# -------------------------------------------------
# Project created by QtCreator 2010-08-25T16:49:14
# -------------------------------------------------
QT += network
TARGET = Chat
TEMPLATE = app
SOURCES += main.cpp \
    fenprincipale.cpp \
    opcode.cpp \
    ../shared/paquet.cpp
HEADERS += fenprincipale.h \
    opcode.h \
    ../shared/shared.h \
    ../shared/paquet.h
FORMS += fenprincipale.ui

RESOURCES += \
    ressources.qrc
