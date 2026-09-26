#ifndef LEGACYSETTINGSCODEC_H
#define LEGACYSETTINGSCODEC_H

#include <QString>

// clasa in care se efectuiaza codul vechi de codare si decodare aparolei
//
namespace LegacySettingsCodec {

    QString encode(const QString &value);
    QString decode(const QString &value);
    bool isValid(const QString &encodedValue);

}

#endif // LEGACYSETTINGSCODEC_H
