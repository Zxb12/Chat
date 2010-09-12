/********************************************************************************
** Form generated from reading UI file 'fenprincipale.ui'
**
** Created: Sat 11. Sep 15:39:34 2010
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FENPRINCIPALE_H
#define UI_FENPRINCIPALE_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFormLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FenPrincipale
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QFormLayout *formLayout;
    QLabel *label;
    QLineEdit *adresse;
    QLabel *label_2;
    QSpinBox *port;
    QPushButton *connecter;
    QGridLayout *gridLayout;
    QLineEdit *pseudo;
    QPushButton *renommer;
    QLabel *label_3;
    QPlainTextEdit *chat;
    QHBoxLayout *horizontalLayout;
    QLineEdit *message;
    QPushButton *envoyer;
    QTextEdit *console;

    void setupUi(QWidget *FenPrincipale)
    {
        if (FenPrincipale->objectName().isEmpty())
            FenPrincipale->setObjectName(QString::fromUtf8("FenPrincipale"));
        FenPrincipale->resize(567, 402);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(FenPrincipale->sizePolicy().hasHeightForWidth());
        FenPrincipale->setSizePolicy(sizePolicy);
        FenPrincipale->setMinimumSize(QSize(0, 0));
        verticalLayout = new QVBoxLayout(FenPrincipale);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        formLayout = new QFormLayout();
        formLayout->setSpacing(6);
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        label = new QLabel(FenPrincipale);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        adresse = new QLineEdit(FenPrincipale);
        adresse->setObjectName(QString::fromUtf8("adresse"));

        formLayout->setWidget(0, QFormLayout::FieldRole, adresse);

        label_2 = new QLabel(FenPrincipale);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        port = new QSpinBox(FenPrincipale);
        port->setObjectName(QString::fromUtf8("port"));
        port->setMinimum(1);
        port->setMaximum(65535);
        port->setSingleStep(1);
        port->setValue(50180);

        formLayout->setWidget(1, QFormLayout::FieldRole, port);

        connecter = new QPushButton(FenPrincipale);
        connecter->setObjectName(QString::fromUtf8("connecter"));

        formLayout->setWidget(2, QFormLayout::FieldRole, connecter);


        horizontalLayout_2->addLayout(formLayout);

        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        pseudo = new QLineEdit(FenPrincipale);
        pseudo->setObjectName(QString::fromUtf8("pseudo"));

        gridLayout->addWidget(pseudo, 0, 1, 1, 1);

        renommer = new QPushButton(FenPrincipale);
        renommer->setObjectName(QString::fromUtf8("renommer"));

        gridLayout->addWidget(renommer, 1, 1, 1, 1);

        label_3 = new QLabel(FenPrincipale);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 0, 0, 1, 1);


        horizontalLayout_2->addLayout(gridLayout);


        verticalLayout->addLayout(horizontalLayout_2);

        chat = new QPlainTextEdit(FenPrincipale);
        chat->setObjectName(QString::fromUtf8("chat"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Minimum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(chat->sizePolicy().hasHeightForWidth());
        chat->setSizePolicy(sizePolicy1);

        verticalLayout->addWidget(chat);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        message = new QLineEdit(FenPrincipale);
        message->setObjectName(QString::fromUtf8("message"));

        horizontalLayout->addWidget(message);

        envoyer = new QPushButton(FenPrincipale);
        envoyer->setObjectName(QString::fromUtf8("envoyer"));

        horizontalLayout->addWidget(envoyer);


        verticalLayout->addLayout(horizontalLayout);

        console = new QTextEdit(FenPrincipale);
        console->setObjectName(QString::fromUtf8("console"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(console->sizePolicy().hasHeightForWidth());
        console->setSizePolicy(sizePolicy2);
        console->setMaximumSize(QSize(16777215, 70));
        QFont font;
        font.setFamily(QString::fromUtf8("Courier New"));
        console->setFont(font);
        console->setReadOnly(true);

        verticalLayout->addWidget(console);


        retranslateUi(FenPrincipale);

        QMetaObject::connectSlotsByName(FenPrincipale);
    } // setupUi

    void retranslateUi(QWidget *FenPrincipale)
    {
        FenPrincipale->setWindowTitle(QApplication::translate("FenPrincipale", "FenPrincipale", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("FenPrincipale", "Adresse", 0, QApplication::UnicodeUTF8));
        adresse->setText(QApplication::translate("FenPrincipale", "localhost", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("FenPrincipale", "Port", 0, QApplication::UnicodeUTF8));
        connecter->setText(QApplication::translate("FenPrincipale", "Connecter", 0, QApplication::UnicodeUTF8));
        renommer->setText(QApplication::translate("FenPrincipale", "D\303\251finir pseudo", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("FenPrincipale", "Pseudo", 0, QApplication::UnicodeUTF8));
        envoyer->setText(QApplication::translate("FenPrincipale", "Envoyer", 0, QApplication::UnicodeUTF8));
        console->setHtml(QApplication::translate("FenPrincipale", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Courier New'; font-size:8.25pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:8pt;\">-- CONSOLE --</span></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"></p>\n"
"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:8pt;\"></p></body></html>", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class FenPrincipale: public Ui_FenPrincipale {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FENPRINCIPALE_H
