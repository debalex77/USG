#ifndef GLOBALS_H
#define GLOBALS_H

#include <QHash>
#include <common/StructVariable.h>

// Container global unic accesibil din orice loc al aplicației
extern QVector<GlobalVariable> globalVars;

// Funcții utilitare (opțional, dar recomandat)
GlobalVariable& globals(); // Acces rapid la primul (și probabil unicul) element

#endif // GLOBALS_H
