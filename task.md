# Plan de modernizare USG

Acest fișier urmărește modernizarea proiectului **USG**. Proiectul **USG_old nu se
modifică** și este folosit numai ca sursă de comparație și pentru verificarea
compatibilității datelor existente.

Proiectul rămâne pe **Qt/C++ 6.9.3 și qmake**. Nu se introduce CMake.

## Reguli de lucru și bifare

- `[ ]` — sarcină neîncepută sau neverificată.
- `[~]` — sarcină începută, dar incompletă; se adaugă o explicație scurtă.
- `[x]` — sarcină implementată și verificată conform criteriilor ei de acceptare.
- O sarcină nu se bifează doar pentru că proiectul compilează.
- După fiecare grup de modificări se verifică compilarea, migrarea și integritatea
  datelor afectate.
- Modificările structurale ale bazei se testează întâi pe o copie, niciodată direct
  pe baza de producție.
- Fiecare migrare trebuie să fie tranzacțională, repetabilă în siguranță și să
  producă mesaje de diagnostic utile.
- SQLite și MariaDB se tratează explicit; succesul pe un motor nu presupune automat
  succesul pe celălalt.
- Nu se amestecă într-un singur pas redenumiri masive, schimbări de comportament și
  optimizări fără legătură.

## 0. Baza deja realizată pentru v4.0.1

- [x] Actualizarea versiunii proiectului la `4.0.1`.
- [x] Introducerea secvențelor anuale pentru numerele comenzilor și rapoartelor.
- [x] Generarea numerelor noi în format `n/YYYY`.
- [x] Conversia la actualizare a numerelor istorice strict numerice în `n/YYYY`.
- [x] Păstrarea neschimbată a numerelor istorice care sunt deja formatate.
- [x] Separarea secvenței anuale a comenzilor de secvența anuală a rapoartelor.
- [x] Eliminarea copierii numărului comenzii în numărul raportului.
- [x] Adăugarea constrângerilor de unicitate pe număr și an.
- [x] Testarea migrării SQLite pe o bază temporară.
- [x] Compilarea și linkarea proiectului cu Qt 6.9.3/qmake după modificări.
- [x] Testarea completă a migrării v4.0.1 pe o copie anonimizată a bazei reale:
      migrare structurală, rollback și flux funcțional prin aplicație.
- [x] Testarea migrării v4.0.1 pe MariaDB: duplicatele istorice au fost păstrate,
      inițializarea secvențelor a fost adaptată pentru formatul strict `n/YYYY`, iar
      fluxurile funcționale au fost confirmate în aplicație.
- [x] Crearea automată a tabelei `doc_sequences` când lipsește dintr-o bază veche.
- [x] Protejarea migrării SQLite v4.0.1 prin tranzacție și rollback.
- [x] Prevenirea marcării bazei ca actualizată când versiunea lipsește sau salvarea
      noii versiuni eșuează.
- [x] Afișarea stării migrării v4.0.1 în bara de progres din statusbar.
- [x] Asigurarea vizibilității barei din statusbar: dimensiune fixă lizibilă,
      procent afișat și menținerea rezultatului final timp de 10 secunde.
- [x] Poziționarea barei înaintea textului și actualizarea granulară în loturi în
      timpul conversiei comenzilor și rapoartelor, fără blocarea îndelungată la 0%.
- [x] Ajustarea dimensiunii barei de progres din statusbar la `110×16`.
- [x] Actualizarea progresului UUID la fiecare 100 de înregistrări și după fiecare
      tabel verificat pentru indexul unic.
- [x] Compatibilitatea autorizării cu schema pre-v4.0.1 (fără citirea prematură a
      `users.uuid`) și folosirea conexiunii principale reale de către modelele SQL.
- [x] Recrearea la migrare a view-urilor pentru organizații, contracte, utilizatori,
      doctori, asistente și tipuri de preț, inclusiv sintaxa SQLite validă.
- [x] Repararea automată a view-urilor lipsă în bazele deja marcate `4.0.1`, fără
      modificarea datelor și fără repetarea migrării.
- [x] Precompletarea independentă a numelui utilizatorului când `Memorează` este
      bifat și sincronizarea imediată a cheilor de autorizare în configurație.
- [x] Afișarea etapelor și statisticilor agregate în timpul migrării, fără date
      personale sau conținut medical.
- [x] Îmbunătățirea diagnosticului pentru interogările `QueryRolesModel` eșuate.
- [x] Deschiderea și validarea conexiunii principale înainte de autorizare, inclusiv
      după transferarea aplicației, cu interogări legate explicit de conexiune.
- [x] Păstrarea numelor complete ale configurațiilor care conțin puncte (de exemplu
      `test_v4.0.1.conf`) și prevenirea relansării eronate în modul `firstLaunch`.

## 1. Inventarierea înaintea refactorizării

- [x] Inventarierea tuturor tabelelor SQLite și MariaDB.
- [x] Inventarierea coloanelor, tipurilor, valorilor implicite și regulilor `NULL`
      pentru domeniul pacientului; inventarul complet rămâne iterativ pe domenii.
- [x] Inventarierea cheilor primare, cheilor externe și indexurilor pentru domeniul
      pacientului.
- [x] Inventarierea view-urilor și triggerelor care depind de tabelele redenumite.
- [x] Inventarierea fișierelor SQL din resurse și a interogărilor scrise în C++.
- [x] Inventarierea claselor, modelelor, formularelor și rapoartelor care folosesc
      numele `pacient`, `pacients` sau `id_pacients`.
- [x] Inventarierea sincronizării și importului/exportului care folosesc schema veche.
- [x] Identificarea preliminară a coloanelor duplicate, neutilizate sau cu sens neclar.
- [x] Identificarea diferențelor reale dintre schema SQLite și schema MariaDB pentru
      domeniul pacientului.
- [~] Salvarea valorilor de control dintr-o copie reală; valorile principale au
      fost consemnate pentru copia SQLite (6.358 pacienți, 8.110 comenzi și 7.476
      rapoarte), dar inventarul tabelelor dependente nu este încă complet:
  - [x] numărul pacienților;
  - [x] numărul comenzilor;
  - [x] numărul rapoartelor;
  - [ ] numărul înregistrărilor din tabelele dependente;
  - [ ] numărul relațiilor invalide/orfane;
  - [ ] valorile maxime și duplicatele pentru identificatori și numere de document.

### Criteriu de finalizare

- [ ] Există o listă completă `obiect actual -> obiect propus -> utilizări afectate`.
- [ ] Nu se începe migrarea până când lista nu este revizuită și aprobată.

## 2. Convenția unitară de denumire

- [x] Stabilirea regulii pentru numele tabelelor: engleză, plural, `snake_case`.
- [x] Stabilirea regulii pentru coloane: engleză, `snake_case`.
- [x] Stabilirea regulii pentru chei externe: `<entitate_singular>_id`.
- [x] Stabilirea regulii pentru clase și structuri C++: `PascalCase`.
- [x] Stabilirea regulii pentru metode și variabile locale: `camelCase`.
- [x] Păstrarea convenției `m_` pentru membrii claselor.
- [x] Stabilirea regulii de păstrare a termenilor medicali și textelor UI care nu
      trebuie traduși sau schimbați arbitrar.
