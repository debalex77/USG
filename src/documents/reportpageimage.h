#ifndef REPORTPAGEIMAGE_H
#define REPORTPAGEIMAGE_H

#include <QPlainTextEdit>
#include <QToolButton>
#include <QWidget>
#include <QLabel>
#include <vector>

#include <documents/reportpagebase.h>

#include <data/database.h>

#include <customs/toolbarcustom.h>

namespace Ui {
class ReportPageImage;
}

class ReportPageImage : public ReportPageBase
{
    Q_OBJECT

public:
    explicit ReportPageImage(DataBase &db,
                             QSqlDatabase &currentDB,
                             QWidget *parent = nullptr);
    ~ReportPageImage();

    bool loadData(int idReport) override;
    bool saveData(int idReport) override;
    bool saveData(int idReport, const QSqlDatabase &database, const QString &table);

    QString concluzionText() const override;
    ReportSections::ReportSystem system() const override;

private slots:
    void onAddImage();
    void onEditImage();
    void onDeleteImage();
    void onSaveImage();

private:
    void initRequiredStructure();

    void loadFile(const QString &fileName);

    bool setPixmapBase64(QLabel* label,
                         const QByteArray& base64,
                         const QSize& targetSize);

    bool existDocument(const int idReport);

private:
    Ui::ReportPageImage *ui;
    DataBase &m_db;
    QSqlDatabase m_currentDB;

    ToolBarCustom *toolBar;
    QString toolButtonStyleForIcon;

    enum IndexNumImage
    {
        n_image1 = 1,
        n_image2 = 2,
        n_image3 = 3,
        n_image4 = 4,
        n_image5 = 5
    };

    int countImages = 0;

    //---------------------------------------------------
    /** structura atasarea/eliminarea imaginilor */
    struct StructImg {
        QLabel         *label_img;
        QPlainTextEdit *comment_img;
        IndexNumImage   nr_img;
        QByteArray      bArray;
    };
    std::vector<StructImg> rows_items_images;
};

#endif // REPORTPAGEIMAGE_H
