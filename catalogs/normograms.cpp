#include "normograms.h"
#include "ui_normograms.h"
#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
#include <QDesktopWidget>
#else
#include <QScreen>
#endif

Normograms::Normograms(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Normograms)
{
    ui->setupUi(this);

    setWindowTitle(tr("Normograme"));

    setModels();

    initConnections();
    updateStyleBtnPressed();

    // initial setam normograma NT
    ui->stackedWidget->setCurrentIndex(page_nt);
    ui->btnNT->setFocus(); // focusul pe btn NT

    // ********************************************************
    // resize and move windows

#if (QT_VERSION < QT_VERSION_CHECK(5, 15, 1))
    QDesktopWidget *desktop = QApplication::desktop();

    int screenWidth = desktop->screenGeometry().width();
    int screenHeight = desktop->screenGeometry().height();
#else
    QScreen *screen = QGuiApplication::primaryScreen();

    int screenWidth = screen->geometry().width();
    int screenHeight = screen->geometry().height();
#endif

    this->resize(720, 480);
    int x = (screenWidth / 2) - (width() / 2);//*0.1;
    int y = (screenHeight / 2) - (height() / 2);//*0.1;
    move(x, y);

    // ********************************************************
    // resize frame btn

    ui->frame_tables->resize(500, ui->frame_tables->height());

    // ********************************************************
    // style for OS Windows

#if defined(Q_OS_WIN)
    ui->frame_btn->setStyle(style_fusion);
    ui->frame_tables->setStyle(style_fusion);
#endif
}

Normograms::~Normograms()
{
    delete model_nt;
    delete model_bn;
    delete model_indexAmniotic;
    delete model_doppler_uterine;
    delete model_doppler_umbelicale;
    delete db;
    delete ui;
}

