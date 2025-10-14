#ifndef DOCORDERECHOHANDLER_H
#define DOCORDERECHOHANDLER_H

#include <memory>
#include <docs/docorderecho.h>

class DocOrderEchoHandler
{
public:
    DocOrderEchoHandler(DocOrderEcho& owner);
    ~DocOrderEchoHandler();

    void insertNewDoctor();
    void openCatDoctor();
    void updateModelDoctor(BaseSqlQueryModel& modelDoctors);
    void updateModelNurse(BaseSqlQueryModel& modelNurses);
    void updateModelPatients(QStandardItemModel& modelPatients);

private:
    class Impl;                       // forward-decl
    std::unique_ptr<Impl> d;          // PIMPL
};

#endif // DOCORDERECHOHANDLER_H
