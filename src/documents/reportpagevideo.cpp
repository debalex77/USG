#include "reportpagevideo.h"
#include "ui_reportpagevideo.h"

#include <QFileDialog>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTime>
#include <QUuid>
#include <QSqlQuery>
#include <QSqlError>
#include <documents/reportdialog.h>

ReportPageVideo::ReportPageVideo(DataBase &db, QSqlDatabase &currentDB, QWidget *parent)
    : ReportPageBase(parent)
    , ui(new Ui::ReportPageVideo)
    , m_db(db)
    , m_currentDB(currentDB)
    , toolBar(new ToolBarCustom(this, ToolBarCustom::AddEditDelete))
    , toolButtonStyleForIcon(m_db.toolButtonStyleForIcon())
{
    ui->setupUi(this);
    buildForm();
}

ReportPageVideo::~ReportPageVideo()
{
    delete ui;
}

bool ReportPageVideo::loadData(int idReport)
{
    if (idReport <= 0)
        return false;

    ui->playList->clear();
    m_count_video = 0;

    QSqlQuery q(m_currentDB);
    const QString selectSql = QStringLiteral(
        "SELECT * FROM reportVideo WHERE id_reportEcho = %1").arg(idReport);
    if(!q.exec(selectSql)) {
        qWarning(logWarning()).noquote()
        << "ReportPageVideo error:" << q.lastError().text()
        << "\nLast query:" << q.lastQuery();
        return false;
    }

    while (q.next()) {
        const QString relativePath = q.value("relative_path").toString();
        const QString fileName     = q.value("file_name").toString();

        auto *item = new QListWidgetItem(fileName);
        item->setData(Qt::UserRole, relativePath);
        item->setToolTip(relativePath);
        ui->playList->addItem(item);

        emit countVideoBtn(++m_count_video);
    }

    return true;
}

bool ReportPageVideo::saveData(const int idReport)
{
    if (idReport <= 0)
        return false;

    int id_order = 0;
    if (ReportDialog *report = qobject_cast<ReportDialog *>(window())) {
        const int idOrder = report->getIdOrder();
        if (idOrder > 0)
            id_order = idOrder;
    }

    QSqlQuery q(m_currentDB);

    const QString deleteSql = QStringLiteral(
        "DELETE FROM reportVideo WHERE id_reportEcho = %1").arg(idReport);
    if (!q.exec(deleteSql)) {
        qWarning(logWarning()) << "Eroare DELETE reportVideo:" << q.lastError().text();
        return false;
    }

    for (int i = 0; i < ui->playList->count(); ++i) {
        QListWidgetItem *item = ui->playList->item(i);
        if (!item)
            continue;

        QSqlQuery ins(m_currentDB);
        ins.prepare(R"(
            INSERT INTO reportVideo (
                id_orderEcho,
                id_reportEcho,
                relative_path,
                file_name,
                comment,
                uuid
            ) VALUES (
                :id_orderEcho,
                :id_reportEcho,
                :relative_path,
                :file_name,
                :comment,
                :uuid
            )
        )");
        ins.bindValue(":id_orderEcho",  id_order);
        ins.bindValue(":id_reportEcho", idReport);
        ins.bindValue(":relative_path", item->data(Qt::UserRole).toString());
        ins.bindValue(":file_name",     item->text());
        ins.bindValue(":comment",       QVariant());
        ins.bindValue(":uuid",          QUuid::createUuid().toRfc4122());

        if (!ins.exec()) {
            qWarning(logWarning()) << "Eroare INSERT reportVideo:" << ins.lastError().text();
            return false;
        }
    }
    return true;
}

QString ReportPageVideo::concluzionText() const
{
    return QString();
}

ReportSections::ReportSystem ReportPageVideo::system() const
{
    return ReportSections::ReportSystem::Video;
}

void ReportPageVideo::clickPlayVideo()
{
    if (ui->playList->count() == 0 || !player)
        return;

    switch (player->playbackState()) {
    case QMediaPlayer::PlayingState:
        player->pause();
        break;
    default:
        player->play();
        break;
    }
}

void ReportPageVideo::setPositionSlider(int milisecund)
{
    if (ui->playList->count() == 0 || !player)
        return;

    player->setPosition(milisecund);
}

