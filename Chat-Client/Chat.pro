# -------------------------------------------------
# Project created 2010-08-25T16:49:14 by Zxb12
# Chat Client
# -------------------------------------------------
QT += network
TARGET = Chat-Client
TEMPLATE = app
SOURCES += main.cpp \
    fenprincipale.cpp \
    opcode.cpp \
    ../shared/paquet.cpp \
    fenban.cpp \
    fenchannel.cpp
HEADERS += fenprincipale.h \
    opcode.h \
    ../shared/shared.h \
    ../shared/paquet.h \
    fenban.h \
    fenchannel.h
FORMS += fenprincipale.ui \
    fenban.ui \
    fenchannel.ui
RESOURCES += ressources.qrc
UI_HEADERS_DIR += tmp/ui/headers
UI_SOURCES_DIR += tmp/ui/src
MOC_DIR += tmp/moc/
OBJECTS_DIR += tmp/obj/
RCC_DIR += tmp/rcc/
DESTDIR += bin/