void Normograms::setModels()
{
    db = new DataBase(this);
    QSqlQuery qry;
    QVariantMap norm_data;

    // --- NT

    model_nt = new VariantMapTableModel(this);
    model_nt->setTypeNormograms(VariantMapTableModel::TypeNormograms::NT_BN);
    model_nt->registerColumn(new SimpleColumn("id"));
    model_nt->registerColumn(new SimpleColumn("crl"));
    model_nt->registerColumn(new SimpleColumn("5_centile"));
    model_nt->registerColumn(new SimpleColumn("50_centile"));
    model_nt->registerColumn(new SimpleColumn("95_centile"));

    qry.prepare("SELECT * FROM normograms WHERE name = :name;");
    qry.bindValue(":name", "normograma_nt");
    if (qry.exec() && qry.next()){
        while (qry.next()){
            QSqlRecord rec = qry.record();
            norm_data.insert("id",         qry.value(rec.indexOf("id")).toInt());
            norm_data.insert("crl",        qry.value(rec.indexOf("crl")).toString());
            norm_data.insert("5_centile",  qry.value(rec.indexOf("5_centile")).toDouble());
            norm_data.insert("50_centile", qry.value(rec.indexOf("50_centile")).toDouble());
            norm_data.insert("95_centile", qry.value(rec.indexOf("95_centile")).toDouble());
            model_nt->addRow(norm_data);
        }
    } else {
        qCritical(logCritical()) << "Solicitarea nereusita tabela 'normograms' - " + qry.lastError().text();
    }
    ui->tableNT->setModel(model_nt);
    ui->tableNT->hideColumn(0);

    // --- BN
    norm_data.clear();
    model_bn = new VariantMapTableModel(this);
    model_bn->setTypeNormograms(VariantMapTableModel::TypeNormograms::NT_BN);
    model_bn->registerColumn(new SimpleColumn("id"));
    model_bn->registerColumn(new SimpleColumn("crl"));
    model_bn->registerColumn(new SimpleColumn("5_centile"));
    model_bn->registerColumn(new SimpleColumn("50_centile"));
    model_bn->registerColumn(new SimpleColumn("95_centile"));

    qry.prepare("SELECT * FROM normograms WHERE name = :name;");
    qry.bindValue(":name", "normograma_bn");
    if (qry.exec()){
        while (qry.next()){
            QSqlRecord rec = qry.record();
            norm_data.insert("id",         qry.value(rec.indexOf("id")).toInt());
            norm_data.insert("crl",        qry.value(rec.indexOf("crl")).toString());
            norm_data.insert("5_centile",  qry.value(rec.indexOf("5_centile")).toDouble());
            norm_data.insert("50_centile", qry.value(rec.indexOf("50_centile")).toDouble());
            norm_data.insert("95_centile", qry.value(rec.indexOf("95_centile")).toDouble());
            model_bn->addRow(norm_data);
        }
    } else {
        qCritical(logCritical()) << "Solicitarea nereusita tabela 'normograms' - " + qry.lastError().text();
    }
    ui->tableBN->setModel(model_bn);
    ui->tableBN->hideColumn(0);

    // --- index amniotic
    norm_data.clear();
    model_indexAmniotic = new VariantMapTableModel(this);
    model_indexAmniotic->setTypeNormograms(VariantMapTableModel::TypeNormograms::INDEX_AMNIOTIC);
    model_indexAmniotic->registerColumn(new SimpleColumn("id"));
    model_indexAmniotic->registerColumn(new SimpleColumn("crl"));
    model_indexAmniotic->registerColumn(new SimpleColumn("5_centile"));
    model_indexAmniotic->registerColumn(new SimpleColumn("50_centile"));
    model_indexAmniotic->registerColumn(new SimpleColumn("95_centile"));

    qry.prepare("SELECT * FROM normograms WHERE name = :name;");
    qry.bindValue(":name", "index_amniotic");
    if (qry.exec()){
        while (qry.next()){
            QSqlRecord rec = qry.record();
            norm_data.insert("id",         qry.value(rec.indexOf("id")).toInt());
            norm_data.insert("crl",        qry.value(rec.indexOf("crl")).toString());
            norm_data.insert("5_centile",  qry.value(rec.indexOf("5_centile")).toDouble());
            norm_data.insert("50_centile", qry.value(rec.indexOf("50_centile")).toDouble());
            norm_data.insert("95_centile", qry.value(rec.indexOf("95_centile")).toDouble());
            model_indexAmniotic->addRow(norm_data);
        }
    } else {
        qCritical(logCritical()) << "Solicitarea nereusita tabela 'normograms' - " + qry.lastError().text();
    }
    ui->tableIndexAmniotic->setModel(model_indexAmniotic);
    ui->tableIndexAmniotic->hideColumn(0);

    // --- a.uterine
    norm_data.clear();
    model_doppler_uterine = new VariantMapTableModel(this);
    model_doppler_uterine->setTypeNormograms(VariantMapTableModel::TypeNormograms::DOPPLER);
    model_doppler_uterine->registerColumn(new SimpleColumn("id"));
    model_doppler_uterine->registerColumn(new SimpleColumn("crl"));
    model_doppler_uterine->registerColumn(new SimpleColumn("5_centile"));
    model_doppler_uterine->registerColumn(new SimpleColumn("50_centile"));
    model_doppler_uterine->registerColumn(new SimpleColumn("95_centile"));
    qry.prepare("SELECT * FROM normograms WHERE name = :name;");
    qry.bindValue(":name", "uterine_PI");
    if (qry.exec()){
        while (qry.next()){
            QSqlRecord rec = qry.record();
            norm_data.insert("id",         qry.value(rec.indexOf("id")).toInt());
            norm_data.insert("crl",        qry.value(rec.indexOf("crl")).toString());
            norm_data.insert("5_centile",  qry.value(rec.indexOf("5_centile")).toDouble());
            norm_data.insert("50_centile", qry.value(rec.indexOf("50_centile")).toDouble());
            norm_data.insert("95_centile", qry.value(rec.indexOf("95_centile")).toDouble());
            model_doppler_uterine->addRow(norm_data);
        }
    } else {
        qCritical(logCritical()) << "Solicitarea nereusita tabela 'normograms' - " + qry.lastError().text();
    }
    ui->tableUterine->setModel(model_doppler_uterine);
    ui->tableUterine->hideColumn(0);

    // --- a.umbelicale
    norm_data.clear();
    model_doppler_umbelicale = new VariantMapTableModel(this);
    model_doppler_umbelicale->setTypeNormograms(VariantMapTableModel::TypeNormograms::DOPPLER);
    model_doppler_umbelicale->registerColumn(new SimpleColumn("id"));
    model_doppler_umbelicale->registerColumn(new SimpleColumn("crl"));
    model_doppler_umbelicale->registerColumn(new SimpleColumn("5_centile"));
    model_doppler_umbelicale->registerColumn(new SimpleColumn("50_centile"));
    model_doppler_umbelicale->registerColumn(new SimpleColumn("95_centile"));
    qry.prepare("SELECT * FROM normograms WHERE name = :name;");
    qry.bindValue(":name", "umbilical_PI");
    if (qry.exec()){
        while (qry.next()){
            QSqlRecord rec = qry.record();
            norm_data.insert("id",         qry.value(rec.indexOf("id")).toInt());
            norm_data.insert("crl",        qry.value(rec.indexOf("crl")).toString());
            norm_data.insert("5_centile",  qry.value(rec.indexOf("5_centile")).toDouble());
            norm_data.insert("50_centile", qry.value(rec.indexOf("50_centile")).toDouble());
            norm_data.insert("95_centile", qry.value(rec.indexOf("95_centile")).toDouble());
            model_doppler_umbelicale->addRow(norm_data);
        }
    } else {
        qCritical(logCritical()) << "Solicitarea nereusita tabela 'normograms' - " + qry.lastError().text();
    }
    ui->tableUmbelical->setModel(model_doppler_umbelicale);
    ui->tableUmbelical->hideColumn(0);

}