- [~] Crearea dicționarului de redenumire, inclusiv cel puțin:
  - [x] `pacients` -> `patients`;
  - [x] `id_pacients` -> `patient_id`;
  - [ ] `id_organizations` -> `organization_id` (etapa organizațiilor);
  - [ ] `id_users` -> `user_id` (etapa utilizatorilor);
  - [ ] numele tabelelor pentru comenzi și rapoarte;
  - [ ] numele coloanelor de dată, stare și marcaj de ștergere.
- [x] Verificarea riscului redenumirilor din domeniul pacientului asupra rapoartelor,
      exportului și sincronizării.
- [x] Aprobarea dicționarului pentru domeniul pacientului înainte de implementare:
      `middle_name` se păstrează, iar `fullNamePacients` se elimină.

## 3. Consolidarea mecanismului de actualizare

- [x] Analiza responsabilităților curente ale clasei `UpdateReleasesApp`.
- [x] Separarea logică a validării/dispecerizării versiunii de corpurile migrărilor,
      prin registrul ordonat de pași.
- [x] Definirea unei migrări distincte pentru fiecare versiune în registrul de pași.
- [~] Executarea fiecărei migrări într-o tranzacție completă: realizată pentru
      pașii SQLite; DDL-ul MariaDB este reluabil deoarece motorul poate face commit
      implicit la operațiile structurale.
- [~] Anularea tranzacției la prima instrucțiune eșuată: implementată pentru pașii
      tranzacționali; MariaDB se oprește la eroare și reia sigur pașii deja aplicați.
- [x] Înregistrarea versiunii noi numai după succesul complet al lanțului de migrări.
- [~] Adăugarea jurnalizării pentru început, pas curent, eroare, rollback și succes;
      jurnalizarea generală este implementată, detalierea va continua în 4.1.0.
- [~] Adăugarea verificărilor pre-migrare:
  - [x] motorul bazei și versiunea schemei sunt recunoscute;
  - [~] tabelele și coloanele așteptate există; tabelele obligatorii pentru 4.0.1
        sunt verificate, iar verificările de coloane vor fi definite pentru 4.1.0;
  - [ ] baza nu conține incompatibilități cunoscute;
  - [ ] există spațiu și drepturi suficiente pentru operație/backup.
- [~] Adăugarea verificărilor post-migrare:
  - [x] numărul comenzilor și rapoartelor este păstrat în migrarea 4.0.1;
  - [x] relațiile pacient–comandă–raport sunt valide;
  - [x] UUID-urile obligatorii sunt prezente, au 16 octeți și nu sunt duplicate;
  - [~] indexurile, view-urile și trigger-ele sunt valide; view-urile și obiectele
        4.0.1 sunt verificate, iar auditul complet va fi definit pentru 4.1.0;
  - [ ] versiunea finală este cea așteptată.
- [x] Definirea comportamentului la întreruperea aplicației în timpul migrării.
- [~] Verificarea reluării sigure după rollback; 4.0.1 a fost verificată, iar scenariul
      pe faze pentru DDL MariaDB 4.1.0 este proiectat și urmează implementarea.
- [x] Stabilirea versiunii pentru prima migrare structurală: `4.1.0`.

## 4. Migrarea `pacients` -> `patients`

- [x] Crearea unei copii de test dintr-o bază reprezentativă, pregătită de utilizator.
- [~] Verificarea relațiilor orfane înainte și după migrare este implementată;
      curățarea automată nu se face pentru a nu elimina date fără confirmare.
- [x] Implementarea migrării SQLite 4.1.0, tranzacțională și validată pe o bază
      sintetică; testul pe copia bazei reprezentative rămâne separat.
- [x] Implementarea migrării MariaDB 4.1.0 în pași DDL reluabili; testul pe
      instanța MariaDB de test rămâne separat.
- [x] Redenumirea tabelei `pacients` în `patients` în migrare, schema pentru baze
      noi, resursele SQL și consumatorii runtime.
- [x] Redenumirea cheilor externe conform dicționarului aprobat la `patient_id`.
- [x] Refacerea/păstrarea cheilor primare și străine.
- [x] Refacerea/păstrarea indexurilor.
- [x] Actualizarea view-ului de completare a pacienților.
- [x] Eliminarea triggerelor vechi dependente de `fullNamePacients`; cache-ul și
      trigger-ele lui nu mai sunt create pentru baze noi.
- [x] Actualizarea SQL-ului din resurse pentru schema canonică a pacientului.
- [x] Actualizarea interogărilor SQL din C++ pentru schema canonică.
- [x] Actualizarea modelelor și workerelor pentru pacienți.
- [x] Actualizarea sincronizării pentru pacienți; importul/exportul va fi bifat
      numai după verificarea manuală a fluxurilor disponibile.
- [x] Verificarea păstrării tuturor ID-urilor pacienților pe copiile testate.
- [x] Verificarea relațiilor pacient-comandă-raport.
- [x] Verificarea deschiderii și salvării pacienților existenți.
- [x] Verificarea creării unui pacient nou după migrare și sincronizarea lui cloud.
- [x] Verificarea tranzacției SQLite pe o bază sintetică, inclusiv relațiile și
      eliminarea triggerelor cache; simularea explicită a erorii rămâne de făcut.

### Criteriu de finalizare

- [x] Numărul pacienților și identificatorii coincid înainte și după migrare.
- [x] Nu există comenzi sau rapoarte fără pacient valid în copiile verificate.
- [x] Migrarea trece pe SQLite și MariaDB.
- [x] Proiectul compilează și fluxurile principale funcționează.

## 5. Revizuirea coloanelor pe domenii

### 5.1 Pacienți

- [x] Revizuirea numelor și sensului tuturor coloanelor; `middle_name` este păstrat,
      iar tabela/cache-ul `fullNamePacients` a fost eliminat.
- [~] Revizuirea tipurilor SQL, lungimilor și valorilor implicite este realizată
      pentru schema migrată; validarea datelor istorice rare rămâne deschisă.
- [x] Revizuirea câmpurilor obligatorii și a validării în C++.
- [ ] Identificarea și tratarea datelor invalide existente.
- [x] Migrarea și testarea pe copii SQLite și MariaDB.

### 5.2 Organizații, medici și utilizatori

- [ ] Revizuirea denumirilor și relațiilor.
- [ ] Revizuirea constrângerilor și indexurilor.
- [ ] Migrarea și testarea pe copie reală.

### 5.3 Comenzi

- [~] Revizuirea coloanelor și a relațiilor cu pacientul, organizația, contractul și
      raportul; relațiile runtime sunt corectate, redenumirea completă rămâne etapă viitoare.
- [x] Verificarea compatibilității cu numerotarea `n/YYYY`.
- [x] Verificarea sincronizării și tipăririi pe SQLite și MariaDB.
- [x] Migrarea și testarea pe copii de test.

### 5.4 Rapoarte ecografice

- [~] Revizuirea tabelei principale și a tabelelor pe sisteme/organe; salvarea și
      revalidarea sistemelor au fost corectate, redenumirea completă rămâne deschisă.
- [x] Verificarea legăturilor cu pacientul și comanda.
- [x] Verificarea imaginilor și formelor LimeReport; lipsa opțională a tabelei video
      este tratată fără blocarea raportului.
- [x] Verificarea compatibilității cu numerotarea `n/YYYY`.
- [x] Migrarea și testarea pe copii SQLite și MariaDB.

### 5.5 Contracte, prețuri și alte cataloage

- [ ] Revizuirea denumirilor și relațiilor.
- [ ] Eliminarea redundanțelor numai după confirmarea utilizărilor.
- [ ] Migrarea și testarea pe copie reală.

## 6. Refactorizarea denumirilor și variabilelor C++

