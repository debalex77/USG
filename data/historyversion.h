#ifndef HISTORYVERSION_H
#define HISTORYVERSION_H

#include <QDialog>
#include <QWebEngineView>
#include <QFrame>

namespace Ui {
class HistoryVersion;
}

class HistoryVersion : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryVersion(QWidget *parent = nullptr);
    ~HistoryVersion();

private:
    Ui::HistoryVersion *ui;
};

#endif // HISTORYVERSION_H