// *******************************************************************
// **************** INITIEREA CONEXIUNILOR SI PROCESAREA LOR *********

void Normograms::initConnections()
{
    connect(ui->btnNT, &QAbstractButton::clicked, this, &Normograms::click_btn_NT);
    connect(ui->btnBN, &QAbstractButton::clicked, this, &Normograms::click_btn_BN);
    connect(ui->btnIndexAmniotic, &QAbstractButton::clicked, this, &Normograms::click_btn_indexAmniotic);
    connect(ui->btnDopplerUterine, &QAbstractButton::clicked, this, &Normograms::click_btn_uterine);
    connect(ui->btnDopplerUmbelical, &QAbstractButton::clicked, this, &Normograms::click_btn_umbelicale);
}

void Normograms::updateStyleBtnPressed()
{
    const QString style_pressed = "background: 0px #C2C2C3; "
                                "border: 1px inset blue; "
                                "border-color: navy;";

    const QString style_unpressed = "background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #f6f7fa, stop: 1 #dadbde); "
                                  "border: 0px #8f8f91;";

    if (ui->stackedWidget->currentIndex() == page_nt){
        ui->btnNT->setStyleSheet(style_pressed);
    } else {
        ui->btnNT->setStyleSheet(style_unpressed);
    }

    if (ui->stackedWidget->currentIndex() == page_bn){
        ui->btnBN->setStyleSheet(style_pressed);
    } else {
        ui->btnBN->setStyleSheet(style_unpressed);
    }

    if (ui->stackedWidget->currentIndex() == page_index_amniotic){
        ui->btnIndexAmniotic->setStyleSheet(style_pressed);
    } else {
        ui->btnIndexAmniotic->setStyleSheet(style_unpressed);
    }

    if (ui->stackedWidget->currentIndex() == page_uterine){
        ui->btnDopplerUterine->setStyleSheet(style_pressed);
    } else {
        ui->btnDopplerUterine->setStyleSheet(style_unpressed);
    }

    if (ui->stackedWidget->currentIndex() == page_umbelicale){
        ui->btnDopplerUmbelical->setStyleSheet(style_pressed);
    } else {
        ui->btnDopplerUmbelical->setStyleSheet(style_unpressed);
    }
}

void Normograms::click_btn_NT()
{
    ui->stackedWidget->setCurrentIndex(page_nt);
    updateStyleBtnPressed();
}

void Normograms::click_btn_BN()
{
    ui->stackedWidget->setCurrentIndex(page_bn);
    updateStyleBtnPressed();
}

void Normograms::click_btn_indexAmniotic()
{
    ui->stackedWidget->setCurrentIndex(page_index_amniotic);
    updateStyleBtnPressed();
}

void Normograms::click_btn_uterine()
{
    ui->stackedWidget->setCurrentIndex(page_uterine);
    updateStyleBtnPressed();
}

void Normograms::click_btn_umbelicale()
{
    ui->stackedWidget->setCurrentIndex(page_umbelicale);
    updateStyleBtnPressed();
}
