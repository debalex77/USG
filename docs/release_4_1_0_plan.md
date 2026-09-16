# Plan de release în producție — USG 4.1.0

Data pregătirii: 15 septembrie 2026. Stare: candidat în pregătire; lansarea depinde de închiderea criteriilor de mai jos.

Planul se bazează pe codul și documentația locale. Testele consemnate în `task.md` sunt rezultate istorice, nu verificări executate din nou la pregătirea acestui plan. Data lansării, stațiile țintă și responsabilii nominali se stabilesc înainte de înghețarea candidatului.

## 1. Domeniul livrării

Păstrăm Qt 6.9.3, C++20 și qmake. Release-ul include migrarea pacienților la schema `patients`, programările și investigațiile multiple, jurnalele `OrderView`/`ReportView`, fluxurile documentelor, sincronizarea prin UUID, imaginile, e-mailul, setările și logarea descrise în `task.md`. Lista finală se confruntă cu diferențele față de ultima versiune distribuită.

Windows x64 este ținta principală propusă, conform README; Linux se califică separat dacă se distribuie pachetul `.deb`. macOS intră în livrare numai dacă există candidat și validare pe platformă. Succesul buildului Linux nu califică Windows.

Refactorizările viitoare ale organizațiilor, utilizatorilor și celorlalte domenii rămân în backlog. Orice funcție deja expusă utilizatorilor trebuie însă verificată sau retrasă controlat din candidat dacă este incompletă.

## 2. Blocaje și neconcordanțe identificate

| Constatare locală | Acțiune obligatorie | Dovadă de închidere |
|---|---|---|
| **Închis:** `USG.pro`, `src/common/version.h`, `version.txt` și metadatele Linux indică 4.1.0; `src/data/version.h` redundant a fost eliminat | Nu publicați modificarea `version.txt` înainte ca artefactele 4.1.0 să fie disponibile | Executabilul, About, installerul, pachetul Debian și notele indică 4.1.0 |
| **Închis:** `resources/RELEASES.md` conține secțiunea 4.1.0 | Revizuire editorială finală înainte de tag | Notele descriu migrarea, compatibilitatea, funcțiile și avertismentul de backup |
| Multe ștergeri/adăugări locale, inclusiv mutarea în `src/` | Revizuirea setului complet de modificări și includerea resurselor necesare | Commit identificabil și build din checkout curat |
| `task.md` lasă deschise atomicitatea salvării și concurența | Audit și teste pentru comenzi/rapoarte, erori SQL și numere concurente | Fără salvări parțiale sau coliziuni necontrolate |
| Testele de întrerupere a migrării, durată și profiluri sunt deschise | Executarea matricei de calificare de mai jos | Rezultate și loguri pentru candidatul final |
| **Închis:** proiectarea migrării și documentația jurnalelor au fost reconciliate cu implementarea curentă | Păstrarea documentației sincronizate cu eventualele corecții ulterioare | `docs/migration_4_1_0_design.md` și `docs/document_journal_replacement.md` descriu starea curentă |
| Împachetarea Windows nu face parte din candidatul Linux curent | Calificare și rețetă separată înaintea unei distribuiri Windows | Installer Windows reproductibil și validat pe un sistem curat |

## 3. Etape și responsabilități

Estimare orientativă: 5–8 zile lucrătoare după închiderea defectelor blocante; durata migrării se măsoară, nu se presupune.

| Etapă | Responsabil propus | Rezultat necesar |
|---|---|---|
| 1. Stabilirea domeniului și bazei de comparație — 0,5–1 zi | Responsabil release + dezvoltator | Versiuni instalate inventariate, platforme și motoare țintă, scope final |
| 2. Închiderea blocajelor și înghețarea codului — durată după audit | Dezvoltator | Commit candidat, metadate coerente, note de release |
| 3. Calificare migrare și regresie — 2–3 zile | QA + administrator BD | Matrice completă, restaurare demonstrată, durate măsurate |
| 4. Build și împachetare — 1 zi | Responsabil build | Artefacte instalabile, manifest, SHA-256 și rezultate pe sisteme curate |
| 5. Pilot — 1–2 zile de lucru | Administrator + utilizator pilot | Flux complet validat în utilizare reală și verificare după repornire |
| 6. Distribuire și supraveghere — 24–48 h | Responsabil release + suport | Actualizare pe loturi, fără defecte blocante |

