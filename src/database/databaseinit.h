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
    bool run(QWidget *parent, Callable task)
    {
        QProgressDialog dialog(tr("Se creează schema bazei de date..."),
                               QString(), 0, 0, parent);
        dialog.setWindowModality(Qt::ApplicationModal);
        dialog.setCancelButton(nullptr);
        dialog.setMinimumDuration(0);

        QFutureWatcher<bool> watcher;
        connect(&watcher, &QFutureWatcher<bool>::finished,
                &dialog, &QProgressDialog::accept);
        watcher.setFuture(QtConcurrent::run(task));
        dialog.exec();
        watcher.waitForFinished();
        return watcher.result();
    }
};

#endif // DATABASEINIT_H
