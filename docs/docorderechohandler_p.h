#pragma once

#include <docs/docorderechohandler.h>
#include <docs/docorderecho.h>
#include <ui_docorderecho.h>

class DocOrderEchoHandler::Impl {
public:
    DocOrderEcho& o; // owner
    explicit Impl(DocOrderEcho& owner) : o(owner) {}

    void insertNewCatDoctor();
    void openCatDoctor();
    void updateModelDoctor(BaseSqlQueryModel& modelDoctors);
    void updateModelNurse(BaseSqlQueryModel& modelNurses);
    void updateModelPatients(QStandardItemModel& modelPatients);

};