- [x] Generarea unei liste de simboluri care trebuie redenumite.
- [~] Redenumirea etapizată pe module, nu global într-un singur pas: enum-urile și
      mapările coloanelor pacientului, apoi modelele și filtrele jurnalelor au fost
      uniformizate fără schimbarea schemei.
- [~] `pacient`/`idPacient` -> `patient`/`patientId`, unde nu afectează texte medicale
      sau compatibilitatea externă: finalizat în modelele/filtrele jurnalelor;
      `SyncPatientDataWorker` și variabilele sale interne au fost uniformizate;
      fluxul nou `OrderDialog`/`SyncPatientWorker` și enumul intern
      `BaseSqlQueryModel::Patients` au fost uniformizate;
      `PatientDataStructure::firstName` și `middleName` înlocuiesc abrevierile
      `fname`/`mname`, iar `middleName` este păstrat la salvare și sincronizare;
      identificatorii Qt Designer și semnalele publice existente se păstrează până
      la refactorizarea controlată a interfeței.
- [ ] Eliminarea conversiilor nesigure dintre `QString` și tipuri numerice.
- [ ] Tratarea numărului de document `n/YYYY` ca text/valoare structurată, nu `int`.
- [ ] Revizuirea tipurilor pentru ID-uri, date, sume și valori opționale.
- [~] Eliminarea variabilelor și claselor neutilizate: au fost eliminate modelele
      vechi confirmate, `ListDocReportOrder`, `DocOrderEcho` și `DocReportEcho`;
      auditul continuă modular.
- [ ] Eliminarea duplicării stării între interfață, model și obiectul documentului.
- [~] Revizuirea duratei de viață și ownership-ului Qt: corectată în fluxul e-mail,
      logging și `AppSettings`; auditul celorlalte module rămâne deschis.
- [x] Compilarea după fiecare modul redenumit.
- [~] Testarea manuală a fluxurilor afectate: comenzile și rapoartele au fost
      confirmate pe SQLite și MariaDB; `AppSettings` necesită retestarea finală.
- [x] Corectarea verificării duplicatelor fără IDNP din `PatientDataSaverWorker`:
      se execută acum interogarea pregătită pentru nume, prenume și data nașterii.

## 7. Revizuirea clasei `AppSettings`

- [x] Inventarierea tuturor cheilor citite și scrise direct de `AppSettings` și a
      consumatorilor care modifică individual preferințele.
- [x] Identificarea cheilor duplicate, neuniforme și neutilizate; denumirile
      existente rămân neschimbate pentru compatibilitatea fișierelor `.conf`.
- [x] Documentarea tipului, valorii implicite și sensibilității fiecărei chei în
      `docs/appsettings_schema.md`.
- [~] Separarea conceptuală a categoriilor:
  - [~] setările generale ale aplicației sunt grupate, dar persistența rămâne în dialog;
  - [x] citirea configurației este separată de deschiderea bazei de date;
        conexiunea principală este gestionată exclusiv de `AppController`;
  - [x] preferințele de memorare a utilizatorului și mesajele au API tipizat;
  - [ ] serverul și sincronizarea;
  - [ ] căile locale;
  - [ ] tipărirea și configurările medicale.
- [~] Definirea unei interfețe tipizate pentru citirea/scrierea setărilor:
      memorarea utilizatorului și vizibilitatea mesajelor folosesc operații
      dedicate; cheile, valorile implicite, validările tipizate și scrierea
      grupurilor INI au fost extrase în `AppSettingsStore`; citirea profilului
      construiește acum un `ProfileData` complet înainte ca valorile să fie
      aplicate în starea globală, iar salvarea profilului este executată tot prin
      `AppSettingsStore`. Dialogul doar colectează/aplică datele și afișează
      erorile; testarea manuală a compatibilității profilurilor rămâne deschisă.
- [x] Centralizarea mapării formularului în `ProfileData` și a aplicării
      `ProfileData` în starea globală; încărcarea și salvarea nu mai conțin două
      liste independente ale acelorași câmpuri.
- [x] Centralizarea mapării `ProfileData` în formular; `readSettings()` gestionează
      numai încărcarea, blocarea temporară a semnalelor și starea de modificare a
      dialogului.
- [x] Centralizarea valorilor implicite folosite de configurația principală.
- [x] Auditarea migrării cheilor istorice: comparația cu `USG_old` confirmă că
      grupurile și cheile configurației principale nu au fost redenumite; nu este
      necesară o migrare și nu se introduc aliasuri artificiale.
- [x] Păstrarea compatibilității cu setările utilizatorilor existenți: numele
      grupurilor și cheilor `.conf` nu au fost schimbate.
- [~] Revizuirea stocării parolelor și datelor sensibile: parola cloud folosește
      criptare autentificată și flux de reintroducere; acreditările conexiunii
      principale folosesc încă mecanismul istoric compatibil.
- [~] Verificarea comportamentului la fișier lipsă, JSON invalid și valori
      incompatibile: indexurile INI și retenția logurilor sunt validate cu fallback
      și avertizare; eroarea de scriere/format păstrează dialogul deschis și starea
      modificată; validarea fișierelor JSON ale ferestrelor rămâne separată.
- [x] Normalizarea limbii aplicației la una dintre valorile suportate (`ru-RU` sau
      `ro-RO`) pentru lansare nouă, profil necunoscut și profil existent.
- [x] Validarea portului MariaDB în intervalul `1..65535` și menținerea exclusivă a
      stărilor `thisMySQL`/`thisSqlite` la schimbarea tipului conexiunii.
- [x] Gestionarea explicită a duratei de viață pentru `QSettings` și
      `QTranslator`: operațiile `QSettings` folosesc instanțe locale,
      `QTranslator` este membru direct al dialogului, iar obiectul `PopUp`
      construit, dar neutilizat de dialog, a fost eliminat.
- [x] Modelul listei de loguri este deținut direct de `AppSettings`; a fost
      eliminată alocarea manuală și dependența de parentarea `QObject`.
- [x] Constructorul și salvarea `AppSettings` nu mai execută comenzi shell pentru
      crearea logurilor (`pkexec`, `chmod 777`, `touch`); crearea directorului și
      deschiderea fișierului aparțin exclusiv clasei `LogManager`.
- [x] Corectarea ramurilor multiplatformă din `AppSettings`: macOS folosește
      starea globală reală pentru căile profilului/logului, ramurile Windows
      folosesc macro-ul Qt `Q_OS_WIN`, iar calea de configurare duplicată și
      neutilizată a fost eliminată.
- [x] Crearea directorului JSON pentru starea interfeței a fost mutată din
      constructorul `AppSettings` în inițializarea `AppController`, este executată
      uniform pe toate sistemele și raportează explicit eșecul.
- [x] Derivarea căii logului din numele profilului și detectarea profilului lipsă
      folosesc un singur flux comun tuturor sistemelor, fără trei implementări
      divergente în constructor.
- [x] Modificarea numărului de arhive log păstrate marchează acum dialogul ca
      modificat; închiderea după schimbarea exclusivă a retenției solicită
      salvarea la fel ca pentru celelalte câmpuri.
- [x] Instanța `AppSettings` din fluxul de pornire nu mai este alocată cu `new`
      fără eliberare; are durată de viață automată în `AppController::run()` și
      este distrusă înaintea obiectului `QApplication`.
- [x] Eliminarea aliasului membru `AppController::m_appSettings`; dialogul este
      transmis explicit prin referință fluxului de lansare, fără pointer care ar
      deveni invalid după terminarea metodei `run()`.
