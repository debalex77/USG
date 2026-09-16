#ifndef NORMOGRAMS_H
#define NORMOGRAMS_H

#include "models/variantmaptablemodel.h"
#include <QDialog>
#include <data/database.h>

namespace Ui {
class Normograms;
}

class Normograms : public QDialog
{
    Q_OBJECT

public:
    explicit Normograms(QWidget *parent = nullptr);
    ~Normograms();

private:
    void setModels();

private slots:
    void initConnections();
    void updateStyleBtnPressed();

    void click_btn_NT();
    void click_btn_BN();
    void click_btn_indexAmniotic();
    void click_btn_uterine();
    void click_btn_umbelicale();

private:
    Ui::Normograms *ui;

    enum IndexPage {
        page_nt             = 0,
        page_bn             = 1,
        page_index_amniotic = 2,
        page_uterine        = 3,
        page_umbelicale     = 4
    };

    DataBase *db;
    VariantMapTableModel *model_nt;
    VariantMapTableModel *model_bn;
    VariantMapTableModel *model_indexAmniotic;
    VariantMapTableModel *model_doppler_uterine;
    VariantMapTableModel *model_doppler_umbelicale;
};

#endif // NORMOGRAMS_H
