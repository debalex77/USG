#ifndef STRUCTVARIABLE_H
#define STRUCTVARIABLE_H

#include <QPixmapCache>
#include <QString>

// Structură pentru toate variabilele globale
struct GlobalVariable {

    // Tipuri complexe mari
    QPixmapCache cache_img;

    QString langApp     = nullptr;
    QString unitMeasure = nullptr;
    QString nameUserApp = nullptr;

    // int-uri
    int moveApp              = -1;
    int numSavedFilesLog      = -1;

    // bool-uri (la final)
    bool unknowModeLaunch  = false;
    bool isSystemThemeDark = false;
    bool firstLaunch       = false;
    bool memoryUser            = false;
};

#endif // STRUCTVARIABLE_H
