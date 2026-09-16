#include "legacysettingscodec.h"

#include <QByteArray>

namespace {
constexpr char encodingKey = 073;
}

QString LegacySettingsCodec::encode(const QString &value)
{
    QByteArray bytes(value.toUtf8());
    for (char &byte : bytes)
        byte ^= encodingKey;
    return QString::fromLatin1(bytes.toBase64());
}

QString LegacySettingsCodec::decode(const QString &value)
{
    QByteArray bytes = QByteArray::fromBase64(value.toLatin1());
    for (char &byte : bytes)
        byte ^= encodingKey;
    return QString::fromUtf8(bytes);
}

bool LegacySettingsCodec::isValid(const QString &encodedValue)
{
    return encodedValue.isEmpty() || encode(decode(encodedValue)) == encodedValue;
}
