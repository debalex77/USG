#include "docorderechohandler_p.h"

DocOrderEchoHandler::DocOrderEchoHandler(DocOrderEcho &owner)
    : d(std::make_unique<Impl>(owner)) {}

DocOrderEchoHandler::~DocOrderEchoHandler()
{

}

void DocOrderEchoHandler::insertNewDoctor()
{
    d->insertNewCatDoctor();
}

void DocOrderEchoHandler::openCatDoctor()
{
    d->openCatDoctor();
}

void DocOrderEchoHandler::updateModelDoctor(BaseSqlQueryModel &modelDoctors)
{
    d->updateModelDoctor(modelDoctors);
}

void DocOrderEchoHandler::updateModelNurse(BaseSqlQueryModel &modelNurses)
{
    d->updateModelNurse(modelNurses);
}

void DocOrderEchoHandler::updateModelPatients(QStandardItemModel &modelPatients)
{
    d->updateModelPatients(modelPatients);
}
