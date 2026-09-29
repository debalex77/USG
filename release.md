# USG v4.2.3

## Modificări principale

- Manualul de utilizare în limba română a fost rescris, completat cu
  capturi de ecran și legat direct de butonul **Manual Online** din fereastra
  principală. Pagina actualizată este publicată în GitHub Wiki.
- A fost adăugată fereastra **Servere cloud** pentru vizualizarea, adăugarea,
  modificarea și eliminarea configurațiilor de sincronizare.
- Sincronizarea SQLite–MariaDB poate fi activată sau dezactivată din
  preferințele aplicației. Migrarea 4.2.3 adaugă în mod controlat coloana
  necesară atât în SQLite, cât și în MariaDB.
- Datele de tipărire sunt încărcate în contextul documentului: organizația,
  logo-ul, ștampila organizației, doctorul, semnătura și ștampila doctorului nu
  mai depind de imagini globale rămase în cache.
- Contextele dedicate pentru Comanda ecografică și Raportul ecografic păstrează
  identitatea organizației, utilizatorului și doctorului documentului deschis.
- Au fost revizuite exportul PDF, anexele și selectarea contului în agentul de
  e-mail.
- Au fost completate acțiunile pentru istoricul pacientului și jurnalele
  Comenzilor/Rapoartelor ecografice.
- A fost corectată închiderea ferestrei Servere cloud și a aplicației când
  această fereastră este deschisă.
- Pachetele Linux verifică RPATH-ul portabil și includ LimeReport fără a depinde
  de instalarea globală a bibliotecii. Instalatorul nu mai acumulează intrări
  desktop duplicate.
- Interfața în limba rusă a fost actualizată pentru funcțiile noi.

## Actualizare și verificare

Înainte de instalare creați copii de siguranță pentru baza principală, baza de
imagini și configurația profilului.

Pentru un profil SQLite sincronizat cu MariaDB:

1. actualizați mai întâi profilul SQLite, cu serverul cloud accesibil;
2. așteptați finalizarea migrării și verificați jurnalul;
3. deschideți profilul MariaDB direct numai după confirmarea actualizării.

După actualizare verificați autentificarea, deschiderea unei Comenzi și a unui
Raport, previzualizarea cu imaginile corecte ale organizației/doctorului,
trimiterea unui e-mail și starea preferinței de sincronizare.

## Pachete Windows

- `USG_v4.2.3_Windows_amd64.exe` — installer.
- `LimeReport_v1.7.23_USG_source.zip` — sursa dependenței.
- Fișierele SHA-256 sunt publicate alături de arhive.

## Pachete Linux

- `USG_v4.2.3_Linux_amd64.run` — Qt Installer Framework installer.
- `USG_v4.2.3_Linux_amd64.deb` — pachet Debian.
- `USG_v4.2.3-x86_64.AppImage` — pachet portabil.
- `LimeReport_v1.7.23_USG_source.tar.gz` — sursa LimeReport și corecția folosită
  de USG.

Verificați fiecare pachet folosind suma SHA-256 publicată.

### Pornirea AppImage

```bash
chmod +x USG_v4.2.3-x86_64.AppImage
./USG_v4.2.3-x86_64.AppImage
```

Dacă FUSE nu este disponibil:

```bash
./USG_v4.2.3-x86_64.AppImage --appimage-extract-and-run
```

Istoricul complet: [`resources/RELEASES.md`](resources/RELEASES.md).
