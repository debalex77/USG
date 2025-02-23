#include "globals.h"

// Inițializare container global cu un singur obiect GlobalVariable implicit
QVector<GlobalVariable> globalVars = { GlobalVariable{} };

// Funcție utilitară care returnează referință la primul element
GlobalVariable& globals() {
    return globalVars.first();
}
