#include "asistanttipapp.h"
#include "data/globals.h"
#include "ui_asistanttipapp.h"

AsistantTipApp::AsistantTipApp(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AsistantTipApp)
{
    ui->setupUi(this);

    setWindowTitle(tr("Asistentul sfaturilor aplicației."));
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    db = new DataBase(this);

    connect(ui->btnNext, &QAbstractButton::clicked, this, &AsistantTipApp::stepNext);
    connect(ui->btnPreview, &QAbstractButton::clicked, this, &AsistantTipApp::stepPreview);
    connect(ui->btnClose, &QAbstractButton::clicked, this, &AsistantTipApp::onClose);

    ui->btnPreview->setStyleSheet("width: 95px;");
    ui->btnNext->setStyleSheet("width: 95px;");
    ui->btnClose->setStyleSheet("width: 95px;");

    ui->show_launch->setChecked(globals::showAsistantHelper);

#if defined(Q_OS_WIN)
    ui->tip_text->setStyleSheet("font: 14px 'Cantarell';");
    ui->frame->setStyle(style_fusion);
#endif

}

AsistantTipApp::~AsistantTipApp()
{
    delete db;
    delete ui;
}

void AsistantTipApp::setStep(int m_step)
{
    if (m_step == 0)
        m_step += 1;

    current_step = m_step;
    setTipText();
}

void AsistantTipApp::stepNext()
{
    if ((current_step + 1) > max_step)
        return;

    current_step += 1;
    setTipText();
}

void AsistantTipApp::stepPreview()
{
    if ((current_step - 1) == 0)
        return;

    current_step -= 1;
    setTipText();
}

void AsistantTipApp::onClose()
{
    if (ui->show_launch->checkState() != globals::showAsistantHelper){
        QSqlQuery qry;
        qry.prepare("UPDATE userPreferences SET showAsistantHelper = :showAsistantHelper WHERE id_users = :id_users;");
        qry.bindValue(":showAsistantHelper", ui->show_launch->isChecked());
        qry.bindValue(":id_users",           globals::idUserApp);
        if (! qry.exec())
            qWarning(logWarning()) << tr("Nu a fost actualizate date de prezentare a asistentului de sfaturi %1")
                                          .arg((qry.lastError().text().isEmpty()) ? "" : "- " + qry.lastError().text());
        globals::showAsistantHelper = ui->show_launch->isChecked();
    }
    this->close();
}

void AsistantTipApp::setTipText()
{
    if (current_step == 1)
        ui->tip_text->setHtml(tr("<u><b>Preferințele utilizatorului</b></u> - configurarea setărilor utilizatorului:"
                                 "<li>&nbsp;&nbsp; - Setările generale - setarea aplicației implicite, logotipul etc.</li>"
                                 "<li>&nbsp;&nbsp; - Lansarea/închiderea - setările la lansarea sau închiderea aplicației.</li>"
                                 "<li>&nbsp;&nbsp; - Setările documentelor - setări pentru documente.</li>"
                                 "<li>&nbsp;&nbsp; - Setările neredactabile - setările informative.</li>"));

    if (current_step == 2)
        ui->tip_text->setHtml(tr("<u><b>Jurnalul de logare</b></u> - vizualizați jurnalul de logare.<br>"
                                 "Deschideți Setările aplicației -> Tabul 'Fișiere de logare' -> alegeți fișierul pentru a vizualiza "
                                 "jurnalul."));

    if (current_step == 3)
        ui->tip_text->setHtml(tr("<u><b>Logotipul organizației</b></u> - folosiți logotipul organizației.<br>"
                                 "Atașați logotipul organizației în setările utilizatorului pentru a prezenta în formele de tipar a documentelor și în rapoarte."));

    if (current_step == 4)
        ui->tip_text->setHtml(tr("<u><b>Ștampila organizației</b></u> - folosiți ștampila organizației.<br>"
                                 "Deschideți catalogul \"Centre medicale\" și găsiți organizația -> tabul \"Ștampila\" - atașați ștampila, "
                                 "pentru a prezenta în formele de tipar a documentelor și în rapoarte."));

    if (current_step == 5)
        ui->tip_text->setHtml(tr("<u><b>Ștampila și semnătura doctorului</b></u> - folosiți ștampila sau semnătura doctorului pentru a prezenta în formele "
                                 "de tipar a documentelor și în rapoarte.<br>"
                                 "Deschideți catalogul \"Doctori\" și alegeți persoana -> tabul \"Ștampila, semnătura\" - atașați ștampila sau semnărura."));

}
