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
    fenban.cpp
HEADERS += fenprincipale.h \
    opcode.h \
    ../shared/shared.h \
    ../shared/paquet.h \
    fenban.h
FORMS += fenprincipale.ui \
    fenban.ui
RESOURCES += ressources.qrc
MOC_DIR += tmp/moc/
OBJECTS_DIR += tmp/obj/
DESTDIR += bin/
