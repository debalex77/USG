#pragma once

#include <memory>
#include <docs/docreportecho.h>

class DocReportEchoHandler
{
public:
    explicit DocReportEchoHandler(DocReportEcho& owner);
    ~DocReportEchoHandler();

    void applyPropertyMaxLengthText();
    void applyDefaultsForSelected();
    void appleConnections(bool enable);
    void loadAllSelectedIntoForm();

    bool insertDataDoc(DataBase* db, QString &details_error);
    bool updateDataDoc(DataBase* db, QString &details_error);

    void printDoc(Enums::TYPE_PRINT typeReport, QString &filePDF, LimeReport::ReportEngine *m_report, DataBase* db);

private:
    class Impl;                       // forward-decl
    std::unique_ptr<Impl> d;          // PIMPL
};