- [x] Instanța `AppSettings` deschisă din `MainWindow` este urmărită prin
      `QPointer`: după `WA_DeleteOnClose` referința se golește automat, iar
      apăsările repetate reactivează dialogul existent în loc să creeze duplicate.
- [x] Corectarea referinței invalide din inițializarea SQLite pe macOS:
      `setDefaultPathSqlite()` actualizează explicit
      `globals().pathLogAppSettings`.
- [x] Căile implicite pentru șabloane și rapoarte sunt aplicate independent;
      lipsa uneia nu mai suprascrie cealaltă cale personalizată deja configurată.
- [x] Validarea căilor locale înainte de salvare: directoarele pentru șabloane și
      rapoarte trebuie să existe și să fie lizibile; directorul video este
      opțional, dar dacă este configurat trebuie să fie lizibil și inscriptibil.
- [x] Eliminarea obiectului `DataBase` persistent din `AppSettings`: operația
      explicită de creare SQLite folosește o instanță locală, iar codificarea
      compatibilă a setărilor folosește funcțiile statice.
- [x] Anularea selectării directoarelor pentru șabloane, rapoarte sau video nu mai
      golește calea existentă și nu mai produce o modificare falsă a formularului.
- [x] Numele profilului derivat din baza MariaDB/SQLite este validat înainte de
      salvare; separatorii de cale și caracterele de control sunt respinse.
- [x] Extragerea codec-ului istoric din `DataBase` în `LegacySettingsCodec`;
      metodele publice vechi ale clasei `DataBase` rămân compatibile și deleagă
      către aceeași implementare, fără modificarea valorilor existente.
- [x] Teste QtTest izolate pentru `AppSettingsStore`: profil legacy MariaDB,
      round-trip SQLite, profil curat, fallback pentru valori numerice invalide,
      respingerea datelor codificate deteriorate și păstrarea utilizatorului
      memorat la salvarea generală — 8 teste trecute din 8.
- [ ] Testarea pornirii cu setări vechi, setări noi și profil curat.

## 8. Integritatea salvării comenzilor și rapoartelor

- [ ] Revizuirea tranzacției complete din `OrderDialog`.
- [ ] Revizuirea tranzacției complete din `ReportDialog`.
- [ ] Garantarea că antetul și toate secțiunile documentului se salvează atomic.
- [ ] Restabilirea corectă a stării interfeței după eroare.
- [ ] Prevenirea documentelor salvate parțial.
- [ ] Verificarea comportamentului la pierderea conexiunii MariaDB.
- [ ] Verificarea coliziunilor de număr în sesiuni/utilizatori concurenți.
- [ ] Verificarea repetării operației după rollback.

## 8.1. Înlocuirea jurnalelor vechi de comenzi și rapoarte

`ListDocReportOrder` a fost înlocuit cu două ferestre cu responsabilități clare:
`OrderView` pentru comenzi și `ReportView` pentru rapoarte. `OrderView` rămâne
componenta activă și nu trebuie eliminată; ținta veche de eliminare a ei a fost
abandonată după verificarea funcțională.

- [x] Inventarierea comparativă a funcțiilor din `ListDocReportOrder` și `OrderView`:
  - [x] încărcare, paginare și sortare;
  - [x] filtre și memorarea coloanelor;
  - [x] creare, deschidere și ștergere document;
  - [x] tipărire, previzualizare și expediere prin e-mail;
  - [x] creare raport din comandă și deschidere comandă din raport;
  - [x] meniuri contextuale, istoric pacient și previzualizare imagini.
- [x] Separarea jurnalelor în `OrderView` și `ReportView`, fără introducerea unei
      clase comune monolitice.
- [~] Separarea logicii care nu aparține interfeței în componente mici:
  - [x] configurarea și persistența filtrelor, perioadei și coloanelor;
  - [x] acțiunile principale asupra documentelor;
  - [~] tipărirea și exportul/e-mailul: e-mailul rămâne intenționat numai în `OrderView`;
  - [x] încărcarea datelor și adaptarea SQL pentru SQLite/MariaDB prin loadere.
- [x] Folosirea modelelor `OrderJournalModel`/`OrderJournalLoader` și
      `ReportJournalModel`/`ReportJournalLoader`.
- [x] Implementarea și verificarea funcțională a `OrderView`.
- [x] Implementarea și verificarea funcțională a `ReportView`: creare/deschidere,
      filtrare, perioadă persistentă, coloane, preview concluzie și imagini,
      printare și eliminarea raportului/comenzii asociate conform alegerii utilizatorului.
- [x] Comutarea `MainWindow` la `OrderView` și `ReportView`.
- [~] Membrul istoric `list_report` există încă în `MainWindow`, dar tipul său este
      `ReportView`; redenumirea membrului nu este necesară funcțional și rămâne cleanup.
- [x] Eliminarea din proiect a `ListDocReportOrder` (`.h`, `.cpp`, `.ui`).
- [x] Păstrarea `OrderView` ca jurnal activ; nu mai este candidat pentru eliminare.
- [x] Eliminarea claselor vechi `DocOrderEcho` și `DocReportEcho`; documentele sunt
      gestionate de `OrderDialog` și `ReportDialog`.
- [x] Curățarea intrărilor qmake, include-urilor, conexiunilor și cheilor vechi de
      configurare rămase fără utilizatori.
- [x] Compilare completă Qt 6.9.3/qmake după comutarea ferestrelor.
- [x] Testare manuală pe SQLite și MariaDB pentru comenzile și rapoartele tuturor
      sistemelor, inclusiv revalidare și printare, confirmată de utilizator.

### Criteriu de finalizare

- [x] Comenzile și rapoartele se deschid prin `OrderView` și `ReportView`.
- [x] Funcțiile utilizate din `ListDocReportOrder` au echivalent verificat.
- [x] Nicio sursă activă sau intrare qmake pentru `ListDocReportOrder`,
      `DocOrderEcho` ori `DocReportEcho` nu mai există; traducerile și directoarele
      build vechi pot conține referințe istorice generate.
- [x] Clasele vechi au fost eliminate după testarea ambelor motoare SQL.

## 8.2. Sincronizare cloud, imagini și e-mail

- [x] UUID-urile create în SQLite la migrarea 4.0.1 sunt transferate în MariaDB,
      astfel aceleași entități au UUID identic în ambele baze.
- [x] Sincronizarea pacientului nou după salvarea locală.
- [x] Sincronizarea comenzii după creare și modificare, cu maparea relațiilor prin UUID.
- [x] Sincronizarea raportului după creare și modificare.
- [x] Migrarea/compatibilizarea `db_image`, inclusiv `patient_id`, UUID pentru imagini
      și eliminarea view-ului invalid dependent de vechea tabelă `pacients`.
- [x] Sincronizarea imaginilor raportului și actualizarea indicatorului de imagine
      din jurnalul comenzilor.
- [x] Tratarea explicită a absenței tabelei opționale `reportVideo`.
- [x] Crearea compatibilă a tabelei `onlineAccount` în migrarea 4.0.1.
- [x] Refacerea fluxului `OrderView -> DocEmailExporterWorker -> AgentSendEmail`.
- [x] Exportul comenzii, raportului și imaginilor cu nume de fișiere sigure.
- [x] Deschiderea atașamentelor PDF și imagine în funcție de sistemul de operare.
- [x] Eliminarea coruperii memoriei/double-free din fluxul e-mail; corecția necesară
      în LimeReport a fost identificată și compilată separat cu acordul utilizatorului.
