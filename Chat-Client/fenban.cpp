#include "fenban.h"
#include "ui_fenban.h"

FenBan::FenBan(QWidget *parent, QString& quiBannir, quint32& duree, QString& raison, bool &ok) :
        QDialog(parent), ui(new Ui::FenBan), m_quiBannir(quiBannir), m_duree(duree), m_raison(raison), m_ok(ok)
{
    ui->setupUi(this);

    //Timer pour la mise à jour du QDateTime
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateDateTime()));
    timer->start(1000);
    updateDateTime();   //Mise à jour de la limite dès l'ouverture
}

FenBan::~FenBan()
{
    delete timer;
    delete ui;
}

void FenBan::updateDateTime()
{
    QDateTime now = QDateTime::currentDateTime();
    ui->dateFinBan->setMinimumDateTime(now);
}

void FenBan::on_validation_accepted()
{
    QString quiBannir = ui->quiBannir->text().trimmed();
    QString raison = ui->raison->text().trimmed();
    quint32 duree = QDateTime::currentDateTime().secsTo(ui->dateFinBan->dateTime());

    if (quiBannir.isEmpty())
    {
        QMessageBox::warning(this, "OokChat", "Veuillez spécifier un utilisateur.");
        return;
    }

    //On assure le ban à vie.
    if (ui->banVie->isChecked())
        duree = 0;

    //Attribution des valeurs
    m_quiBannir = quiBannir;
    m_raison = raison;
    m_duree = duree;
    m_ok = true;
    this->close();
}

void FenBan::on_validation_rejected()
{
    m_ok = false;
    this->close();
}
