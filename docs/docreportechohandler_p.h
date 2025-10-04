#pragma once

#include <docs/docreportechohandler.h>
#include <docs/docreportecho.h>
#include <ui_docreportecho.h>

class DocReportEchoHandler::Impl {
public:
    DocReportEcho& o; // owner

    explicit Impl(DocReportEcho& owner) : o(owner) {}

    /** setam lungimea maxima a textului in elementele formei - docreportechohandler_maxlength.cpp */
    void setPropertyMaxLengthText();

    /** conectari si deconectari - docreportechohandler_connect.cpp */
    void enableSystemConnections(bool enableConnection);

    /** setarea datelor in forma documenului - docreportechohandler_setdata.cpp */
    void setMainTableDoc();
    void setData_OrgansInternal();
    void setData_UrinarySystem();
    void setData_Prostate();
    void setData_Gynecology();
    void setData_Breast();
    void setData_Thyroid();
    void setData_Gestation0();
    void setData_Gestation1();
    void setData_Gestation2();
    void setData_LymphNodes();

    /** date implicite - docreportechohandler_default.cpp
     ** doar la deschiderea documentului nou */
    void setDefault_OrgansInternal();
    void setDefault_UtinarySystem();
    void setDefault_Prostate();
    void setDefault_Gynecology();
    void setDefault_Breast();
    void setDefault_Thyroid();
    void setDefault_Gestation0();
    void setDefault_Gestation1();
    void setDefault_Gestation2();

    /** inserarea datelor - docreportechohandler_insertData.cpp */
    bool insertingDocumentDataIntoTables(DataBase* db, QString &details_error);

    /** inserarea datelor - docreportechohandler_updateData.cpp */
    bool updatingDocumentDataIntoTables(DataBase* db, QString &details_error);

    /** printarea documentului - docreportechohandler_print.cpp */
    void printDoc(Enums::TYPE_PRINT typeReport, QString &filePDF, LimeReport::ReportEngine *m_report, DataBase *db);
};