Orice modificare a codului după calificare produce un candidat nou și repetarea verificărilor afectate. Pachetul distribuit trebuie să fie cel calificat, cu același hash.

## 4. Matrice obligatorie de calificare

Pentru fiecare scenariu se înregistrează: commit, platformă, motor și versiune BD, versiune sursă, set de date anonimizat, rezultat, durată și log. Se lucrează exclusiv pe copii izolate, inclusiv pentru sincronizare și e-mail.

- [ ] Instalare nouă și bază nouă, separat SQLite și MariaDB.
- [ ] Upgrade din 3.0.6, 3.0.7 și 4.0.1, dacă aceste versiuni există la utilizatorii țintă. Alte versiuni instalate primesc test dedicat sau traseu intermediar validat înainte de includerea în suport.
- [ ] Capturare înainte/după: număr de pacienți, comenzi, rapoarte, imagini, programări și relații; ID-uri, UUID-uri, numere anuale, indexuri, FK-uri și view-uri. Orice diferență trebuie explicată de migrare; zero pierderi și zero relații orfane noi.
- [ ] SQLite: integritate, `foreign_key_check`, eroare deliberată și întrerupere de proces; revenire și relansare fără date parțiale.
- [ ] MariaDB: întrerupere între fazele DDL, reluare din stări neambigue și oprire explicită la stare ambiguă; versiunea nu este marcată ca finală după eșec.
- [ ] Bază deja 4.1.0: a doua lansare nu modifică din nou datele; bază mai nouă: refuz controlat.
- [ ] Migrare pe volum reprezentativ: timp, spațiu temporar și durată de restaurare măsurate. Fereastra de mentenanță acoperă migrarea, verificarea și restaurarea, cu marjă.
- [ ] Pacient: creare, căutare, editare și istoric; documente vechi și noi; numerotare pe ani și sesiuni concurente.
- [ ] Comandă/raport: toate sistemele, salvare, redeschidere, validare și anulare; eroare în secțiune sau conexiune pierdută fără antet/date parțiale și fără duplicare la reîncercare.
- [ ] Programare: text liber, pacient existent, investigații multiple, creare comandă și ștergere.
- [ ] Tipărire și PDF: toate șabloanele livrate, diacritice, fonturi, logo, semnătură și ștampilă; imagini/video și atașamente.
- [ ] Sincronizare: pacient → comandă → raport → imagini, UUID-uri păstrate, retry fără duplicate, comportament offline și reconectare. E-mail numai către adresă de test controlată.
- [ ] Profil vechi/nou/curat: autentificare, „Memorează”, salvare/anulare setări, schimbare limbă și restart; căi invalide și lipsă drepturi cu mesaj corect.
- [ ] Jurnale, filtre, MDI, temele și limbile distribuite; închidere fără crash; loguri fără parole sau date de pacient inutile.

## 5. Build și pachet

- [ ] Build Release din checkout curat al commitului candidat, cu Qt 6.9.3/qmake și toolchain consemnat pentru fiecare platformă.
- [ ] Includerea bibliotecilor LimeReport/QtZint, driverelor SQL, OpenSSL, pluginurilor Qt, componentelor WebEngine/multimedia folosite, traducerilor, șabloanelor și resurselor necesare.
- [ ] Instalare și upgrade pe sistem fără mediul de dezvoltare; verificarea păstrării configurației, bazelor și șabloanelor personalizate, inclusiv la dezinstalare.
- [ ] Pachet fără baze de test, configurații personale, credențiale, loguri sau date reale. Licențe și metadate incluse; verificarea semnăturii dacă aceasta este folosită în distribuție.
- [ ] Manifest cu versiune, commit, toolchain, dependențe, platformă și SHA-256 pentru fiecare artefact.
- [ ] Verificarea mecanismului de actualizare pe fiecare platformă. Codul citește `version.txt` de pe `master`; pentru Linux construiește numele `USG_v4.1.0_Linux_amd64.deb` sub tagul `v4.1.0`. Verificarea numelor și rutelor Windows/macOS se face înainte de publicare.

## 6. Procedura de lansare

Workflow-ul `.github/workflows/release-linux.yml` construiește pachetele și le
încarcă automat astfel:

- lansarea manuală (`workflow_dispatch`) produce un artefact GitHub Actions cu
  retenție de 14 zile, fără a crea un release public;
- publicarea tagului `v4.1.0` construiește din acel tag, verifică potrivirea cu
  `version.txt`, creează un GitHub Release **draft** și încarcă automat `.run`,
  `.deb`, AppImage, sursa LimeReport și fișierele SHA-256;
- reluarea workflow-ului actualizează același draft și înlocuiește artefactele;
  un release deja publicat nu este suprascris automat;
- trecerea din draft în public rămâne o decizie manuală după verificarea
  descărcărilor și a sumelor SHA-256.

1. Confirmați toate criteriile Go/No-Go și lista stațiilor. Pregătiți pachetul anterior, backupul verificat și persoana care execută revenirea.
2. Anunțați fereastra de mentenanță. Opriți scrierile tuturor clienților către baza vizată și sincronizarea; nu permiteți clienți vechi pe schema migrată.
3. Faceți backup consistent pentru BD, `db_image`/atașamente, configurații, șabloane personalizate și materialul criptografic necesar restaurării. Pentru SQLite folosiți backup consistent sau copia cu toate conexiunile închise; pentru MariaDB folosiți procedura validată prin restaurare.
4. Instalați candidatul pe un pilot izolat. Dacă stațiile împart aceeași BD, unitatea pilot este întregul grup care folosește acea bază; nu se amestecă versiunile.
5. Executați migrarea printr-un singur client. Verificați versiunea 4.1.0, valorile de control și logurile înainte de permiterea lucrului.
6. Verificați autentificare, pacient istoric, comandă/raport, PDF și imagini. Reactivați sincronizarea numai după confirmarea compatibilității bazei cloud; verificați un flux complet controlat.
7. După 1–2 zile de pilot reușit, publicați artefactele finale și notele sub tagul `v4.1.0`, legat de commitul calificat. Verificați descărcarea și hashurile înainte de actualizarea indicatorului public `version.txt` la 4.1.0.
8. Distribuiți pe loturi de baze/stații; verificați fiecare lot înainte de următorul. Urmăriți 24–48 h migrarea, crashurile, salvarea, tipărirea și sincronizarea.

## 7. Revenire și oprirea distribuirii

Declanșatoare: migrare eșuată sau peste limita ferestrei validate, diferențe neexplicate de date, salvări parțiale, crash repetat în flux principal, imposibilitatea tipăririi sau sincronizare care alterează datele.

1. Opriți distribuirea, scrierile și sincronizarea; păstrați logurile și o copie a stării eșuate pentru diagnostic.
2. Dacă nu s-au produs date noi după upgrade, restaurați ansamblul verificat BD + imagini/atașamente + configurații și reinstalați pachetul anterior. Revenirea doar la executabilul vechi nu este suficientă după schimbarea schemei.
3. Dacă există date noi, exportați și reconciliați controlat modificările înainte de restaurare; nu suprascrieți aceste date automat. Dacă au ajuns în cloud, includeți și starea cloud în reconciliere.
4. Verificați integritatea, autentificarea și documentele după restaurare, apoi permiteți reluarea lucrului.
5. Retrageți semnalizarea actualizării pentru clienții neactualizați. Un defect necesită un candidat corectat și recalificat; nu înlocuiți în tăcere artefactele deja publicate cu alte binare.

## 8. Go/No-Go și închiderea release-ului

- [ ] Responsabil release, administrator BD și QA nominalizați; fereastră, limită de migrare și durată de restaurare consemnate.
- [ ] Niciun defect deschis de pierdere/corupere date, migrare, salvare, autentificare sau flux principal. Problemele minore acceptate au impact, soluție temporară și responsabil documentate.
- [ ] Matricea trecută pe candidatul final pentru toate platformele și versiunile sursă anunțate.
- [ ] Backup restaurat cu succes în mediu separat și procedură de revenire repetată.
- [ ] Commit, tag propus, pachete, hashuri și note revizuite; pilot acceptat înaintea distribuirii generale.
- [ ] După supraveghere: arhivarea dovezilor, consemnarea versiunilor instalate și transferarea problemelor neblocante în backlog.

Decizia actuală: **No-Go pentru publicare imediată**, până la verificarea și închiderea blocajelor. Acest document pregătește release-ul; nu confirmă că un candidat a fost calificat.
