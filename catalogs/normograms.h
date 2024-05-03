#ifndef NORMOGRAMS_H
#define NORMOGRAMS_H

#include "models/variantmaptablemodel.h"
#include <QDialog>
#include <QStyleFactory>
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

private:
    Ui::Normograms *ui;

    enum IndexPage {
        page_nt             = 0,
        page_bn             = 1,
        page_index_amniotic = 2
    };

    DataBase *db;
    VariantMapTableModel *model_nt;
    VariantMapTableModel *model_bn;
    VariantMapTableModel *model_indexAmniotic;
    QStyle* style_fusion = QStyleFactory::create("Fusion");
};

#endif // NORMOGRAMS_H
