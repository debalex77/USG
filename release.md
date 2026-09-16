# USG v4.1.0

Versiunea 4.1.0 consolidează lucrul cu pacienții, Comenzile ecografice și
Rapoartele ecografice pe SQLite și MariaDB. Aplicația este compilată cu Qt
6.9.3, C++20 și qmake.

## Modificări principale

- migrarea tabelei `pacients` la schema revizuită `patients`;
- UUID-uri coerente între SQLite și MariaDB pentru sincronizare;
- ferestre dedicate `OrderView` și `ReportView`, în locul jurnalului vechi;
- programarea pacienților cu pacient existent sau nume liber și investigații
  multiple;
- corectări pentru salvarea, redeschiderea, revalidarea și tipărirea rapoartelor;
- sincronizarea pacientului, comenzii, raportului și imaginilor;
- export PDF și pregătirea mesajelor e-mail cu semnătură și ștampilă;
- setări, lansare, creare de bază nouă și jurnalizare mai robuste;
- LimeReport actualizat de la 1.7.14 la 1.7.23 și compilat pentru Qt 6.9.3;
- audit și documentație pentru licențele iconițelor și componentelor terțe.

Istoricul detaliat este disponibil în
[`resources/RELEASES.md`](resources/RELEASES.md).

## Înainte de actualizare

Faceți copii de siguranță pentru:

- baza principală SQLite sau MariaDB;
- baza imaginilor și celelalte atașamente;
- configurația aplicației și materialul criptografic;
- șabloanele de tipărire personalizate.

O bază partajată trebuie migrată de un singur client. Nu folosiți simultan
versiuni USG vechi și noi pe aceeași bază în timpul actualizării.

## Artefacte Linux

- `USG_v4.1.0_Linux_amd64.run` — installer Qt Installer Framework;
- `USG_v4.1.0_Linux_amd64.deb` — pachet Debian;
- `USG_v4.1.0-x86_64.AppImage` — pachet portabil;
- `LimeReport_v1.7.23_USG_source.tar.gz` — sursa corespunzătoare LimeReport și
  corecția folosită de USG.

Verificați suma SHA-256 furnizată pentru fiecare artefact înainte de instalare.

### AppImage pe Debian 13

Runtime-ul AppImage folosește FUSE 2. Instalați biblioteca compatibilă și
porniți aplicația:

```bash
sudo apt install libfuse2t64
chmod +x USG_v4.1.0-x86_64.AppImage
./USG_v4.1.0-x86_64.AppImage
```

Dacă FUSE nu poate fi instalat, folosiți modul de extragere temporară:

```bash
./USG_v4.1.0-x86_64.AppImage --appimage-extract-and-run
```

## Confidențialitate și raportarea problemelor

Nu publicați baze reale, date despre pacienți, imagini medicale, configurații,
parole sau chei criptografice. Pentru probleme reproductibile folosiți o copie
anonimizată și indicați versiunea USG, sistemul de operare, motorul bazei de date
și numai liniile de log relevante.
