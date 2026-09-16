#ifndef REPORTPAGEVIDEO_H
#define REPORTPAGEVIDEO_H

#include <QWidget>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSplitter>
#include <QToolButton>
#include <QVideoWidget>
#include <QSlider>

#include <customs/toolbarcustom.h>
#include <data/database.h>
#include <documents/reportpagebase.h>

namespace Ui {
class ReportPageVideo;
}

class ReportPageVideo : public ReportPageBase
{
    Q_OBJECT

public:
    explicit ReportPageVideo(DataBase &db,
                             QSqlDatabase &currentDB,
                             QWidget *parent = nullptr);
    ~ReportPageVideo();

    bool loadData(int idReport) override;
    bool saveData(int idReport) override;

    QString concluzionText() const override;
    ReportSections::ReportSystem system() const override;

private slots:
    void clickPlayVideo();
    void setPositionSlider(int milisecund);
    void mediaStateChanged(QMediaPlayer::PlaybackState state);
    void positionChanged(qint64 progress);
    void durationChanged(qint64 duration);
    void handleError();

    void onAddVideo();
    void onEditVideo();
    void onDeleteVideo();

    void onCurrentItemChanged(QListWidgetItem *current,
                              QListWidgetItem *previous);

private:
    void buildForm();

    void setUrl(const QUrl &url);
    QUrl ensureVideoInStorage(const QUrl &url);

    void updateDurationInfo(qint64 currentInfo);

private:
    Ui::ReportPageVideo *ui;
    DataBase &m_db;
    QSqlDatabase m_currentDB;

    ToolBarCustom *toolBar;
    QString toolButtonStyleForIcon;

    QLabel *titlePlayer   = nullptr;
    QLabel *labelDuration = nullptr;
    QLabel *errorLabel    = nullptr;

    qint64 m_duration = 0;
    int m_count_video = 0;

    QPushButton  *playButton = nullptr;

    QMediaPlayer *player         = nullptr;
    QVideoWidget *videoWidget    = nullptr;
    QSlider      *positionSlider = nullptr;
};

#endif // REPORTPAGEVIDEO_H
