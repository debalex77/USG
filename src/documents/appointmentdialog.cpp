#include "appointmentdialog.h"
#include "ui_appointmentdialog.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QCompleter>
#include <QHeaderView>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyledItemDelegate>
#include <QStyleFactory>
#include <QTimer>

#include <common/globals.h>
#include <data/popup.h>
#include <delegates/checkboxdelegate.h>
#include <delegates/combodelegate.h>
#include <documents/orderdialog.h>
#include <models/registrationtablemodel.h>

//---------------------------------------------------------------------

class PatientAppointmentDelegate final : public QStyledItemDelegate
{
public:
    PatientAppointmentDelegate(DataBase &db, QObject *parent)
        : QStyledItemDelegate(parent), m_db(db) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
                          const QModelIndex &) const override
    {
        auto *combo = new QComboBox(parent);
        combo->setEditable(true);
        combo->setInsertPolicy(QComboBox::NoInsert);

        auto *patients = new QStandardItemModel(combo);
        auto *completer = new QCompleter(patients, combo);

        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setCompletionMode(QCompleter::PopupCompletion);
        completer->setFilterMode(Qt::MatchContains);

        auto *popup = completer->popup();

        popup->setMinimumWidth(380);
        popup->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        popup->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        combo->lineEdit()->setCompleter(completer);

        auto *timer = new QTimer(combo);
        timer->setSingleShot(true);
        timer->setInterval(250);

        connect(combo->lineEdit(), &QLineEdit::textEdited, combo,
                [combo, timer](const QString &) {
                    combo->setProperty("selectedPatientId", QVariant());
                    combo->setProperty("selectedPatientText", QVariant());
                    timer->start();
                });

        connect(timer, &QTimer::timeout, combo, [this, combo, patients, completer]() {
            updateModelPatientsByText(combo, patients, completer);
        });

        connect(completer, QOverload<const QModelIndex &>::of(&QCompleter::activated),
                combo, [combo](const QModelIndex &index) {
                    const QString text = index.data(Qt::DisplayRole).toString();
                    combo->setEditText(text);
                    combo->setProperty("selectedPatientId", index.data(Qt::UserRole));
                    combo->setProperty("selectedPatientText", text);
                });

        connect(combo->lineEdit(), &QLineEdit::returnPressed, combo,
                [this, combo]() {
                    if (combo->property("patientCommitScheduled").toBool())
                        return;

                    combo->setProperty("patientCommitScheduled", true);

                    // QCompleter procesează Enter în același ciclu. Confirmăm
                    // după activarea rezultatului, ca să fie păstrat și ID-ul.
                    QTimer::singleShot(0, combo, [this, combo]() {
                        auto *delegate = const_cast<PatientAppointmentDelegate *>(this);
                        emit delegate->commitData(combo);
                        emit delegate->closeEditor(combo, QAbstractItemDelegate::EditNextItem);
                    });
                });

        return combo;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        auto *combo = qobject_cast<QComboBox *>(editor);
        if (!combo)
            return;

        const QString text = index.data(Qt::EditRole).toString();

        combo->setEditText(text);
        combo->setProperty("selectedPatientId", index.data(Qt::UserRole));
        combo->setProperty("selectedPatientText", text);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override
    {
        auto *combo = qobject_cast<QComboBox *>(editor);
        if (!combo)
            return;

        const QString text = combo->currentText().trimmed();
        QVariant patientId;

        if (text == combo->property("selectedPatientText").toString())
            patientId = combo->property("selectedPatientId");

        model->setData(index, text, Qt::EditRole);
        model->setData(index, patientId, Qt::UserRole);
    }

private:
    void updateModelPatientsByText(QComboBox *combo, QStandardItemModel *patients,
                                   QCompleter *completer) const
    {
        const QString text = combo->currentText().trimmed();
        patients->clear();
        patients->setColumnCount(1);

        if (text.size() < 2)
            return;

        QSqlQuery query(m_db.getDatabase());
        query.prepare(m_db.getTextSQL(globals().thisMySQL
            ? QStringLiteral(":/sql/queries_doc/searchPatientByText_mariadb.sql")
            : QStringLiteral(":/sql/queries_doc/searchPatientByText_sqlite.sql")));
        const QString prefix = text + QLatin1Char('%');
        const QString contains = QLatin1Char('%') + text + QLatin1Char('%');
        query.addBindValue(prefix);
        query.addBindValue(prefix);
        query.addBindValue(contains);
        query.addBindValue(contains);
        if (!query.exec()) {
            qWarning(logWarning()) << "AppointmentDialog patient search error:"
                                   << query.lastError().text();
            return;
        }
        while (query.next()) {
            auto *item = new QStandardItem(query.value(1).toString());
            item->setData(query.value(0).toInt(), Qt::UserRole);
            patients->appendRow(item);
        }
        if (patients->rowCount() > 0)
            completer->complete();
    }

    DataBase &m_db;
};

