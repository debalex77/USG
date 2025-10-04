#include "docreportechohandler_p.h"

DocReportEchoHandler::DocReportEchoHandler(DocReportEcho& owner)
    : d(std::make_unique<Impl>(owner)) {}

DocReportEchoHandler::~DocReportEchoHandler()
{

}

void DocReportEchoHandler::applyPropertyMaxLengthText()
{
    d->setPropertyMaxLengthText();
}

void DocReportEchoHandler::applyDefaultsForSelected()
{
    d->setDefault_OrgansInternal();
    d->setDefault_UtinarySystem();
    d->setDefault_Prostate();
    d->setDefault_Gynecology();
    d->setDefault_Breast();
    d->setDefault_Thyroid();
    d->setDefault_Gestation0();
    d->setDefault_Gestation1();
    d->setDefault_Gestation2();
}

void DocReportEchoHandler::appleConnections(bool enable)
{
    d->enableSystemConnections(enable);
}

void DocReportEchoHandler::loadAllSelectedIntoForm()
{
    d->setMainTableDoc();
    d->setData_OrgansInternal();
    d->setData_UrinarySystem();
    d->setData_Prostate();
    d->setData_Gynecology();
    d->setData_Breast();
    d->setData_Thyroid();
    d->setData_Gestation0();
    d->setData_Gestation1();
    d->setData_Gestation2();
    d->setData_LymphNodes();
}

bool DocReportEchoHandler::insertDataDoc(DataBase *db, QString &details_error)
{
    return d->insertingDocumentDataIntoTables(db, details_error);
}

bool DocReportEchoHandler::updateDataDoc(DataBase *db, QString &details_error)
{
    return d->updatingDocumentDataIntoTables(db, details_error);
}

void DocReportEchoHandler::printDoc(Enums::TYPE_PRINT typeReport, QString &filePDF, LimeReport::ReportEngine *m_report, DataBase *db)
{
    d->printDoc(typeReport, filePDF, m_report, db);
}