void ReportPageVideo::mediaStateChanged(QMediaPlayer::PlaybackState state)
{
    if (!playButton)
        return;

    switch(state) {
    case QMediaPlayer::PlayingState:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void ReportPageVideo::positionChanged(qint64 progress)
{
    if (!positionSlider)
        return;

    positionSlider->setValue(progress);
    updateDurationInfo(progress / 1000);
}

void ReportPageVideo::durationChanged(qint64 duration)
{
    m_duration = duration / 1000;
    positionSlider->setRange(0, duration);
}

void ReportPageVideo::handleError()
{
    if (playButton)
        playButton->setEnabled(false);
    const QString errorString = player->errorString();
    QString message = "Error: ";
    if (errorString.isEmpty())
        message += " #" + QString::number(int(player->error()));
    else
        message += errorString;
    errorLabel->setText(message);
}

void ReportPageVideo::onAddVideo()
{
    if (ReportDialog *report = qobject_cast<ReportDialog *>(window())) {
        if (report->documentModified() ||
            report->documentIsNew()) {
            CustomMessage msg(this);
            msg.setWindowTitle(QGuiApplication::applicationDisplayName());
            msg.setTextTitle(tr("Atașarea video"));
            msg.setDetailedText(tr("Video poate fi atașată după validarea documentului."));
            msg.exec();
            return;
        }
    }

    const QUrl sourceUrl = QFileDialog::getOpenFileUrl(
        this,
        tr("Selectează video"),
        QUrl::fromLocalFile(QDir::homePath()),
        tr("Video files (*.mp4 *.avi *.mkv *.mov)")
        );

    if (!sourceUrl.isValid() || sourceUrl.isEmpty())
        return;

    const QUrl finalUrl = ensureVideoInStorage(sourceUrl);
    if (!finalUrl.isValid())
        return;

    setUrl(finalUrl);
}

void ReportPageVideo::onEditVideo()
{
    QListWidgetItem *item = ui->playList->currentItem();
    if (!item)
        return;

    const QString oldRelativePath = item->data(Qt::UserRole).toString();
    if (oldRelativePath.isEmpty())
        return;

    const QUrl newUrl = QFileDialog::getOpenFileUrl(
        this,
        tr("Selectează video nou"),
        QUrl::fromLocalFile(QDir::homePath()),
        tr("Video files (*.mp4 *.avi *.mkv *.mov)")
        );

    if (!newUrl.isValid() || newUrl.isEmpty())
        return;

    const QUrl finalUrl = ensureVideoInStorage(newUrl);
    if (!finalUrl.isValid() || !finalUrl.isLocalFile())
        return;

    const QString fullPath = QDir::cleanPath(finalUrl.toLocalFile());
    const QString basePath = QDir::cleanPath(globals().pathDirectoryVideo);

    QString newRelativePath;
    if (!fullPath.startsWith(basePath + QDir::separator()))
        return;

    newRelativePath = fullPath.mid(basePath.length() + 1);

    const QString newFileName = QFileInfo(fullPath).fileName();

    item->setText(newFileName);
    item->setData(Qt::UserRole, newRelativePath);
    item->setToolTip(newRelativePath);

    const QString oldFullPath = QDir(globals().pathDirectoryVideo).filePath(oldRelativePath);
    if (oldRelativePath != newRelativePath) {
        const QString oldFullPath = QDir(globals().pathDirectoryVideo).filePath(oldRelativePath);
        QFile::remove(oldFullPath);
    }
}

void ReportPageVideo::onDeleteVideo()
{
    QListWidgetItem *item = ui->playList->currentItem();
    if (!item)
        return;

    const QString relativePath = item->data(Qt::UserRole).toString();
    if (relativePath.isEmpty())
        return;

    if (QMessageBox::question(this,
                              tr("Ștergere video"),
                              tr("Sigur doriți să ștergeți video-ul selectat?"),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    const QString fullPath = QDir(globals().pathDirectoryVideo).filePath(relativePath);
    const QUrl fileUrl = QUrl::fromLocalFile(fullPath);

    if (player && player->source() == fileUrl) {
        player->stop();
        player->setSource(QUrl());
    }

    const int row = ui->playList->row(item);
    if (row < 0)
        return;

    delete ui->playList->takeItem(row);

    if (QFile::exists(fullPath) && !QFile::remove(fullPath)) {
        qWarning(logWarning()) << "Nu s-a putut șterge fișierul video:" << fullPath;
    }

    m_count_video = qMax(0, ui->playList->count());
    emit countVideoBtn(m_count_video);
}

void ReportPageVideo::onCurrentItemChanged(QListWidgetItem *current,
                                           QListWidgetItem *previous)
{
    Q_UNUSED(previous)

    if (!current || !player)
        return;

    const QString relativePath = current->data(Qt::UserRole).toString();
    if (relativePath.isEmpty())
        return;

    const QString fullPath = QDir(globals().pathDirectoryVideo).filePath(relativePath);

    if (player->source() == QUrl::fromLocalFile(fullPath))
        return;

    if (!QFileInfo::exists(fullPath)) {
        if (errorLabel)
            errorLabel->setText(tr("Fișierul video nu există: %1").arg(fullPath));
        return;
    }

    player->stop();
    player->setSource(QUrl::fromLocalFile(fullPath));

    if (!playButton->isEnabled())
        playButton->setEnabled(true);

    titlePlayer->setText("Se rulează - " + current->text() + "\n" + fullPath);
}

void ReportPageVideo::buildForm()
{
    toolBar->setStyles(toolButtonStyleForIcon);
    ui->layout_toolBar->addWidget(toolBar);
    ui->layout_toolBar->addStretch();

    connect(toolBar, &ToolBarCustom::addDoc,
            this, &ReportPageVideo::onAddVideo, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::editDoc,
            this, &ReportPageVideo::onEditVideo, Qt::UniqueConnection);
    connect(toolBar, &ToolBarCustom::deleteDoc,
            this, &ReportPageVideo::onDeleteVideo, Qt::UniqueConnection);

    connect(ui->playList, &QListWidget::currentItemChanged,
            this, &ReportPageVideo::onCurrentItemChanged, Qt::UniqueConnection);

    //------------------------------------------------------

    QFont font;
    font.setBold(true);

    //--- title player
    titlePlayer = new QLabel(ui->groupBox_player);
    titlePlayer->setMaximumHeight(40);
    titlePlayer->setFont(font);
    titlePlayer->setStyleSheet(QString::fromUtf8("color: rgb(198, 70, 0);"));
    titlePlayer->setAlignment(Qt::AlignCenter);
    ui->gridLayout_player->addWidget(titlePlayer, 0, 0, 1, 3);

    //--- player & videoWidget
    player = new QMediaPlayer(ui->groupBox_player);
    videoWidget = new QVideoWidget(ui->groupBox_player);
    player->setVideoOutput(videoWidget);
    ui->gridLayout_player->addWidget(videoWidget, 1, 0, 1, 3);

    //--- playButton
    playButton = new QPushButton(ui->groupBox_player);
    playButton->setEnabled(false);
    playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->gridLayout_player->addWidget(playButton, 2, 0, 1, 1);

    //--- positionSlider
    positionSlider = new QSlider(ui->groupBox_player);
    positionSlider->setOrientation(Qt::Horizontal);
    ui->gridLayout_player->addWidget(positionSlider, 2, 1, 1, 1);

    //--- labelDuration
    labelDuration = new QLabel(ui->groupBox_player);
    ui->gridLayout_player->addWidget(labelDuration, 2, 2, 1, 1);

    //--- labelError
    errorLabel = new QLabel(ui->groupBox_player);
    errorLabel->setMaximumHeight(40);
    ui->gridLayout_player->addWidget(errorLabel, 3, 0, 1, 3);

    //--- connect
    connect(playButton, &QAbstractButton::clicked,
            this, &ReportPageVideo::clickPlayVideo, Qt::UniqueConnection);
    connect(positionSlider, &QAbstractSlider::sliderMoved,
            this, &ReportPageVideo::setPositionSlider, Qt::UniqueConnection);
    connect(player, &QMediaPlayer::positionChanged,
            this, &ReportPageVideo::positionChanged, Qt::UniqueConnection);
    connect(player, &QMediaPlayer::playbackStateChanged,
            this, &ReportPageVideo::mediaStateChanged, Qt::UniqueConnection);
    connect(player, &QMediaPlayer::durationChanged,
            this, &ReportPageVideo::durationChanged, Qt::UniqueConnection);
    connect(player, &QMediaPlayer::errorChanged,
            this, &ReportPageVideo::handleError, Qt::UniqueConnection);

    videoWidget->show();

    ui->splitter->setSizes({240, 9999}); // initial
}

void ReportPageVideo::setUrl(const QUrl &url)
{
    if (!url.isValid() || !url.isLocalFile())
        return;

    const QString fullPath = QDir::cleanPath(url.toLocalFile());
    const QString basePath = QDir::cleanPath(globals().pathDirectoryVideo);

    QString relativePath;
    if (!fullPath.startsWith(basePath + QDir::separator()))
        return;
    relativePath = fullPath.mid(basePath.length() + 1);

    const QString fileName = QFileInfo(fullPath).fileName();

    for (int i = 0; i < ui->playList->count(); ++i) {
        auto *it = ui->playList->item(i);
        if (it && it->data(Qt::UserRole).toString() == relativePath)
            return;
    }

    QListWidgetItem *item = new QListWidgetItem();
    item->setText(fileName);
    item->setData(Qt::UserRole, relativePath);
    item->setToolTip(relativePath);

    ui->playList->addItem(item);
    ui->playList->setCurrentRow(ui->playList->count() - 1);

    emit countVideoBtn(++m_count_video);
}

QUrl ReportPageVideo::ensureVideoInStorage(const QUrl &url)
{
    if (!url.isValid() || !url.isLocalFile())
        return {};

    const QString sourceFilePath = url.toLocalFile();
    const QFileInfo sourceInfo(sourceFilePath);

    if (!sourceInfo.exists() || !sourceInfo.isFile())
        return {};

    QString reportDirName;

    if (ReportDialog *report = qobject_cast<ReportDialog *>(window())) {
        const int idReport = report->getId();
        if (idReport > 0)
            reportDirName = QString("report_%1").arg(idReport);
        else
            return {};
    }

    if (reportDirName.isEmpty())
        return {};

    QDir baseDir(globals().pathDirectoryVideo);
    if (!baseDir.exists() && !baseDir.mkpath(".")) {
        qWarning(logWarning()) << "Nu s-a putut crea directorul de bază:"
                               << baseDir.absolutePath();
        return {};
    }

    const QString targetDirPath = baseDir.filePath(reportDirName);
    QDir targetDir(targetDirPath);

    if (!targetDir.exists() && !baseDir.mkpath(reportDirName)) {
        qWarning(logWarning()) << "Nu s-a putut crea directorul pentru report:"
                               << targetDirPath;
        return {};
    }

    const QString sourceAbsPath = QDir::cleanPath(sourceInfo.absoluteFilePath());
    const QString targetDirAbsPath = QDir::cleanPath(targetDir.absolutePath());

    // dacă fișierul este deja exact în directorul țintă, îl returnăm
    if (QDir::cleanPath(sourceInfo.absolutePath()) == targetDirAbsPath)
        return QUrl::fromLocalFile(sourceAbsPath);

    QString targetFileName = sourceInfo.fileName();
    QString targetFilePath = targetDir.filePath(targetFileName);

    if (QFile::exists(targetFilePath)) {
        const QString baseName = sourceInfo.completeBaseName();
        const QString suffix = sourceInfo.suffix();

        int counter = 1;
        do {
            if (suffix.isEmpty()) {
                targetFileName = QString("%1_%2")
                .arg(baseName)
                    .arg(counter++);
            } else {
                targetFileName = QString("%1_%2.%3")
                .arg(baseName)
                    .arg(counter++)
                    .arg(suffix);
            }

            targetFilePath = targetDir.filePath(targetFileName);
        } while (QFile::exists(targetFilePath));
    }

    if (!QFile::copy(sourceFilePath, targetFilePath)) {
        qWarning(logWarning()) << "Nu s-a putut copia fișierul:"
                               << sourceFilePath << "->" << targetFilePath;
        return {};
    }

    return QUrl::fromLocalFile(targetFilePath);
}

void ReportPageVideo::updateDurationInfo(qint64 currentInfo)
{
    QString tStr;
    if (currentInfo || m_duration) {
        QTime currentTime(currentInfo / 3600,
                          (currentInfo / 60) % 60,
                          currentInfo % 60);
        QTime totalTime((m_duration / 3600) % 60, (m_duration / 60) % 60,
                        m_duration % 60, (m_duration * 1000) % 1000);
        QString format = "mm:ss";
        if (m_duration > 3600)
            format = "hh:mm:ss";
        tStr = currentTime.toString(format) + " / " + totalTime.toString(format);
    }
    labelDuration->setText(tStr);
}