- [x] Semnătura și ștampila doctorului sunt incluse corect în PDF-urile trimise.
- [x] Fluxurile pacient–comandă–raport și sincronizarea au fost confirmate pe
      copiile SQLite și MariaDB.

## 8.3. Programarea pacienților

- [x] Codul programărilor a fost mutat și consolidat în `documents/AppointmentDialog`.
- [x] Tabela `registrationPatients` este migrată la `patientAppointments` în 4.0.1.
- [x] Pacientul poate fi ales cu model/completer sau introdus ca text liber.
- [x] Tasta Enter după selectarea pacientului continuă editarea în următoarea
      secțiune a rândului.
- [x] Investigațiile multiple sunt păstrate în tabela copil
      `patientAppointmentInvestigations`.
- [x] Salvarea și actualizarea programării folosesc coloanele și relațiile noi.
- [x] Crearea `OrderDialog` din programare transferă toate investigațiile selectate.
- [x] Eliminarea oferă alegerea între rândul curent și documentul întreg al zilei.
- [x] Migrarea și operațiile CRUD au fost confirmate de utilizator pe baza de test.

## 8.4. Modele, cataloage și infrastructură

- [x] `BaseAbstractModel` a fost păstrat ca bază comună pentru modelele de catalog.
- [x] Au fost introduse modele dedicate: `OrderInvestigationModel`,
      `OnlineAccountModel` și `OrganizationContractModel`.
- [x] `CatalogTableEditor` rămâne editorul generic bazat direct pe
      `BaseAbstractModel`.
- [x] Modelele abstracte/SQL vechi confirmate fără utilizatori au fost eliminate
      din surse și din qmake.
- [x] `LogManager` scrie și în Debug, păstrând simultan ieșirea în Application Output.
- [x] Scrierea logului este protejată pentru fire multiple.
- [x] Rotirea și retenția logurilor folosesc profilul configurației curente și
      operații Qt portabile.
- [x] Loggerul este reinițializat după încărcarea sau schimbarea căii din setări.
- [x] Compilarea Debug cu Qt 6.9.3/qmake a fost verificată după aceste schimbări.

## 9. Suita de verificare a migrărilor

- [x] Pregătirea unei baze SQLite minimale pentru teste rapide.
- [x] Pregătirea unei copii anonimizate/reprezentative a bazei reale.
- [x] Pregătirea unei baze MariaDB de test.
- [~] Definirea verificărilor automate înainte/după fiecare migrare: verificările
      de număr, relații, UUID, indexuri și view-uri sunt implementate; suita nu este
      încă extrasă într-un executabil separat de teste.
- [~] Testarea actualizării de la versiunile 3.0.6/3.0.7 la 4.0.1 și 4.1.0 este
      realizată; alte versiuni vechi acceptate trebuie inventariate explicit.
- [x] Testarea executării repetate/reluării mecanismului de actualizare.
- [x] Testarea unei erori deliberate la mijlocul migrării SQLite și a rollback-ului.
- [ ] Testarea unei întreruperi a procesului.
- [ ] Măsurarea timpului de migrare pentru volumul real de date.
- [x] Verificarea manuală a unui eșantion de pacienți, comenzi și rapoarte vechi.
- [x] Verificarea tipăririi și exportului după migrare pe SQLite și MariaDB.

## 10. Ordinea de execuție recomandată

- [~] Pasul 1: inventarierea este completă pentru pacienți și documente; celelalte
      domenii vor fi inventariate înaintea fiecărei migrări.
- [~] Pasul 2: dicționarul este aprobat și aplicat pentru pacienți; organizațiile,
      utilizatorii și documentele rămân pentru etapele lor.
- [~] Pasul 3: `UpdateReleasesApp` are pași versionați și verificări; tranzacțiile
      și precondițiile suplimentare rămân de consolidat.
- [x] Pasul 4: testarea completă a v4.0.1 pe copii reale.
- [x] Pasul 5: migrarea `pacients` -> `patients` în versiunea 4.1.0.
- [~] Pasul 6: actualizarea simbolurilor C++ strict aferente pacienților; primul lot
      (`CatalogType::Patients`, `PatientsColumns`, `PatientSearchColumns`) este finalizat.
- [x] Pasul 7: testarea funcțională principală SQLite și MariaDB.
- [x] Pasul 8: înlocuirea `ListDocReportOrder` cu `OrderView` și `ReportView` și
      eliminarea claselor vechi de document.
- [~] Pasul 9: revizuirea `AppSettings` este în desfășurare; cheile și API-urile
      punctuale au fost centralizate, iar testele profilurilor rămân necesare.
- [ ] Pasul 10: continuarea pe celelalte domenii, câte unul pe rând.

## Jurnalul etapelor

La încheierea fiecărei etape se adaugă o intrare scurtă:

```text
Data:
Etapa:
Fișiere/componente afectate:
Migrare testată pe:
Compilare:
Teste efectuate:
Rezultat și observații:
```

### 2026-09-03 — Numerotarea anuală v4.0.1

- Etapa: introducerea numerelor `n/YYYY` și actualizarea datelor istorice numerice.
- Componente afectate: schema documentelor, migrarea versiunii, dialogurile de
  comandă și raport.
- Migrare testată pe: bază SQLite temporară.
- Compilare: reușită cu Qt 6.9.3/qmake.
- Observații: verificarea pe baza reală și testul MariaDB sunt încă necesare.

### 2026-09-05 — Migrare SQLite pe copie anonimizată reală

- Etapa: audit și test structural al migrării v4.0.1.
- Componente afectate: `UpdateReleasesApp` și salvarea versiunii în `MainWindow`.
- Migrare testată pe: clonă temporară a `test/test.sqlite3`; copia de referință nu a
  fost modificată.
- Date păstrate: 6.358 pacienți, 8.110 comenzi și 7.476 rapoarte.
- Rezultat: toate comenzile și rapoartele au fost convertite în `n/YYYY`, cele două
  indexuri anuale și 11 rânduri de secvență au fost create, fără neconcordanțe.
- Rollback: verificat prin eroare deliberată; schema și cele 8.110 numere de comandă
  au revenit integral la starea inițială.
- Compilare: Release reușită cu Qt 6.9.3/qmake.
- Observații: testul funcțional prin aplicație și testul MariaDB rămân necesare.

### 2026-09-05 — Migrare MariaDB și verificare funcțională finală v4.0.1

- Etapa: migrarea și verificarea funcțională pe ambele motoare SQL.
- Migrare testată pe: copii de test SQLite și MariaDB pregătite de utilizator.
- MariaDB: UUID-urile, indexurile, secvențele anuale și view-urile au fost create;
  duplicatele istorice au rămas nemodificate și sunt indexate fără unicitate.
- Compatibilitate SQL: jurnalul comenzilor și încărcarea pacientului folosesc acum
  expresii distincte pentru SQLite și MariaDB.
- Rezultat: lansarea, autorizarea, migrarea și deschiderea comenzilor au fost
  confirmate de utilizator pe SQLite și MariaDB.
- Compilare: reușită cu Qt 6.9.3/qmake.

### 2026-09-10 — Refactorizarea programărilor pacienților

- [x] Tabela `registrationPatients` a fost redenumită în `patientAppointments`.
- [x] Au fost adăugate coloanele nullable `patient_id` și `investigation_id`.
- [x] Selecția multiplă a investigațiilor este păstrată exclusiv în
      `patientAppointmentInvestigations`; coloana singulară este citită numai la migrarea
      datelor istorice și rămâne `NULL` pentru salvările noi.
