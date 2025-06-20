#ifndef DATABASEINIT_H
#define DATABASEINIT_H

#include <QFutureWatcher>
#include <QObject>
#include <QProgressDialog>
#include <QtConcurrent>

class DatabaseInit : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseInit(QObject *parent = nullptr)
        : QObject(parent)
    {}

    template<typename Callable>
    void runAsync(QWidget *parent, Callable task, std::function<void()> onFinished)
    {
        QProgressDialog *dlg = new QProgressDialog(tr("Se creează schema bazei de date..."),
                                                   QString(),
                                                   0,
                                                   0,
                                                   parent);
        dlg->setWindowModality(Qt::ApplicationModal);
        dlg->setCancelButton(nullptr);
        dlg->setMinimumDuration(0);
        dlg->show();

        auto watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished, this, [=]() {
            dlg->close();
            dlg->deleteLater();
            watcher->deleteLater();
            if (onFinished)
                onFinished();
        });
        watcher->setFuture(QtConcurrent::run(task));
    }
};

#endif // DATABASEINIT_H
