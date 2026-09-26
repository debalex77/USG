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
