#ifndef CLOUDSERVERCONFIG_H
#define CLOUDSERVERCONFIG_H

#include <QDialog>

namespace Ui {
class CloudServerConfig;
}

class CloudServerConfig : public QDialog
{
    Q_OBJECT

public:
    explicit CloudServerConfig(QWidget *parent = nullptr);
    ~CloudServerConfig();

private:
    Ui::CloudServerConfig *ui;
};

#endif // CLOUDSERVERCONFIG_H
