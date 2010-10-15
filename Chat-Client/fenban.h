#ifndef FENBAN_H
#define FENBAN_H

#include <QDialog>
#include <QMessageBox>
#include <QTimer>

namespace Ui {
    class FenBan;
}

class FenBan : public QDialog
{
    Q_OBJECT
public:
    FenBan(QWidget *parent, QString*, quint32*, QString*, bool*);
    ~FenBan();

private:
    Ui::FenBan *ui;
    QTimer *timer;

    QString *m_quiBannir;
    quint32 *m_duree;
    QString *m_raison;
    bool *m_ok;

private slots:
    void updateDateTime();
    void on_validation_rejected();
    void on_validation_accepted();
};

#endif // FENBAN_H