class MultiSelectionCombo final : public QComboBox
{
public:
    explicit MultiSelectionCombo(QWidget *parent = nullptr)
        : QComboBox(parent)
    {
        setEditable(true);
        lineEdit()->setReadOnly(true);
        view()->viewport()->installEventFilter(this);
    }

    QList<int> selectedIds() const
    {
        QList<int> result;
        for (int row = 0; row < count(); ++row) {
            const auto *item = qobject_cast<QStandardItemModel *>(model())->item(row);
            if (item && item->checkState() == Qt::Checked)
                result.append(item->data(Qt::UserRole).toInt());
        }
        return result;
    }

    void setSelectedIds(const QList<int> &ids)
    {
        for (int row = 0; row < count(); ++row) {
            auto *item = qobject_cast<QStandardItemModel *>(model())->item(row);
            if (item)
                item->setCheckState(
                    ids.contains(item->data(Qt::UserRole).toInt())
                        ? Qt::Checked : Qt::Unchecked
                    );
        }
        updateText();
    }

    QString selectedText() const
    {
        QStringList result;
        for (int row = 0; row < count(); ++row) {
            const auto *item = qobject_cast<QStandardItemModel *>(model())->item(row);
            if (item && item->checkState() == Qt::Checked)
                result.append(item->text());
        }
        return result.join(QStringLiteral("; "));
    }

protected:
    bool eventFilter(QObject *watched, QEvent *event) override
    {
        if (watched == view()->viewport() && event->type() == QEvent::MouseButtonRelease) {
            const auto *mouse = static_cast<QMouseEvent *>(event);
            const QModelIndex index = view()->indexAt(mouse->position().toPoint());
            auto *item = qobject_cast<QStandardItemModel *>(model())->itemFromIndex(index);
            if (item) {
                item->setCheckState(item->checkState() == Qt::Checked
                                        ? Qt::Unchecked : Qt::Checked);
                updateText();
            }
            return true;
        }
        return QComboBox::eventFilter(watched, event);
    }

private:
    void updateText()
    {
        lineEdit()->setText(selectedText());
    }
};

