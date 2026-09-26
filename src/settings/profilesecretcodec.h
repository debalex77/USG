#ifndef PROFILESECRETCODEC_H
#define PROFILESECRETCODEC_H

#include <QString>

namespace ProfileSecretCodec {

[[nodiscard]] bool isEncrypted(const QString &value);

[[nodiscard]] bool encrypt(const QString &plainText,
                           const QString &settingsPath,
                           QString *encodedValue,
                           QString *error = nullptr);

[[nodiscard]] bool decrypt(const QString &encodedValue,
                           const QString &settingsPath,
                           QString *plainText,
                           QString *error = nullptr);

}

#endif // PROFILESECRETCODEC_H
