#ifndef SEARCHLINEEDIT_H
#define SEARCHLINEEDIT_H

#include <QLineEdit>
#include <QPushButton>
#include <QMenu>
#include <QHBoxLayout>

class SearchLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchLineEdit(QWidget *parent = nullptr);
    ~SearchLineEdit();

    void setupUI();
    void setupMenu();
    void updateButtonPosition();
    void setSearchOptionSelected(QString typeSearch);
    /** Înlocuiește opțiunile implicite (cod/denumire): perechi (identificator, text). */
    void setSearchOptions(const QList<std::pair<QString, QString>> &options);
    /** Iconița opțiunii: apare în meniu și pe buton după alegerea opțiunii. */
    void setSearchOptionIcon(const QString &typeSearch, const QIcon &icon);
    QString getSearchOptionSelected();

signals:
    void setSearchByCodeName(QString typeSearch);

private slots:
    void onSearchOptionSelected();

private:
    QPushButton *searchButton;
    QMenu *searchMenu;
    QString m_typeSearch = nullptr;
    const int PADDING = 4;

protected:
    void resizeEvent(QResizeEvent *event) override;

};

#endif // SEARCHLINEEDIT_H
