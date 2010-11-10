#include <QtCore/QCoreApplication>
#include "fenprincipale.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    FenPrincipale server;
    return a.exec();
}