- [x] Migrarea completează ID-urile istorice numai pentru potriviri text unice.
- [x] Pacientul poate fi selectat prin model/completer sau introdus ca text liber.
- [x] Salvarea și actualizarea păstrează atât textul istoric, cât și ID-urile selectate.
- [x] La salvare, denumirile investigațiilor sunt reconstruite din catalog după ID;
      textul afișat de delegate nu este folosit ca sursă pentru relații.
- [x] Crearea unei comenzi din programare completează toate investigațiile disponibile
      în lista de prețuri și raportează separat investigațiile indisponibile.
- [x] Butonul de eliminare oferă ștergerea rândului sau a întregii zile.
- [x] Compilare Debug reușită cu Qt 6.9.3/qmake.
- [x] SQL-ul de completare a relațiilor a fost verificat pe SQLite sintetic.
- [x] Migrarea și operațiile CRUD au fost confirmate prin aplicație pe baza de test.
- [x] Compatibilitatea SQL MariaDB a fost corectată pentru tabela copil și cheile
      externe; testarea funcțională generală MariaDB a fost confirmată.

### 2026-09-10 — Finalizarea jurnalelor și fluxurilor documentelor

- Etapa: înlocuirea `ListDocReportOrder` cu `OrderView` și `ReportView`.
- Componente afectate: jurnalele și loaderele de comenzi/rapoarte, `MainWindow`,
  filtrele, coloanele, perioada, preview-ul și acțiunile documentelor.
- Curățare: eliminate `ListDocReportOrder`, `DocOrderEcho` și `DocReportEcho`;
  `OrderView` rămâne jurnalul activ pentru comenzi.
- Teste efectuate: toate sistemele de raport, revalidare și printare pe SQLite și
  MariaDB, confirmate de utilizator.
- Compilare: Debug reușită cu Qt 6.9.3/qmake.

### 2026-09-10 — Sincronizare cloud, imagini și e-mail

- Etapa: sincronizarea prin UUID a pacientului, comenzii, raportului și imaginilor.
- Migrare: UUID-urile locale sunt transferate în MariaDB; `db_image` și obiectele
  cloud lipsă sunt compatibilizate la actualizare.
- E-mail: export PDF/imagine, deschiderea atașamentelor, semnătură și ștampilă;
  coruperea memoriei la închidere a fost eliminată.
- Teste efectuate: creare și modificare pacient/comandă/raport cu sincronizare
  reușită; export și deschidere atașamente confirmate.

### 2026-09-11 — Modele, AppSettings și logging

- Etapa: eliminarea infrastructurii vechi neutilizate și reducerea efectelor
  secundare din setări.
- Modele: păstrat `BaseAbstractModel`; adăugate `OrderInvestigationModel`,
  `OnlineAccountModel` și `OrganizationContractModel`; `CatalogTableEditor` rămâne
  consumatorul generic direct.
- AppSettings: ownership-ul `QSettings` este gestionat, starea este restaurată la
  anulare, cheile sunt centralizate, iar memorarea utilizatorului și preferințele
  mesajelor folosesc operații tipizate fără construirea dialogului.
- Schema INI este documentată în `docs/appsettings_schema.md`; valorile implicite
  sunt centralizate, iar indexurile și retenția logurilor sunt validate la citire.
- Salvarea verifică explicit rezultatul `QSettings::sync()`; la eroare nu închide
  dialogul, nu pornește restartul și nu raportează fals succesul.
- `loadSettings()` nu mai creează/deschide o conexiune SQL și nu mai citește
  preferințele din baza de date; aceste responsabilități aparțin fluxului principal.
- Preferințele utilizatorului sunt încărcate explicit de `AppController` după
  autentificare, înainte de construirea `MainWindow`; opțiunea de confirmare la
  închiderea aplicației și celelalte preferințe sunt astfel disponibile la timp.
- Inițializarea limbii nu mai are ramuri duplicate și nu mai poate lăsa comboboxul
  în dezacord cu `globals().langApp`.
- Codificarea legacy a valorilor `.conf` este separată de instanța `DataBase`;
  memorarea utilizatorului nu mai construiește inutil un obiect de acces SQL.
- Logging: ieșirea Debug este vizibilă și în Application Output, scrierea este
  thread-safe, iar rotirea, retenția și schimbarea căii sunt corectate.
- Responsabilitatea pentru rotația și retenția fișierelor a fost mutată din
  `AppSettings` în `LogManager`; dialogul doar configurează și afișează logurile.
- `LogManager` închide fișierul activ înainte de rotație, folosește data primei
  înregistrări pentru numele arhivei și păstrează convenția
  `profil_dd.MM.yyyy[_n].log`.
- Handlerul global de logare este restaurat explicit, iar fișierul este închis la
  orice ieșire din `AppController`, inclusiv în fluxurile întrerupte înainte de
  afișarea ferestrei principale.
- Dacă aplicația rămâne deschisă peste miezul nopții, primul mesaj din ziua nouă
  rotește logul activ și continuă scrierea într-un fișier activ nou.
- Inițializarea `LogManager` raportează explicit succesul/eșecul; dacă directorul
  sau fișierul nu poate fi deschis, mesajele continuă în ieșirea standard Qt.
- Rotirea și retenția logurilor recunosc integral numele configurațiilor care conțin
  puncte (de exemplu `test_v4.1.0.conf`), folosind `completeBaseName()`.
- Citirea setărilor verifică înainte de aplicare existența, permisiunea de citire și
  formatul fișierului `.conf`; la eroare, pornirea este oprită cu un mesaj explicit,
  fără folosirea unei configurații globale parțiale.
- Starea de revenire pentru „Anulare” este capturată după încărcarea profilului valid;
  dacă salvarea eșuează, variabilele globale revin la ultima configurație validă.
- Validarea elimină spațiile marginale din identificatorii și căile configurației
  (fără a modifica parola), iar relansarea după schimbarea limbii nu poate salva
  o configurație care nu a trecut `checkDataSettings()`.
- Relansarea după schimbarea limbii transmite corect argumentele și închide procesul
  curent numai după confirmarea pornirii noului proces.
- Valorile legacy Base64/XOR sunt validate înainte de aplicarea profilului, fără
  expunerea datelor în log; portul MariaDB este validat și la citirea `.conf`.
- Opțiunile booleene din `.conf` sunt citite strict (`true/false`, `1/0`); valorile
  incompatibile nu mai sunt convertite silențios și folosesc implicitul documentat.
- Editarea câmpurilor din `AppSettings` nu mai modifică imediat conexiunea și căile
  globale; `globals()` este actualizat din formular numai după `QSettings::sync()`
  reușit. Popularea formularului nu îl mai marchează fals ca modificat.
- `readSettings()` blochează temporar semnalele controalelor cu `QSignalBlocker`;
  secvențele fragile de deconectare/reconectare și riscul conexiunilor duplicate au
  fost eliminate.
- Rotirea detectează și un log activ mixt pe baza primei înregistrări, nu doar după
  `lastModified()`; reconstruirea comboboxului la traducere nu mai marchează fals
  dialogul ca modificat.
- „Memorează utilizatorul” verifică perechea ID/nume; datele incoerente dezactivează
  precompletarea, iar debifarea elimină valorile codificate rămase în `.conf`.
