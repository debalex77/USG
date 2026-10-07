// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Modified in 2026 for the USG QSQLCIPHER driver.

#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "qsql_sqlcipher_p.h"
#include "qsql_sqlcipher_vfs_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

class QSQLCipherDriverPlugin : public QSqlDriverPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlcipher.json")

public:
    QSQLCipherDriverPlugin();

    QSqlDriver* create(const QString &) override;
};

QSQLCipherDriverPlugin::QSQLCipherDriverPlugin()
    : QSqlDriverPlugin()
{
    register_qt_vfs_sqlcipher();
}

QSqlDriver* QSQLCipherDriverPlugin::create(const QString &name)
{
    if (name == "QSQLCIPHER"_L1) {
        QSQLCipherDriver* driver = new QSQLCipherDriver();
        return driver;
    }

    return nullptr;
}

QT_END_NAMESPACE

#include "smain.moc"
