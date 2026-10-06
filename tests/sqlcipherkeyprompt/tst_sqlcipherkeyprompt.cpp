#include <QApplication>
#include <QTimer>
#include "infrastructure/database/sqlcipherkeyprompt.h"
int main(int argc,char **argv) {
 QApplication app(argc,argv);
 QTimer::singleShot(0, [] {
  auto *dialog=qobject_cast<QDialog *>(QApplication::activeModalWidget());
  if(!dialog) qFatal("dialog missing");
  for(auto *button:dialog->findChildren<QPushButton *>())
   if(button->text().contains("Generează")) button->click();
  auto *key=dialog->findChild<QLineEdit *>();
  if(!key || key->text().size()!=64) qFatal("generated key invalid");
  dialog->accept();
 });
 if(!SqlCipherKeyPrompt::ensure(true)) return 1;
 if(MainDatabaseConnectionContext::instance().data().sqliteKey.size()!=64) return 2;
 MainDatabaseConnectionContext::instance().clear();
 QTimer::singleShot(0, [] { qobject_cast<QDialog *>(QApplication::activeModalWidget())->reject(); });
 if(SqlCipherKeyPrompt::ensure(true)) return 3;
 if(!MainDatabaseConnectionContext::instance().data().sqliteKey.isEmpty()) return 4;
}