- Înainte de salvare sunt validate locațiile `.conf`, log, SQLite și `db_image`:
  accesul fișierelor existente și posibilitatea creării într-un director părinte.
- Obiectele auxiliare deținute de `AppSettings` folosesc ownership explicit;
  ștergerile manuale și temporizatorul fără proprietar au fost eliminate.
- Structura duplicată `SettingsState` a fost eliminată; încărcarea, formularul,
  salvarea și restaurarea la „Anulare” folosesc același `ProfileData` tipizat.
  Restaurarea acoperă întregul profil și păstrează exact calea logului și calea
  fișierului de configurare active înaintea deschiderii dialogului.
- Construirea dialogului `AppSettings` nu mai suprascrie prematur limba sau calea
  globală a logului. Calea implicită este calculată local și devine globală numai
  după încărcarea ori salvarea reușită a profilului; un profil fără nume păstrează
  implicitul `usg.log`, nu generează fișierul ascuns `.log`.
- Profilul păstrează simultan setările SQLite și MariaDB. Schimbarea motorului activ
  nu mai golește câmpurile celuilalt motor, iar `AppSettingsStore` citește și scrie
  ambele configurații; testul round-trip verifică explicit această proprietate.
- Crearea bazei SQLite din `AppSettings` transmite explicit numele și calea către
  `DataBase`; nu mai înlocuiește temporar `globals().sqliteNameBase` și
  `globals().sqlitePathBase`. Inițializarea schemei acceptă acum o conexiune SQL
  explicită, izolată și eliminată corect după operație.
- Regulile reutilizabile pentru validarea fișierelor, directoarelor, numelor de
  profil și a configurațiilor complete SQLite/MariaDB sunt separate în
  `AppSettingsValidator` și acoperite de teste automate.
- `checkDataSettings()` folosește exclusiv rezultatul tipizat al validatorului;
  ramurile duplicate SQLite/MariaDB au fost eliminate, iar dialogul doar afișează
  motivul și mută focusul pe controlul invalid.
- Sloturile intermediare vechi pentru modificarea numelui/căii SQLite și a
  parametrilor MariaDB au fost eliminate. Toate câmpurile conexiunilor marchează
  direct și uniform dialogul ca modificat prin `textChanged`; blocarea semnalelor
  la popularea formularului previne în continuare marcarea falsă drept modificat.
- Calea implicită a șabloanelor este `<director executabil>/templets`, iar cea a
  rapoartelor este `<director executabil>/templets/reports`; pornirea aplicației
  din alt director de lucru nu mai schimbă localizarea implicită.
- Ramura macOS nu mai copiază șabloanele în `~/USG/templets`; inițializarea
  SQLite creează numai directorul bazelor, iar șabloanele sunt utilizate uniform
  din directorul în care este instalată aplicația.
- Directorul implicit al bazelor SQLite este verificat ca director și creat
  recursiv cu `QDir::mkpath`; prima lansare pe macOS nu mai eșuează atunci când
  directorul intermediar `~/USG` încă nu există.
- Fluxul de creare a unei baze noi a fost serializat: aplicația așteaptă
  finalizarea inițializării în worker înainte să deschidă conexiunea UI, să creeze
  administratorul și să construiască `MainWindow`.
- `DatabaseInit` și operațiile de creare a tabelelor returnează succes/eșec;
  lansarea este oprită cu mesaj explicit dacă schema principală sau `db_image`
  nu a fost creată complet. Conexiunile SQL ale workerului sunt eliminate înainte
  de deschiderea conexiunii principale în thread-ul UI.
- Au fost corectate blocajele certe ale schemei MariaDB noi: sintaxa tabelei
  `users`, declarațiile `tablePancreas`/`tableIntestinalLoop` și acțiunile
  `ON DELETE` incompatibile cu FK-urile obligatorii din comenzi și prețuri.
- Încărcarea normogramelor este validată integral: resursa XML și parsarea,
  starea tabelei, fiecare inserare și commit-ul tranzacției propagă eroarea către
  fluxul de lansare; o inițializare parțială nu mai este raportată ca succes.
- Crearea MariaDB include acum tabela `imagesReports`, al cărei script exista dar
  nu era apelat. Înainte de administrator sunt verificate toate tabelele medicale,
  tabelele auxiliare și view-urile obligatorii, inclusiv `db_image.imagesReports`
  pentru configurația SQLite.
- Crearea fișierului SQLite din `AppSettings` nu mai creează și schema; controlerul
  aplicației este singurul responsabil de inițializarea și verificarea schemei la
  prima lansare. Mesajul rezultat diferențiază acum succesul de eroare.
- Scriptul MariaDB pentru `contracts` poate fi reluat după o inițializare parțială:
  indexul și cheia externă spre contractul implicit sunt adăugate idempotent, iar
  indexul UUID are denumirea corectă `uq_contracts_uuid`.
- Executorul resurselor SQL recunoaște acum blocurile `BEGIN ... END`; trigger-ele
  SQLite și MariaDB nu mai sunt rupte la separatorii interni `;`. Trigger-ele
  existente sunt înlocuite controlat, astfel încât inițializarea poate fi reluată.
- Ordinea schemei MariaDB respectă FK-ul investigațiilor: `investigationsGroup`
  este creată înainte de `investigations`; virgula finală invalidă din scriptul
  grupurilor a fost eliminată.
- Căile SQL vechi din migrările 2.x–3.0.6 au fost mutate la structura actuală a
  resurselor. Pentru MariaDB, `onlineAccount` și `cloudServer` se creează în 4.0.1
  folosind tipurile reale ale ID-urilor bazei 3.x, evitând incompatibilitatea
  `INT`/`BIGINT UNSIGNED` a cheilor externe.
- Încărcarea clasificatorului Investigații nu mai conține virgula SQL invalidă,
  generează UUID doar când schema îl are și păstrează `owner=NULL` dacă grupul
  referit nu există. Eșecul este propagat către migrare și FirstRunWizard.
- Schema șabloanelor definește indexuri UUID unice pentru SQLite și MariaDB;
  `reportVideo` are aceeași politică `ON DELETE CASCADE` în ambele motoare și
  indexul MariaDB poartă denumirea corectă.
- Adăugarea șabloanelor de concluzii și descrieri din toate paginile medicale
  generează și salvează UUID-ul obligatoriu; schema nouă MariaDB/SQLite nu mai
  diferă de comportamentul formularelor la inserare.
- Verificarea unei baze noi controlează acum coloanele critice, executarea
  view-urilor, toate cele 10 triggere obligatorii, `cryptoSplitKey` și coloanele
  `patient_id`/`uuid` din `imagesReports`.
- Migrarea 3.0.1 nu mai reconstruiește inutil tabela SQLite `pacients`; vechiul
  flux putea șterge tabela fără să o recreeze. Actualizarea investigațiilor este
  tranzacțională, actualizează și grupul (`owner`), folosește numărul real de
  elemente XML pentru progres și revine complet la eroare.
- Adăugările de coloane din 3.0.3 și 3.0.6 sunt idempotente și propagă eroarea;
  reconstruirea SQLite a `formationsSystemTemplates` este tranzacțională și
  păstrează UUID-urile valide existente.
- Toate cele 57 de tabele MariaDB sunt create explicit cu `ENGINE=InnoDB`.
  Tipul `cryptoSplitKey.id_organizations` corespunde cheii `organizations.id`,
  iar sintaxa idempotentă a FK-ului organizație–contract a fost corectată.