class MultiInvestigationDelegate final : public QStyledItemDelegate
{
public:
    MultiInvestigationDelegate(DataBase &db, QObject *parent)
        : QStyledItemDelegate(parent), m_db(db) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
                          const QModelIndex &) const override
    {
        auto *combo = new MultiSelectionCombo(parent);
        auto *model = new QStandardItemModel(combo);
        combo->setModel(model);

        QSqlQuery query(m_db.getDatabase());
        query.prepare(globals().thisMySQL
            ? QStringLiteral("SELECT id, CONCAT(cod, ' - ', name) FROM investigations WHERE `use`=1 ORDER BY cod")
            : QStringLiteral("SELECT id, cod || ' - ' || name FROM investigations WHERE `use`=1 ORDER BY cod"));
        if (!query.exec()) {
            qWarning(logWarning()) << "AppointmentDialog investigations load error:"
                                   << query.lastError().text();
            return combo;
        }
        while (query.next()) {
            auto *item = new QStandardItem(query.value(1).toString());
            item->setData(query.value(0).toInt(), Qt::UserRole);
            item->setCheckable(true);
            item->setCheckState(Qt::Unchecked);
            model->appendRow(item);
        }

        combo->view()->setMinimumWidth(800);

        return combo;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        auto *combo = dynamic_cast<MultiSelectionCombo *>(editor);
        if (!combo)
            return;

        QList<int> ids;
        const QVariantList values = index.data(Qt::UserRole).toList();
        for (const QVariant &value : values)
            ids.append(value.toInt());
        combo->setSelectedIds(ids);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override
    {
        auto *combo = dynamic_cast<MultiSelectionCombo *>(editor);
        if (!combo)
            return;

        QVariantList ids;
        for (int id : combo->selectedIds())
            ids.append(id);
        model->setData(index, combo->selectedText(), Qt::EditRole);
        model->setData(index, ids, Qt::UserRole);
    }

private:
    DataBase &m_db;
};

struct AppointmentInvestigationSelection
{
    QList<int> ids;
    QStringList names;

    QString displayText() const
    {
        return names.join(QStringLiteral("; "));
    }
};

static bool resolveInvestigationSelection(DataBase &db,
                                          const QVariant &storedIds,
                                          AppointmentInvestigationSelection *selection,
                                          QString *errorText)
{
    if (!selection)
        return false;

    selection->ids.clear();
    selection->names.clear();

    QSqlQuery query(db.getDatabase());
    query.prepare(QStringLiteral(
        "SELECT cod, name FROM investigations WHERE id=:investigationId"));

    for (const QVariant &value : storedIds.toList()) {
        const int id = value.toInt();
        if (id <= 0 || selection->ids.contains(id))
            continue;

        query.bindValue(QStringLiteral(":investigationId"), id);
        if (!query.exec()) {
            if (errorText)
                *errorText = query.lastError().text();
            return false;
        }
        if (!query.next()) {
            if (errorText)
                *errorText = QObject::tr("Investigația selectată cu ID %1 nu mai există.").arg(id);
            return false;
        }

        selection->ids.append(id);
        selection->names.append(QStringLiteral("%1 - %2")
                                    .arg(query.value(0).toString(), query.value(1).toString()));
        query.finish();
    }

    return true;
}

//---------------------------------------------------------------------
//
//---------------------------------------------------------------------
AppointmentDialog::AppointmentDialog(DataBase &db, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AppointmentDialog)
    , m_db(db)
    , m_popup(new PopUp(this))
    , m_model(new RegistrationPatientsModel(RowCount, SectionCount, this))
{
    ui->setupUi(this);
    setWindowTitle(tr("Programarea pacienților [*]"));
    setWindowIcon(QIcon(QStringLiteral(":/img/documents/appointment_pacients.png")));
    ui->docDate->setDate(QDate::currentDate());
    setupStyle();
    setupDelegates();
    setupTable();
    setupConnections();
    reload();
}

AppointmentDialog::~AppointmentDialog()
{
    delete ui;
}

