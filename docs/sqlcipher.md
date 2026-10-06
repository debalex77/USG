# SQLCipher local

SQLite rămâne implicit. În Setări → conexiune SQLite, opțiunea SQLCipher
selectează QSQLCIPHER pentru baza principală și baza de imagini. Profilul
salvează numai `connect/sqliteEncrypted=true`, fără cheia de criptare.

La salvarea setărilor sau la pornire, aplicația cere cheia într-o fereastră
SQLCipher. Butonul „Generează cheie pentru o bază nouă” produce 32 de octeți
aleatori, afișați ca 64 de caractere hexazecimale. Păstrați cheia și folosiți-o
la lansările următoare. Pentru o bază existentă, introduceți cheia ei, fără
a genera alta. Alternativ, furnizați cheia în variabila de mediu
`USG_SQLCIPHER_KEY` prin mecanismul de lansare al aplicației. Nu introduceți
cheia în fișierul INI. Aceeași cheie este folosită pentru cele două fișiere.
Variabilele de mediu nu sunt un seif de parole; integrarea cu un seif al
sistemului poate înlocui ulterior acest mecanism.

Opțiunea necesită pluginul QSQLCIPHER (libqsqlcipher.so pe Linux,
qsqlcipher.dll pe Windows) și bibliotecile Qt corespunzătoare.
Se verifică cipher_version, accesul la schemă și foreign_keys pe fiecare
conexiune. Cheile nu se includ în mesajele de eroare ale configurării.

Bazele existente nu sunt convertite automat. Pregătiți copii criptate cu
sqlcipher_export(), verificați-le, apoi configurați căile lor și activați
opțiunea. Nu activați SQLCipher peste fișiere SQLite necriptate.
Conversia și rotația cheii nu sunt implementate în interfață.

Conexiunile principale și cele din fire folosesc aceeași funcție de
inițializare. Durata de viață a firelor și conexiunilor nu a fost schimbată;
derivarea cheii se repetă pentru conexiunile noi.
