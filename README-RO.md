<p align="center">
  <img alt="Platform" src="https://img.shields.io/badge/platform-Linux%20%7C%20Windows-blue">
  <img alt="Qt" src="https://img.shields.io/badge/Qt-6.9.3-brightgreen">
  <img alt="Ultima versiune" src="https://img.shields.io/github/v/release/debalex77/USG">
  <img alt="Descărcări" src="https://img.shields.io/github/downloads/debalex77/USG/total">
  <img alt="Licență" src="https://img.shields.io/badge/license-GPL--3.0--or--later-blue">
  <a href="https://github.com/debalex77/USG/actions/workflows/build-linux.yml"><img alt="Linux build" src="https://github.com/debalex77/USG/actions/workflows/build-linux.yml/badge.svg?branch=master"></a>
  <a href="https://github.com/sponsors/debalex77"><img alt="Sponsor" src="https://img.shields.io/badge/Sponsor-GitHub-ea4aaa?logo=github"></a>
</p>

# USG – Evidența investigațiilor ecografice

[English](README.md) | [Română](README-RO.md)

USG este o aplicație desktop open-source pentru gestionarea pacienților, a
comenzilor ecografice, a rapoartelor structurate și a imaginilor medicale
atașate. Este destinată medicilor, cabinetelor medicale și clinicilor mici.

Aplicația poate lucra cu o bază locală SQLite sau cu o bază MariaDB. Un flux
opțional, configurat explicit, permite sincronizarea înregistrărilor locale cu
MariaDB. Pentru lucrul local obișnuit nu este obligatoriu un serviciu cloud.

Site oficial: <https://debalex77.github.io/USG/>

## Capturi de ecran

![Fereastra principală](https://github.com/user-attachments/assets/89b3964d-31d1-44ed-bf22-5c2b13642851)
![Evidența pacienților](https://github.com/user-attachments/assets/667abdf9-c456-49e2-91bc-4981ce476da9)

## Funcționalități principale

- evidența pacienților și istoricul investigațiilor ecografice;
- Comenzi ecografice și Rapoarte ecografice structurate;
- ferestre dedicate pentru comenzi și rapoarte, cu filtrare și previzualizare;
- sisteme de examinare pentru organe interne, sistem urinar, prostată,
  ginecologie, glande mamare, tiroidă, sarcină și ganglioni limfatici;
- atașarea imaginilor și a fișierelor video;
- șabloane de tipar configurabile și export PDF prin LimeReport;
- expedierea prin e-mail a comenzii, raportului și imaginilor;
- programarea pacienților cu mai multe investigații;
- rapoarte statistice, medicale și de formare a prețurilor;
- suport pentru SQLite și MariaDB;
- sincronizare opțională în fundal cu MariaDB, pe bază de UUID;
- interfață în limbile română, engleză și rusă.

## Versiunea 4.1.2

Adăugat site-ul organizației, migrarea bazei de date și progresul detaliat al actualizării.

Versiunea 4.1.1 corectează tipărirea listei de prețuri pe SQLite și MariaDB
și aranjarea comenzii pe A4 landscape, inclusiv separatorul central.

Versiunea 4.1.0 include migrarea schemei pacienților, compatibilitate revizuită
SQLite/MariaDB, sincronizare prin UUID, noile ferestre `OrderView` și
`ReportView`, îmbunătățirea programărilor, setări și logare mai robuste, precum
și corectări pentru validarea rapoartelor, tipărire, export PDF și pregătirea
mesajelor e-mail.

LimeReport a fost actualizat de la 1.7.14 la 1.7.23 și este compilat pentru Qt
6.9.3. Revizia upstream exactă și corecția USG pentru închiderea sigură sunt
documentate în [`third_party/LIMEREPORT.md`](third_party/LIMEREPORT.md).

Istoricul complet se află în
[`resources/RELEASES.md`](resources/RELEASES.md).

> Înaintea actualizării unei instalări existente, efectuați copii de siguranță
> pentru baza principală, baza de imagini și configurația aplicației. Migrarea
> unei baze partajate trebuie executată de un singur client, fără folosirea
> simultană a versiunilor USG vechi și noi.

## Pachete de instalare

Artefactele Linux ale unei versiuni pot include:

- installer `.run` bazat pe Qt Installer Framework;
- pachet Debian `.deb`;
- pachet portabil `.AppImage`.

Descărcați executabilele publicate numai din pagina
[GitHub Releases](https://github.com/debalex77/USG/releases) și verificați suma
SHA-256 furnizată înainte de instalare.

Copiile bazelor de date, bazele de test și fișierele de configurare ale
utilizatorului nu sunt incluse în pachetele release.

### Lansarea pachetului AppImage

În mod normal, runtime-ul AppImage folosește FUSE 2. Pe Debian 13 instalați
biblioteca compatibilă astfel:

```bash
sudo apt install libfuse2t64
chmod +x USG_v4.1.2-x86_64.AppImage
./USG_v4.1.2-x86_64.AppImage
```

Dacă FUSE nu poate fi instalat, folosiți modul de extragere oferit de runtime-ul
AppImage:

```bash
./USG_v4.1.2-x86_64.AppImage --appimage-extract-and-run
```

Această variantă extrage temporar pachetul și pornește mai lent, dar nu necesită
FUSE.

## Compilarea din sursă

Configurația de referință pentru versiunea 4.1.2 este:

| Componentă | Versiune / cerință |
|---|---|
| Qt | 6.9.3 |
| C++ | C++20 |
| Sistem de build | qmake |
| LimeReport | 1.7.23, revizie fixată și patch USG |
| OpenSSL | 3.x |
| Bază de date | SQLite sau MariaDB |

USG folosește intenționat qmake; proiectul nu furnizează fișiere CMake.

Pregătiți LimeReport și bibliotecile terțe conform instrucțiunilor din
[`third_party/LIMEREPORT.md`](third_party/LIMEREPORT.md), apoi executați:

```bash
git clone https://github.com/debalex77/USG.git
cd USG
mkdir -p build/Desktop_Qt_6_9_3-Release
cd build/Desktop_Qt_6_9_3-Release
/cale/către/Qt/6.9.3/gcc_64/bin/qmake ../../USG.pro CONFIG+=release
make -j2
```

Fluxul de creare a pachetelor Linux este implementat în
[`build_scripts/build_new`](build_scripts/build_new). Suplimentar față de
dependențele de compilare, acesta necesită CQtDeployer, Qt Installer Framework
și AppImageTool.

## Confidențialitatea datelor

Bazele pacienților, imaginile, configurațiile, parolele și logurile pot conține
informații confidențiale. Nu atașați date reale de producție la rapoartele de
eroare. Reproduceți problemele bazei de date pe o copie anonimizată și eliminați
datele personale, medicale și de autentificare înainte de transmitere.

## Licență și componente terțe

USG este distribuit sub GNU General Public License, versiunea 3 sau orice
versiune ulterioară. Consultați [`LICENSE.txt`](LICENSE.txt).

Proveniența și licențierea iconițelor și imaginilor sunt documentate în
[`third_party/THIRD_PARTY_ICONS.md`](third_party/THIRD_PARTY_ICONS.md).
Licențierea LimeReport, sursa corespunzătoare și patch-ul local sunt descrise
în [`third_party/LIMEREPORT.md`](third_party/LIMEREPORT.md).

## Raportarea problemelor

Problemele reproductibile pot fi raportate prin
[GitHub Issues](https://github.com/debalex77/USG/issues). Indicați versiunea USG,
sistemul de operare, motorul bazei de date și liniile relevante și anonimizate
din log. Nu publicați datele pacienților sau credențiale.
