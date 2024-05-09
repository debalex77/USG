#ifndef CHOICECOLUMNS_H
#define CHOICECOLUMNS_H

#include <QDialog>
#include <QDebug>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>

class ChoiceColumns : public QDialog
{
    Q_OBJECT
public:
    ChoiceColumns(QWidget *parent = nullptr);

    void set_id(bool _id){show_id = _id;}
    bool get_id(){return show_id;}

    void set_deletionMark(bool _deletionMark) {show_deletionMark = _deletionMark;}
    bool get_deletionMark(){return show_deletionMark;}

    void set_attachedImages(bool _attachedImages){show_attachedImages = _attachedImages;}
    bool get_attachedImages(){return show_attachedImages;}

    void set_cardPayment(bool _cardPayment){show_cardPayment = _cardPayment;}
    bool get_cardPayment(){return show_cardPayment;}

    void set_numberDoc(bool _numberDoc){show_numberDoc = _numberDoc;}
    bool get_numberDoc(){return show_numberDoc;}

    void set_dateDoc(bool _dateDoc){show_dateDoc = _dateDoc;}
    bool get_dateDoc(){return show_dateDoc;}

    void set_idOrganization(bool _idOrganization){show_idOrganization = _idOrganization;}
    bool get_idOrganization(){return show_idOrganization;}

    void set_Organization(bool _Organization){show_Organization = _Organization;}
    bool get_Organization(){return show_Organization;}

    void set_idContract(bool _idContract){show_idContract = _idContract;}
    bool get_idContract(){return show_idContract;}

    void set_Contract(bool _Contract){show_Contract = _Contract;}
    bool get_Contract(){return show_Contract;}

    void set_idPacient(bool _idPacient){show_idPacient = _idPacient;}
    bool get_idPacient(){return show_idPacient;}

    void set_searchPacient(bool _searchPacient){show_searchPacient = _searchPacient;}
    bool get_searchPacient(){return show_searchPacient;}

    void set_pacient(bool _pacient){show_pacient = _pacient;}
    bool get_pacient(){return show_pacient;}

    void set_IDNP(bool _idnp){show_IDNP = _idnp;}
    bool get_IDNP(){return show_IDNP;}

    void set_idUser(bool _idUser){show_idUser = _idUser;}
    bool get_idUser(){return show_idUser;}

    void set_user(bool _user){show_user = _user;}
    bool get_user(){return show_user;}

    void set_sum(bool _sum){show_sum = _sum;}
    bool get_sum(){return show_sum;}

    void set_comment(bool _comment){show_comment = _comment;}
    bool get_comment(){return show_comment;}

signals:
    void saveData();

public slots:
    void highlightChecked(QListWidgetItem* item);
    void save();
    void setListWidget();

private:
    void createListWidget();
    void createOtherWidgets();
    void createLayout();
    void createConnections();

private:

    bool show_id             = false;
    bool show_deletionMark   = false;
    bool show_attachedImages = false;
    bool show_cardPayment    = false;
    bool show_numberDoc      = false;
    bool show_dateDoc        = false;
    bool show_idOrganization = false;
    bool show_Organization   = false;
    bool show_idContract     = false;
    bool show_Contract       = false;
    bool show_idPacient      = false;
    bool show_searchPacient  = false;
    bool show_pacient        = false;
    bool show_IDNP           = false;
    bool show_idUser         = false;
    bool show_user           = false;
    bool show_sum            = false;
    bool show_comment        = false;

    QListWidget *widget;
    QGroupBox   *viewBox;
    QPushButton *btnOK;
    QPushButton *btnClose;
    QDialogButtonBox *buttonBox;
};

#endif // CHOICECOLUMNS_H
