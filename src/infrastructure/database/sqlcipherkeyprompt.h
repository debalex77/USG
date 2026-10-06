#ifndef SQLCIPHER_KEY_PROMPT_H
#define SQLCIPHER_KEY_PROMPT_H
#include "common/maindatabaseconnectioncontext.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QLabel>
#include "ui/widgets/lineeditpassword.h"
#include <QPushButton>
#include <QRandomGenerator>
#include <QResizeEvent>
#include <QMessageBox>
#include <QSqlDatabase>

namespace SqlCipherKeyPrompt {
class KeyDialog final : public QDialog
{
public:
    using QDialog::QDialog;

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QDialog::resizeEvent(event);
        if (layout()) {
            // Wrapped text needs more vertical space when the window narrows.
            const int height = layout()->totalHeightForWidth(event->size().width());
            if (height > 0)
                setMinimumHeight(height);
        }
    }
};

inline bool ensure(bool encrypted, QWidget *parent = nullptr)
{
    if (encrypted && !QSqlDatabase::isDriverAvailable(QStringLiteral("QSQLCIPHER"))) {
        QMessageBox::critical(
            parent,
            QObject::tr("SQLCipher indisponibil"),
            QObject::tr("Pluginul bazei de date QSQLCIPHER nu este instalat pentru această "
                        "versiune a aplicației. Instalați pluginul compatibil sau dezactivați "
                        "SQLCipher în profilul bazei de date."));
        return false;
    }

    auto config = MainDatabaseConnectionContext::instance().data();
    if (!encrypted || !config.sqliteKey.isEmpty()) return true;
    config.sqliteKey = qEnvironmentVariable("USG_SQLCIPHER_KEY");
    if (config.sqliteKey.isEmpty()) {
        KeyDialog dialog(parent);
        dialog.setWindowTitle(QObject::tr("SQLCipher — cheia de criptare"));
        dialog.setMinimumWidth(340);
        auto *layout = new QVBoxLayout(&dialog);
        layout->setSizeConstraint(QLayout::SetNoConstraint);
        auto *message = new QHBoxLayout;
        auto *warning = new QLabel(&dialog);
        warning->setPixmap(QPixmap(QStringLiteral(":/img/oxygen/dialog-warning.png"))
                               .scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        warning->setFixedSize(48, 48);
        message->addWidget(warning, 0, Qt::AlignTop);
        auto *label = new QLabel(QObject::tr(
            "Introduceți cheia existentă pentru baza principală și baza de imagini.\n\n"
            "Pentru o bază nouă, puteți genera o cheie. Păstrați cheia într-un loc sigur: "
            "veți avea nevoie de ea la fiecare lansare."), &dialog);
        label->setTextFormat(Qt::PlainText);
        label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        label->setWordWrap(true);
        QSizePolicy textPolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        textPolicy.setHeightForWidth(true);
        label->setSizePolicy(textPolicy);
        message->addWidget(label, 1);
        layout->addLayout(message);
        auto *key = new LineEditPassword(&dialog);
        layout->addWidget(key);
        auto *generate = new QPushButton(QObject::tr("Generează cheie pentru o bază nouă"), &dialog);
        layout->addWidget(generate);
        QObject::connect(generate, &QPushButton::clicked, &dialog, [key] {
            QByteArray bytes(32, '\0');
            for (int i = 0; i < bytes.size(); ++i)
                bytes[i] = static_cast<char>(QRandomGenerator::system()->generate() & 0xff);
            key->setText(QString::fromLatin1(bytes.toHex()));
        });
        auto *buttons = new QDialogButtonBox(&dialog);
        auto *ok = buttons->addButton(QObject::tr("OK"), QDialogButtonBox::ActionRole);
        auto *cancel = buttons->addButton(QObject::tr("Anulează"), QDialogButtonBox::ActionRole);
        ok->setIcon(QIcon(QStringLiteral(":/img/btns/valide.png")));
        cancel->setIcon(QIcon(QStringLiteral(":/img/btns/close.png")));
        ok->setMinimumWidth(104);
        cancel->setMinimumWidth(104);
        const int buttonWidth = qMax(ok->sizeHint().width(), cancel->sizeHint().width());
        ok->setFixedWidth(qMax(104, buttonWidth));
        cancel->setFixedWidth(qMax(104, buttonWidth));
        ok->setDefault(true);
        layout->addWidget(buttons);
        ok->setEnabled(false);
        QObject::connect(key, &QLineEdit::textChanged, &dialog, [ok](const QString &value) {
            ok->setEnabled(!value.isEmpty());
        });
        QObject::connect(ok, &QPushButton::clicked, &dialog, &QDialog::accept);
        QObject::connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
        dialog.resize(420, layout->totalHeightForWidth(420));
        if (dialog.exec() != QDialog::Accepted) return false;
        config.sqliteKey = key->text();
    }
    MainDatabaseConnectionContext::instance().setData(config);
    return true;
}
}
#endif
