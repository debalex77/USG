#include "asistanttipapp.h"
#include "ui_asistanttipapp.h"

AsistantTipApp::AsistantTipApp(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AsistantTipApp)
{
    ui->setupUi(this);

    setWindowTitle(tr("Asistentul sfaturilor aplicației."));
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
}

AsistantTipApp::~AsistantTipApp()
{
    delete ui;
}

void AsistantTipApp::stepNext()
{
    current_step += 1;
    setTipText();
}

void AsistantTipApp::stepPreview()
{
    current_step -= 1;
    setTipText();
}

void AsistantTipApp::setTipText()
{
    if (current_step == 1)
        ui->tip_text->setHtml(tr("<u><b>Preferințele utilizatorului</b></u> - configurarea setărilor utilizatorului:<br>"
                              "<li>Setările generale - setarea aplicației implicite, logotipul etc.</li>"
                              "<li>Lansarea/închiderea - setările la lansarea sau închiderea aplicației.</li>"
                              "<li>Setările documentelor - setări pentru documente.</li>"
                              "<li>Setările neredactabile - setările informative.</li>"));

    if (current_step == 2)
        ui->tip_text->setHtml(tr("<u><b>Jurnalul de logare</b></u> - vizualizați jurnalul de logare.<br>"
                              "Deschideți Setările aplicației -> Tabul 'Fișiere de logare' -> alegeți fișierul pentru a vizualiza "
                              "jurnalul."));
}