- Au fost adăugate cheile primare lipsă pentru `pricingsPresentation.id` și
  `tableBladder.id`; scripturile legacy pentru `fullNamePacients` au fost
  eliminate, deoarece schema 4.1.0 folosește direct `patients`.
- Ordinea MariaDB creează view-ul organizațiilor numai după `contracts`.
  Inițializarea exactă prin `DataBaseCommon` a fost testată de două ori pe o
  instanță MariaDB temporară: 57 tabele, 7 view-uri, 10 triggere, 71 FK-uri,
  zero tabele non-InnoDB și zero coloane AUTO_INCREMENT fără cheie.
- Inițializarea exactă SQLite a fost testată de două ori pe baze temporare:
  schema principală are 56 tabele, 7 view-uri, 10 triggere și zero erori FK;
  `db_image` se poate crea și relua idempotent. `cryptoSplitKey` și
  `imagesReports` folosesc acum `CREATE TABLE IF NOT EXISTS`.
- Crearea administratorului din fluxul inițial folosește proprietatea tipizată
  `setIsNew(true)`; proprietatea dinamică greșită `ItNew` nu mai trimite dialogul
  pe ramura de actualizare cu ID zero.
- Autorizarea se finalizează numai după terminarea workerului care încarcă
  constantele, organizația, doctorul și configurația cloud. La eroare, dialogul
  rămâne deschis și permite reîncercarea; `MainWindow` nu mai pornește în paralel
  cu inițializarea datelor globale.
- Profilul păstrează explicit `initialSetupComplete`. Crearea incompletă a
  schemei, anularea administratorului sau întreruperea `FirstRunWizard` sunt
  reluate idempotent la următoarea lansare. Profilurile istorice fără această
  cheie sunt considerate deja configurate.
- `FirstRunWizard` marchează configurarea drept finalizată numai când utilizatorul
  ajunge la ultimul pas și închide asistentul; închiderea anticipată cere
  confirmare și păstrează starea de reluare.
- Jurnalul este inițializat după citirea/salvarea profilului, când calea sa este
  cunoscută. Crearea SQLite raportează fiecare tabel, view și trigger procesat,
  iar preferințele administratorului nou primesc direct versiunea curentă pentru
  a nu declanșa eronat migrarea unei baze tocmai create.
- Compilare: Debug reușită cu Qt 6.9.3/qmake.
- Teste rămase: flux UI complet pe profil și baze temporare — creare administrator,
  anulare/reluare după administrator, întrerupere/reluare `FirstRunWizard`,
  relansarea cu „Memorează”, profil vechi și profil curat.

## Pregătirea release-ului 4.1.0

- [x] Alinierea versiunii aplicației, Qt IFW și pachetului Debian la 4.1.0.
- [x] Eliminarea antetului redundant `src/data/version.h` rămas la 4.0.1.
- [x] Corectarea câmpului standard `Version=1.0` și a metadatelor în fișierele
  Desktop Entry.
- [x] Actualizarea metainfo și a changelogurilor Debian pentru 4.1.0.
- [x] Documentarea LimeReport 1.7.23 prin tag, commit fix și patch local separat.
- [x] Adăugarea workflow-ului Linux Qt 6.9.3/qmake pentru un build reproductibil.
- [x] Extinderea workflow-ului Linux: rularea manuală păstrează pachetele ca
  artefact temporar, iar tagurile `v*` creează automat un GitHub Release în
  stare draft și încarcă `.run`, `.deb`, AppImage, sursa LimeReport și sumele
  SHA-256. Publicarea draftului rămâne o confirmare manuală.
- [x] Extinderea auditului de licențiere la `resources/img`, `resources/icons` și
  `resources/images`.
- [x] Includerea în pachetele Linux a licenței USG, auditului activelor, licențelor
  LimeReport, documentației de proveniență și patch-ului local.
- [x] Generarea alături de pachete a arhivei sursei LimeReport și a sumei SHA-256.
- [x] Revizuirea `README.md` și `README-RO.md` pentru v4.1.0 și includerea lor,
  împreună cu istoricul versiunilor, în pachetele Linux.
- [x] Documentarea cerinței FUSE 2 pentru AppImage pe Debian 13
  (`libfuse2t64`) și a pornirii alternative `--appimage-extract-and-run`.
- [x] Crearea `release.md` cu descrierea pregătită pentru GitHub Release.
- [x] Revizuirea site-ului public (`index.html`, `privacy.html`, `robots.txt` și
  `sitemap.xml`) și alinierea politicii de confidențialitate cu funcțiile
  opționale configurate de utilizator.
- [x] Executarea buildului Release al candidatului final cu Qt 6.9.3/qmake și
  verificarea dependențelor dinamice ale executabilului rezultat.
- [x] Executarea scriptului de împachetare și verificarea conținutului `.run`,
  `.deb` și `.AppImage`.
- [x] Instalarea și dezinstalarea curată a pachetului `.run` într-un profil Linux
  izolat; shortcutul, executabilul și dependențele au fost verificate, iar
  dezinstalarea nu lasă shortcutul sau fișierele componentei.
- [x] Verificarea structurală și pornirea controlată, cu profil izolat, a
  candidatului `.AppImage`; nu au fost detectate biblioteci lipsă sau erori fatale.
- [x] Verificarea structurală a pachetului `.deb`, a permisiunilor și a scripturilor
  de mentenanță; configurațiile, bazele de date și logurile utilizatorului sunt
  păstrate la actualizare și dezinstalare.
- [x] Recompilarea completă Release după curățarea codului neutilizat și
  regenerarea tuturor artefactelor; dependențele executabilului și digesturile
  celor patru artefacte au fost reverificate cu succes.
- [x] Auditarea setului pregătit pentru commit: bazele de test, configurațiile,
  logurile, cheile, arhivele locale și pachetele din `build/` nu sunt incluse;
  indexul conține toate modificările proiectului, fără fișiere nestaged sau
  neinventariate. Workflow-ul de release are sintaxă YAML validă, dar rularea
  efectivă pe GitHub rămâne de făcut după commit și push.
- [x] Rularea testelor unitare `AppSettingsStore`/validator/codec într-un build
  separat cu Qt 6.9.3: 10 teste trecute, 0 eșuate.
- [x] Build Release complet într-un director temporar gol și pornire controlată
  cu profil XDG izolat; aplicația a rămas activă până la oprirea prin timeout,
  fără crash sau biblioteci lipsă.
- [x] Regenerarea candidatului final și reverificarea celor patru digesturi
  SHA-256 după ultima schimbare staged.
- [x] Testarea izolată a pachetului `.run`: instalare headless, pornire,
  dezinstalare completă și eliminarea shortcutului — reușite.
- [x] Testarea izolată a pachetului `.deb`: metadate, Desktop Entry, fișiere
  obligatorii, dependențe și pornire din rădăcina extrasă — reușite.
- [x] Testarea AppImage prin `--appimage-extract-and-run`, fără FUSE, cu profil
  izolat — pornire reușită și fără crash până la timeout.
- [ ] Validarea finală de către utilizator pe copii ale bazelor reale SQLite și
  MariaDB; commitul se face numai după confirmarea explicită a acestei matrice.
- [ ] Instalarea fiecărui artefact într-un mediu curat și executarea matricei din
  `docs/release_4_1_0_plan.md`.
- [ ] Publicarea artefactelor și a tagului `v4.1.0`; `version.txt` nu trebuie
  împins pe canalul public înainte ca descărcările 4.1.0 să existe.
