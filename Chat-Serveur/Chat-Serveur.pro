# -------------------------------------------------
# Project created 2010-08-25T18:25:52 by Zxb12
# Chat Server.
# -------------------------------------------------
QT += network \
    sql
TARGET = Chat-Serveur
TEMPLATE = app
SOURCES += main.cpp \
    fenprincipale.cpp \
    client.cpp \
    opcode.cpp \
    ../shared/paquet.cpp
HEADERS += fenprincipale.h \
    client.h \
    ../shared/shared.h \
    opcode.h \
    ../shared/paquet.h
FORMS += fenprincipale.ui

MOC_DIR += tmp/moc/
OBJECTS_DIR += tmp/obj/
DESTDIR += bin/