void AppointmentDialog::setupDelegates()
{
    const QString connection = m_db.getDatabase().connectionName();

    // delegate organization
    m_organizationDelegate = new ComboDelegate(ui->tableView);
    m_organizationDelegate->setConnectionName(connection);
    m_organizationDelegate->setQuery(QStringLiteral(
        R"(SELECT
                id,
                name
            FROM
                organizations
            WHERE
                deletionMark = 0
            ORDER BY
                name
            )"));
    ui->tableView->setItemDelegateForColumn(Organization, m_organizationDelegate);

    // delegate doctor
    m_doctorDelegate = new ComboDelegate(ui->tableView);
    m_doctorDelegate->setConnectionName(connection);
    m_doctorDelegate->setQuery(globals().thisMySQL
        ? QStringLiteral(R"(
                SELECT
                    id,
                    CONCAT(name, ' ', LEFT(fName, 1), '.')
                FROM
                    doctors
                WHERE
                    deletionMark = 0
                ORDER BY
                    name
                )")
        : QStringLiteral(R"(
                SELECT
                    id,
                    name || ' ' || substr(fName, 1, 1) || '.'
                FROM
                    doctors
                WHERE
                    deletionMark = 0
                ORDER BY
                    name
            )"));
    ui->tableView->setItemDelegateForColumn(Doctor, m_doctorDelegate);

    m_patientDelegate = new PatientAppointmentDelegate(m_db, ui->tableView);
    ui->tableView->setItemDelegateForColumn(Patient, m_patientDelegate);

    // selecție multiplă; fiecare ID este salvat în tabela copil
    m_investigationDelegate = new MultiInvestigationDelegate(m_db, ui->tableView);
    ui->tableView->setItemDelegateForColumn(Investigation, m_investigationDelegate);

    // delegate execute
    m_executedDelegate = new CheckBoxDelegate(ui->tableView);
    ui->tableView->setItemDelegateForColumn(Executed, m_executedDelegate);
}

void AppointmentDialog::setupConnections()
{
    connect(ui->btnWrite, &QToolButton::clicked, this, &AppointmentDialog::saveAppointments);
    connect(ui->btnPrint, &QToolButton::clicked, this, &AppointmentDialog::printAppointments);
    auto *removeMenu = new QMenu(ui->btnRemove);
    removeMenu->addAction(tr("Elimină rândul selectat"),
                          this, &AppointmentDialog::removeCurrentAppointment);
    removeMenu->addAction(tr("Elimină toate programările zilei"),
                          this, &AppointmentDialog::removeAppointments);
    ui->btnRemove->setMenu(removeMenu);
    ui->btnRemove->setPopupMode(QToolButton::InstantPopup);
    connect(ui->btnOrderEcho, &QToolButton::clicked, this, &AppointmentDialog::createOrder);
    connect(ui->btnClose, &QToolButton::clicked, this, &AppointmentDialog::close);
    connect(ui->btnBackDay, &QToolButton::clicked, this, &AppointmentDialog::showPreviousDay);
    connect(ui->btnNextDay, &QToolButton::clicked, this, &AppointmentDialog::showNextDay);
    connect(ui->docDate, &QDateEdit::dateChanged, this, &AppointmentDialog::reload);
    connect(m_model, &RegistrationTableModel::m_data_changed,
            this, &AppointmentDialog::dataWasModified);
}

void AppointmentDialog::setupTable()
{
    ui->tableView->setModel(m_model);
    ui->tableView->hideColumn(Id);
    ui->tableView->setColumnWidth(Executed, 70);
    ui->tableView->setColumnWidth(Patient, 250);
    ui->tableView->setColumnWidth(Investigation, 300);
    ui->tableView->setColumnWidth(Organization, 200);
    ui->tableView->setColumnWidth(Doctor, 200);
    ui->tableView->setColumnWidth(Comment, 400);
    ui->tableView->verticalHeader()->setStretchLastSection(true);
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
}

void AppointmentDialog::setupStyle()
{
    QString style_toolButton = m_db.toolButtonStyleForText();

    for (QToolButton *button : {ui->btnWrite, ui->btnRemove, ui->btnPrint,
                                ui->btnOrderEcho, ui->btnClose, ui->btnBackDay, ui->btnNextDay})
        button->setStyleSheet(style_toolButton);
#if defined(Q_OS_WIN)
    ui->frame->setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
#endif
}

void AppointmentDialog::dataWasModified()
{
    if (!m_loading)
        setWindowModified(true);
}

void AppointmentDialog::showPreviousDay()
{
    ui->docDate->setDate(ui->docDate->date().addDays(-1));
}

void AppointmentDialog::showNextDay()
{
    ui->docDate->setDate(ui->docDate->date().addDays(1));
}

void AppointmentDialog::reload()
{
    m_loading = true;
    ui->tableView->setModel(nullptr);
    delete m_model;
    m_model = new RegistrationPatientsModel(RowCount, SectionCount, this);

    QSqlQuery query(m_db.getDatabase());
    query.prepare(QStringLiteral(
        R"(SELECT
                id,
                time_registration,
                execute,
                dataPatient,
                id_organizations,
                id_doctors,
                investigations,
                patient_id,
                investigation_id,
                comment
            FROM
                patientAppointments
            WHERE
                dateDoc = :dateDoc
        )"));
    query.bindValue(QStringLiteral(":dateDoc"), ui->docDate->date().toString(Qt::ISODate));
    if (!query.exec()) {
        qCritical(logCritical()) << "AppointmentDialog reload error:"
                                 << query.lastError().text();
        QMessageBox::warning(this,
                             tr("Programarea pacienților"),
                             tr("Programările nu au putut fi încărcate.\n%1")
                                 .arg(query.lastError().text()));
    } else {
        while (query.next()) {
            const int slot = query.value(1).toInt();
            if (slot < 1 || slot > RowCount) {
                qWarning(logWarning()) << "AppointmentDialog: interval invalid:" << slot;
                continue;
            }
            m_model->setDataPatient(slot, Id + 1, query.value(0).toInt());
            m_model->setDataPatient(slot, Executed + 1, query.value(2).toInt());
            m_model->setDataPatient(slot, Patient + 1, query.value(3).toString());
            m_model->setDataPatient(slot, Patient + 1, query.value(7), Qt::UserRole);
            QVariantList investigationIds;
            QStringList investigationTexts;
            QSqlQuery relations(m_db.getDatabase());
            relations.prepare(QStringLiteral(
                "SELECT r.investigation_id, i.cod, i.name "
                "FROM patientAppointmentInvestigations r "
                "JOIN investigations i ON i.id=r.investigation_id "
                "WHERE r.appointment_id=:appointmentId ORDER BY r.position, r.investigation_id"));
            relations.bindValue(QStringLiteral(":appointmentId"), query.value(0));
            if (relations.exec()) {
                while (relations.next()) {
                    investigationIds.append(relations.value(0));
                    investigationTexts.append(QStringLiteral("%1 - %2")
                                                  .arg(relations.value(1).toString(),
                                                       relations.value(2).toString()));
                }
            } else {
                qWarning(logWarning()) << "AppointmentDialog relations load error:"
                                       << relations.lastError().text();
            }
            if (investigationIds.isEmpty() && query.value(8).toInt() > 0)
                investigationIds.append(query.value(8));
            m_model->setDataPatient(slot, Investigation + 1,
                                    investigationTexts.isEmpty()
                                        ? query.value(6).toString()
                                        : investigationTexts.join(QStringLiteral("; ")));
            m_model->setDataPatient(slot, Investigation + 1,
                                    investigationIds, Qt::UserRole);
            m_model->setDataPatient(slot, Organization + 1, query.value(4).toInt());
            m_model->setDataPatient(slot, Doctor + 1, query.value(5).toInt());
            m_model->setDataPatient(slot, Comment + 1, query.value(9).toString());
        }
    }
    setupTable();
    connect(m_model, &RegistrationTableModel::m_data_changed,
            this, &AppointmentDialog::dataWasModified);
    m_loading = false;
    setWindowModified(false);
}

bool AppointmentDialog::saveChanges()
{
    QSqlDatabase database = m_db.getDatabase();
    if (!database.isOpen() || !database.transaction()) {
        QMessageBox::warning(this, tr("Salvarea programării"),
                             database.isOpen() ? database.lastError().text()
                                               : tr("Conexiunea cu baza de date nu este deschisă."));
        return false;
    }

    QString failure;
    for (int row = 0; row < m_model->rowCount(QModelIndex()); ++row) {
        const QModelIndex base = m_model->index(row, Id);
        const int id = base.siblingAtColumn(Id).data(Qt::EditRole).toInt();
        const QString patient = base.siblingAtColumn(Patient).data(Qt::EditRole)
                                    .toString().trimmed();
        if (patient.isEmpty() && id <= 0)
            continue;

        if (patient.isEmpty()) {
            failure = tr("Rândul %1 este deja salvat, dar pacientul a fost golit. "
                         "Folosiți acțiunea «Elimină rândul selectat».").arg(row + 1);
            break;
        }

        const int patientId = base.siblingAtColumn(Patient).data(Qt::UserRole).toInt();
        AppointmentInvestigationSelection investigationSelection;
        if (!resolveInvestigationSelection(
                m_db,
                base.siblingAtColumn(Investigation).data(Qt::UserRole),
                &investigationSelection,
                &failure)) {
            failure = tr("Rândul %1: %2").arg(row + 1).arg(failure);
            break;
        }
        const int organizationId = base.siblingAtColumn(Organization).data(Qt::EditRole).toInt();
        const int doctorId = base.siblingAtColumn(Doctor).data(Qt::EditRole).toInt();

        QSqlQuery query(database);
        if (id <= 0) {
            query.prepare(QStringLiteral(
                R"(INSERT INTO patientAppointments (
                        dateDoc,
                        time_registration,
                        execute,
                        dataPatient,
                        name_organizations,
                        id_organizations,
                        name_doctors,
                        id_doctors,
                        investigations,
                        patient_id,
                        investigation_id,
                        comment
                   ) VALUES (
                        :dateDoc,
                        :time,
                        :execute,
                        :patient,
                        :organizationName,
                        :organizationId,
                        :doctorName,
                        :doctorId,
                        :investigations,
                        :patientId,
                        :investigationId,
                        :comment
                    )
                )"));
        } else {
            query.prepare(QStringLiteral(
                R"(UPDATE patientAppointments SET
                        dateDoc            = :dateDoc,
                        time_registration  = :time,
                        execute            = :execute,
                        dataPatient        = :patient,
                        name_organizations = :organizationName,
                        id_organizations   = :organizationId,
                        name_doctors       = :doctorName,
                        id_doctors         = :doctorId,
                        investigations     = :investigations,
                        patient_id         = :patientId,
                        investigation_id   = :investigationId,
                        comment            = :comment
                    WHERE
                        id = :id
                )"));
            query.bindValue(QStringLiteral(":id"), id);
        }
        query.bindValue(QStringLiteral(":dateDoc"), ui->docDate->date().toString(Qt::ISODate));
        query.bindValue(QStringLiteral(":time"), row + 1);
        query.bindValue(QStringLiteral(":execute"), base.siblingAtColumn(Executed).data(Qt::EditRole).toInt());
        query.bindValue(QStringLiteral(":patient"), patient);
        query.bindValue(QStringLiteral(":organizationName"),
                        m_organizationDelegate->displayText(organizationId, QLocale()));
        query.bindValue(QStringLiteral(":organizationId"), organizationId > 0 ? QVariant(organizationId) : QVariant());
        query.bindValue(QStringLiteral(":doctorName"),
                        m_doctorDelegate->displayText(doctorId, QLocale()));
        query.bindValue(QStringLiteral(":doctorId"), doctorId > 0 ? QVariant(doctorId) : QVariant());
        query.bindValue(QStringLiteral(":investigations"),
                        investigationSelection.ids.isEmpty()
                            ? base.siblingAtColumn(Investigation).data(Qt::EditRole)
                            : QVariant(investigationSelection.displayText()));
        query.bindValue(QStringLiteral(":patientId"),
                        patientId > 0 ? QVariant(patientId) : QVariant());
        // Coloana singulară există numai pentru importul datelor istorice.
        // Pentru salvările curente, relația completă se păstrează exclusiv în
        // patientAppointmentInvestigations.
        query.bindValue(QStringLiteral(":investigationId"), QVariant());
        query.bindValue(QStringLiteral(":comment"), base.siblingAtColumn(Comment).data(Qt::EditRole));
        if (!query.exec()) {
            failure = query.lastError().text();
            break;
        }

        const qint64 appointmentId = id > 0 ? id : query.lastInsertId().toLongLong();
        QSqlQuery relationQuery(database);
        relationQuery.prepare(QStringLiteral(
            R"(DELETE FROM
                    patientAppointmentInvestigations
                WHERE
                    appointment_id = :appointmentId
            )"));
        relationQuery.bindValue(QStringLiteral(":appointmentId"), appointmentId);
        if (!relationQuery.exec()) {
            failure = relationQuery.lastError().text();
            break;
        }
        relationQuery.prepare(QStringLiteral(
            R"(INSERT INTO patientAppointmentInvestigations (
                    appointment_id,
                    investigation_id,
                    position
                ) VALUES (
                    :appointmentId,
                    :investigationId,
                    :position)
            )"));
        for (int position = 0; position < investigationSelection.ids.size(); ++position) {
            relationQuery.bindValue(QStringLiteral(":appointmentId"), appointmentId);
            relationQuery.bindValue(QStringLiteral(":investigationId"),
                                    investigationSelection.ids.at(position));
            relationQuery.bindValue(QStringLiteral(":position"), position);
            if (!relationQuery.exec()) {
                failure = relationQuery.lastError().text();
                break;
            }
        }
        if (!failure.isEmpty())
            break;
    }

    if (!failure.isEmpty() || !database.commit()) {
        if (failure.isEmpty())
            failure = database.lastError().text();
        database.rollback();
        qCritical(logCritical()) << "AppointmentDialog save error:" << failure;
        QMessageBox::warning(this, tr("Salvarea programării"),
                             tr("Datele nu au putut fi salvate.\n%1").arg(failure));
        return false;
    }
    return true;
}

void AppointmentDialog::saveAppointments()
{
    if (!saveChanges())
        return;

    const QString text = tr("Programarea din data %1 a fost salvată cu succes.")
                             .arg(ui->docDate->date().toString(QStringLiteral("dd.MM.yyyy")));

    qInfo(logInfo()) << text;

    m_popup->setPopupText(text);
    m_popup->show();

    reload();
}

void AppointmentDialog::removeAppointments()
{
    if (QMessageBox::question(this, tr("Eliminarea programării"),
                              tr("Doriți să eliminați toate programările din data %1?")
                                  .arg(ui->docDate->date().toString(QStringLiteral("dd.MM.yyyy"))),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes)
        return;
    QSqlQuery query(m_db.getDatabase());
    query.prepare(QStringLiteral("DELETE FROM patientAppointments WHERE dateDoc = :dateDoc"));
    query.bindValue(QStringLiteral(":dateDoc"), ui->docDate->date().toString(Qt::ISODate));
    if (!query.exec()) {
        qCritical(logCritical()) << "AppointmentDialog delete error:" << query.lastError().text();
        QMessageBox::warning(this, tr("Eliminarea programării"), query.lastError().text());
        return;
    }
    const QString text = tr("Programările din data %1 au fost eliminate.")
                             .arg(ui->docDate->date().toString(QStringLiteral("dd.MM.yyyy")));
    qInfo(logInfo()) << text;
    m_popup->setPopupText(text);
    m_popup->show();
    reload();
}

void AppointmentDialog::removeCurrentAppointment()
{
    const int row = currentRow();
    if (row < 0) {
        QMessageBox::information(this, tr("Eliminarea programării"),
                                 tr("Selectați rândul pe care doriți să-l eliminați."));
        return;
    }
    const QModelIndex base = m_model->index(row, Id);
    const int id = base.data(Qt::EditRole).toInt();
    const QString patient = base.siblingAtColumn(Patient).data(Qt::EditRole)
                                .toString().trimmed();
    if (id <= 0 && patient.isEmpty()) {
        QMessageBox::information(this, tr("Eliminarea programării"),
                                 tr("Rândul selectat este gol."));
        return;
    }
    if (QMessageBox::question(this, tr("Eliminarea programării"),
                              tr("Doriți să eliminați programarea selectată%1?")
                                  .arg(patient.isEmpty() ? QString()
                                                         : QStringLiteral(" — ") + patient),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes)
        return;

    if (id > 0) {
        QSqlQuery query(m_db.getDatabase());
        query.prepare(QStringLiteral("DELETE FROM patientAppointments WHERE id = :id"));
        query.bindValue(QStringLiteral(":id"), id);
        if (!query.exec()) {
            qCritical(logCritical()) << "AppointmentDialog row delete error:"
                                     << query.lastError().text();
            QMessageBox::warning(this, tr("Eliminarea programării"),
                                 query.lastError().text());
            return;
        }
        reload();
    } else {
        m_model->clearRow(row);
    }
    const QString text = tr("Programarea selectată a fost eliminată.");
    qInfo(logInfo()) << text;
    m_popup->setPopupText(text);
    m_popup->show();
}

void AppointmentDialog::printAppointments()
{
    QMessageBox::information(this, tr("Printarea programării"),
                             tr("Forma de tipar se află în proces de dezvoltare."));
}

int AppointmentDialog::currentRow() const
{
    return ui->tableView->currentIndex().row();
}

void AppointmentDialog::createOrder()
{
    const int row = currentRow();
    if (row < 0) {
        QMessageBox::warning(this, tr("Crearea comenzii"),
                             tr("Selectați programarea din care doriți să creați comanda."));
        return;
    }
    const QModelIndex base = m_model->index(row, Id);
    const QString patient = base.siblingAtColumn(Patient).data(Qt::EditRole)
                                .toString().trimmed();
    if (patient.isEmpty()) {
        QMessageBox::warning(this, tr("Crearea comenzii"),
                             tr("Programarea selectată nu conține numele pacientului."));
        return;
    }
    auto *order = new OrderDialog(m_db, this);
    order->setAttribute(Qt::WA_DeleteOnClose);
    OrderDialog::PrefillData prefill;
    prefill.organizationId = base.siblingAtColumn(Organization).data(Qt::EditRole).toInt();
    prefill.referringDoctorId = base.siblingAtColumn(Doctor).data(Qt::EditRole).toInt();
    prefill.patientId = base.siblingAtColumn(Patient).data(Qt::UserRole).toInt();
    prefill.patientText = patient;
    const QVariantList investigationValues =
        base.siblingAtColumn(Investigation).data(Qt::UserRole).toList();
    for (const QVariant &value : investigationValues) {
        const int investigationId = value.toInt();
        if (investigationId > 0)
            prefill.investigationIds.append(investigationId);
    }

    QString prefillError;
    const bool prefilled = order->applyPrefillData(prefill, &prefillError);
    order->show();
    if (!prefilled && !prefillError.isEmpty())
        QMessageBox::warning(order, tr("Completarea comenzii"), prefillError);
}

void AppointmentDialog::closeEvent(QCloseEvent *event)
{
    if (!isWindowModified()) {
        event->accept();
        return;
    }
    const auto answer = QMessageBox::warning(
        this, tr("Modificarea datelor"),
        tr("Datele au fost modificate.\nDoriți să salvați aceste modificări?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Cancel);
    if (answer == QMessageBox::Cancel) {
        event->ignore();
        return;
    }
    if (answer == QMessageBox::Yes && !saveChanges()) {
        event->ignore();
        return;
    }
    event->accept();
}
