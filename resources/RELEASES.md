## USG v4.2.8 (04.10.2026)

- În fereastra de selectare a bazei de date, profilul `.conf` selectat poate fi deschis pentru editare: cu aplicația implicită pe Linux și cu Notepad pe Windows; lipsa fișierului sau imposibilitatea deschiderii sunt raportate utilizatorului.
- Butoanele din selectarea bazei de date au iconurile și textele aliniate uniform; a fost adăugată și documentată iconița pentru editarea profilului.
- Corectate căile resurselor Qt în ferestrele de lansare inițială, configurarea serverului cloud, contul online și agentul de e-mail.
- Adăugate exemple în câmpurile de configurare MariaDB, server cloud și cont e-mail, fără modificarea valorilor salvate.
- Actualizate imaginile de pornire și inventarul/licențierea iconurilor distribuite cu aplicația.
- Traducerea rusă a fost actualizată și regenerată; toate textele curente detectate de `lupdate` sunt traduse.
- Fișierele manualului și comenzile pentru instrumentele de traducere sunt documentate în proiectul qmake.
- Versiunea 4.2.8 nu introduce o migrare nouă a schemei bazei de date.

## USG v4.2.7 (03.10.2026)

- Parolele utilizatorilor se păstrează ca hash PBKDF2-SHA256 cu salt; la actualizarea bazei de date hash-urile existente sunt convertite automat, fără schimbarea parolelor.
- Coloana veche cu parola codificată reversibil (bazele create înainte de 4.1.0) este golită.
- Parola serverului cloud este criptată cu cheia organizației (parte în baza de date, parte în fișierul local din profil), ca la conturile de e-mail; schimbarea parolei utilizatorului nu mai afectează configurația cloud. După restaurarea bazei pe alt calculator, parola cloud trebuie reintrodusă.
- Numele utilizatorilor sunt unice și nu țin cont de majuscule („Admin” = „admin”), la SQLite și MariaDB; dacă baza conține nume duplicate, actualizarea le redenumește („nume (2)”) și afișează lista în panoul informativ.
- Verificarea numelui duplicat la crearea sau redenumirea unui utilizator a fost corectată.
- Parolele noi sau schimbate trebuie să aibă cel puțin 8 caractere; la crearea administratorului inițial fără parolă se cere confirmare.
- Autentificare: după 3 încercări nereușite pentru același nume urmează o pauză (30 s, apoi 60, 120, 240, maximum 300 s), butonul afișează timpul rămas, iar mesajul de eroare nu mai indică dacă greșit este numele sau parola. Hash-urile vechi sunt rescrise în formatul nou la prima autentificare reușită.
- Dacă baza a fost mutată fără directorul `crypto/` al profilului, aplicația semnalează că parola cloud și parolele conturilor de e-mail trebuie reintroduse.
- Arhivarea 7z: progresul este afișat corect, arhivarea la închidere rulează într-un dialog cu progres, iar directoarele cu setări și chei de criptare pot fi incluse opțional. Arhiva poate fi criptată AES-256 cu parolă (salvată criptat pentru arhivarea automată) și este verificată după creare.
- Catalogul Pacienți: căutare după nume, prenume, IDNP sau data nașterii (Ctrl+F); încărcarea pe pagini păstrează rândul curent; pacientul fără documente sau programări poate fi eliminat din baza de date (meniul butonului de eliminare, meniul contextual sau Shift+Delete), altfel aplicația afișează documentele care îl folosesc. Corectată marcarea pacienților pentru eliminare.
- Căutarea pacientului în Comanda ecografică, Raportul ecografic, istoricul pacientului și programări găsește și textul „Familie Prenume”.
- Preferințele utilizatorului: butoanele OK, Salvează, Închide în ordinea obișnuită; confirmarea modificărilor nesalvate este tradusă.
- Jurnalul Comenzilor ecografice: câmp de căutare (butonul din bara de instrumente sau Ctrl+F) după pacient (nume, prenume, IDNP) ori după investigație (cod sau denumire); criteriul se alege din meniul câmpului, iar la căutarea după investigație apare lista investigațiilor potrivite.
- Programarea pacienților: forma de tipar a programărilor zilei (A4 orizontal, cu antetul organizației și numele medicului), din butonul **Printează**.
- Programarea pacienților: corectate avertismentele la editarea celulelor (pacient, investigații, organizație, doctor, „Efectuat”) și afișarea rândurilor efectuate.
- Pe o bază MariaDB comună, toate stațiile trebuie actualizate la 4.2.7: versiunile anterioare nu mai pot verifica parolele convertite.

## USG v4.2.6 (02.10.2026)

- E-mail cu mai multe rapoarte: din jurnalul Rapoartelor ecografice (butonul de e-mail din bara de instrumente sau meniul contextual) se deschide o fereastră de selecție a rapoartelor validate, filtrate după organizația care a trimis pacienții și perioadă; rândurile selectate în jurnal (Ctrl/Shift) sunt bifate implicit.
- Rapoartele alese se exportă în PDF și se atașează într-o singură scrisoare adresată organizației trimițătoare (adresa din catalogul organizațiilor); imaginile atașate rapoartelor se pot include opțional.
- Înainte de export fiecare raport este verificat din nou (validat și aparținând organizației alese); la export parțial aplicația afișează erorile și întreabă dacă se continuă, iar la atașamente de peste 20 MB cere confirmare.
- Cât timp se pregătesc rapoartele, jurnalul nu poate fi închis și nu se poate porni un al doilea export; directorul temporar se șterge după trimitere sau la anulare.
- Exportul imaginilor unui raport folosește un serviciu comun pentru trimiterea unui singur document și a mai multor rapoarte.
- Ecran de pornire sezonier nou pentru toamnă (1 septembrie – 30 noiembrie) și primăvară (1 martie – 31 mai).
- Traducerea rusă a fost actualizată pentru funcțiile noi.

## USG v4.2.5 (01.10.2026)

- Pornire: la lansarea obișnuită aplicația verifică existența fișierelor SQLite din profil (baza principală și baza imaginilor) și se oprește cu un mesaj, fără să creeze baze goale; o bază MariaDB goală sau fără schema aplicației este semnalată explicit, cu recomandarea de a alege prima lansare.
- Prima lansare: o bază existentă aleasă la configurare nu mai este recreată, iar versiunea schemei se scrie numai după verificarea completă; numele profilului și baza imaginilor sunt propuse după fișierul ales.
- Inițializarea schemei și încărcarea constantelor după autentificare rulează pe conexiuni proprii; închiderea ferestrei de autentificare în timpul încărcării nu mai provoacă erori.
- Administratorul inițial se creează numai când tabela utilizatorilor este goală; dacă toți utilizatorii sunt marcați ca eliminați, aplicația afișează un mesaj și se oprește.
- Asistentul primei lansări: parola este obligatorie pentru utilizatorii adăugați, tipurile de prețuri și investigațiile nu se mai dublează la apăsări repetate.
- Jurnalul Comenzilor ecografice: ștergerea cere confirmare, elimină și imaginile, video-urile și, la alegere, copia din cloud; filtrele după număr, organizație și contract au fost corectate; sortarea după orice coloană se aplică tuturor documentelor; previzualizarea urmează rândul curent; erorile de încărcare sunt afișate.
- Tipărirea din jurnale folosește aceleași servicii ca documentele (ștampila și semnătura ascunse implicit); imaginea din formularul Comenzii ecografice a fost redimensionată.
- E-mail: detectarea corectă a formatului imaginilor atașate, ștergerea sigură a directorului temporar, respingerea adreselor invalide și blocarea pornirii unui al doilea export în paralel.
- Prețuri: corectată eliminarea documentului la eșecul salvării și eroarea la alegerea organizației (contractele).
- Rapoarte statistice: lista conține numai șabloanele `.lrxml` disponibile, cu rând de selecție explicit.
- Ecranul de pornire în limba rusă afișează textele aliniate corect; traducerea rusă a fost completată.

## USG v4.2.4 (30.09.2026)

- Corectată separarea dintre organizația trimițătoare păstrată în Comanda ecografică și identitatea cabinetului care efectuează investigația: antetul, logotipul și imaginile de tipărire provin din preferințele utilizatorului.
- Comanda ecografică folosește logotipul și ștampila organizației executante configurate în preferințe.
- Raportul ecografic folosește datele organizației executante, iar ștampila și semnătura aparțin doctorului implicit configurat în preferințe și respectă parametrii de tipărire.
- Exportul PDF pentru e-mail folosește aceeași identitate de tipărire ca previzualizarea documentelor.
- Corectată maparea imaginilor în rapoartele statistice: logotipul organizației, ștampila doctorului și semnătura doctorului; ștampila organizației nu mai înlocuiește semnătura.
- Manualul utilizatorului rămâne disponibil din meniul superior de asistență; butonul redundant a fost eliminat din bara de instrumente.

## USG v4.2.3 (29.09.2026)

- Manualul de utilizare în limba română a fost rescris, completat cu capturi de ecran și legat de butonul **Manual Online** din fereastra principală; pagina Wiki veche este înlocuită.
- Adăugată fereastra **Servere cloud** pentru administrarea configurațiilor de sincronizare și corectată închiderea acesteia.
- Sincronizarea SQLite–MariaDB poate fi activată sau dezactivată din preferințe; migrarea 4.2.3 adaugă și verifică opțiunea în SQLite și MariaDB.
- Introduse contexte dedicate pentru Comanda și Raportul ecografic și serviciul comun pentru imaginile de tipărire; logo-ul, ștampilele și semnătura sunt citite conform organizației și doctorului documentului.
- Revizuite exportul PDF, anexele, selectarea contului și fluxul agentului de e-mail.
- Completate acțiunile pentru istoricul pacientului și jurnalele Comenzilor/Rapoartelor ecografice.
- Revizuite pachetele Linux: RPATH portabil pentru LimeReport și instalare fără intrări desktop duplicate.
- Actualizată traducerea interfeței în limba rusă.

## USG v4.2.2 (27.09.2026)

- Migrarea UUID verifică identitatea SQLite–MariaDB înainte de scriere.
- Pacienții sunt identificați prin IDNP compatibil cu NPP și data nașterii sau, în lipsa IDNP, prin NPP și data nașterii; ID-ul numeric nu este folosit ca dovadă de identitate, iar documentele copil folosesc legăturile părinte verificate.
- Tipurile de preț sunt identificate prin rolul semantic comercial/CNAM, nu prin denumirea tradusă sau reducerea modificabilă; astfel pot fi asociate în siguranță contractele și documentele de preț dependente.
- Pentru o pereche SQLite–MariaDB se actualizează mai întâi profilul SQLite cu sincronizarea activă; profilul MariaDB se deschide direct numai după confirmarea commitului UUID comun.
- Cazurile ambigue și duplicatele istorice rămân distincte și sunt raportate; conflictele UUID și erorile de schemă opresc transferul înainte de commit.
- Added read-only migration tests for legacy schemas and different numeric IDs.

## USG v4.2.1 (27.09.2026)

- Directorul de logare Windows este creat înaintea validării primei configurări.
- Administratorul inițial poate fi creat cu parola goală.
- Corectate icoanele din asistentul inițial și din tabele.
- Grupurile investigațiilor sunt inițializate înaintea importului dacă tabela este goală.
- Corectată crearea organizației și compatibilitatea Qt 6/MSVC.
- Adăugate fluxurile pentru installerul Windows, sursele LimeReport și sumele SHA-256.

## USG v4.2.0 (26.09.2026)

- Setările aplicației, organizației și utilizatorului au fost separate în tabele dedicate, cu migrarea și verificarea datelor păstrate anterior în `constants` și `userPreferences`.
- Reorganizate setările aplicației și starea utilizată în timpul execuției prin servicii și contexte dedicate pentru căi, conexiunea principală, conexiunea cloud, sesiune, organizație și doctor.
- Reorganizate directoarele sursă și fișierele qmake pe componente și funcționalități; au fost revizuite dependențele și scriptul de compilare pentru Windows.

- Textul consimțământului informat al Comenzii ecografice este generat în funcție de investigațiile selectate, separat pe patru categorii: invazive, neinvazive, endocavitare și screening obstetrical.
- În Raportul ecografic, butonul „Opțiuni” permite adăugarea investigațiilor în timpul examinării și configurarea prezentării ștampilei și semnăturii doctorului la previzualizare și tipărire.
- A fost păstrat comportamentul existent al exportului PDF pentru ștampila și semnătura doctorului.

- Corectată salvarea parolelor conturilor de e-mail în bazele actualizate: tabela lipsă `cryptoSplitKey` este creată de migrarea 4.2.0 și reparată la lansare pentru bazele deja marcate 4.2.0.
- Butonul „Verificarea conectării” din contul online verifică conexiunea și autentificarea SMTP fără a trimite mesaj; rezultatul este afișat printr-o notificare.
- La eșecul salvării contului online este afișat motivul, nu doar scris în jurnal.
- Câmpurile obligatorii necompletate sunt indicate uniform prin baloane de informare în prețuri, cataloage, configurarea cloud, trimiterea e-mail, autorizare și rapoartele statistice.

## USG v4.1.2 (18.09.2026)

- Redenumite variabilele globale și cheile de căi din `.conf`; profilurile vechi sunt migrate automat cu backup `.pre-4.1.2.bak`, păstrând valorile și cheile necunoscute.
- Corectată afișarea marcajelor de listă în istoricul actualizării.

- Adăugat site-ul organizației și migrarea reluabilă a coloanei `organizations.site` pentru SQLite și MariaDB; valorile existente sunt păstrate.
- Site-ul organizației este inclus în șablonul tipărit al Comenzii ecografice.
- Citirea datelor organizației la autorizare rămâne compatibilă cu bazele anterioare migrării.

- Revizuită salvarea preferințelor utilizatorului: modificările sunt aplicate tranzacțional, selecția utilizatorului este păstrată, iar logo-ul și opțiunile locale nu mai sunt salvate înainte de confirmare.
- Adăugată crearea și modificarea medicului trimițător direct din Comanda ecografică, cu actualizarea listei și selectarea automată a medicului nou.

- Etapele actualizării sunt afișate și în panoul informativ, cu numărul real de UUID-uri completate pe tabelă, erorile și rezultatul final.

- Titlul ferestrei principale păstrează versiunea de la care pornește actualizarea și afișează versiunea nouă numai după finalizarea migrării și salvarea versiunii. La eșec sau amânare, versiunea anterioară rămâne afișată, inclusiv după schimbarea limbii.
- Corectată verificarea versiunilor terminate în zero la solicitarea credențialelor cloud înainte de migrare.

## USG v4.1.1

- Corectată validarea versiunilor bazei de date terminate în zero (de exemplu `4.1.0`), care blocau actualizarea pe SQLite și MariaDB.

- Corectat șablonul `Order.lrxml`: geometrie A4 landscape, separator central la 148,5 mm și printare la dimensiunea paginii, fără scalare la zona imprimabilă.
- Actualizat identificatorul A4 din `Order.lrxml` pentru Qt 6 (`pageSize=7`); vechea valoare `0` era interpretată ca Letter și suprascria geometria la încărcare.
- Corectată printarea listei prețurilor după migrarea `investigations.owner` la ID numeric, pentru SQLite și MariaDB.
- Lipsa grupelor pentru printare este afișată separat de erorile SQL.

## USG v4.1.0
* <b>Actualizare tehnică:</b>
    * generatorul de rapoarte [LimeReport](https://github.com/fralx/LimeReport) a fost actualizat de la versiunea 1.7.14 la versiunea 1.7.23;
    * integrarea a fost recompilată pentru Qt v6.9.3.
    * sursa LimeReport și corecția locală sunt fixate și documentate pentru build-uri reproductibile.
* <b>Migrarea pacienților:</b>
    * tabela <code>pacients</code> a fost redenumită în <code>patients</code>;
    * coloanele pacienților și referințele din documente au fost uniformizate;
    * migrarea include verificări de consistență și protecții pentru relansarea din stări deja actualizate.
* <b>Compatibilitate SQLite și MariaDB:</b>
    * schemele, view-urile, indexurile și cheile externe necesare aplicației au fost revizuite;
    * UUID-urile create în SQLite sunt transferate și păstrate identic în MariaDB;
    * au fost corectate interogările dependente de motorul SQL și încărcarea datelor istorice.
* <b>Comenzi și rapoarte ecografice:</b>
    * jurnalele vechi au fost înlocuite cu ferestrele dedicate <code>OrderView</code> și <code>ReportView</code>;
    * au fost îmbunătățite filtrarea, păstrarea perioadei, previzualizarea documentelor și indicarea imaginilor atașate;
    * salvarea și validarea sunt separate: validarea poate solicita tipărirea documentului;
    * au fost corectate redeschiderea, revalidarea și tipărirea secțiunilor raportului ecografic.
* <b>Programarea pacienților:</b>
    * documentul a fost reorganizat în <code>AppointmentDialog</code>;
    * poate fi selectat un pacient existent sau introdus un nume liber;
    * o programare poate conține mai multe investigații, păstrate într-o tabelă copil;
    * investigațiile selectate sunt transferate automat la crearea Comenzii ecografice.
* <b>Sincronizare cloud:</b>
    * sincronizarea pacientului, comenzii, raportului și imaginilor folosește UUID-uri;
    * au fost corectate maparea relațiilor, evitarea duplicatelor și compatibilizarea bazelor istorice;
    * erorile de sincronizare sunt raportate fără pierderea datelor salvate local.
* <b>Tipărire și e-mail:</b>
    * exportul PDF include corect semnătura și ștampila doctorului;
    * atașamentele PDF și imaginile pot fi deschise prin aplicația implicită a sistemului de operare;
    * au fost eliminate crashurile și coruperea memoriei din fluxul de pregătire a e-mailului.
* <b>Setările aplicației:</b>
    * citirea, validarea și salvarea profilurilor au fost consolidate;
    * setările SQLite și MariaDB sunt păstrate separat în același profil;
    * parolele cloud sunt protejate, iar configurațiile vechi sau invalide sunt tratate explicit;
    * șabloanele sunt încărcate din directorul în care este instalată aplicația.
* <b>Lansare și bază de date nouă:</b>
    * fluxul selectare bază → creare schemă → administrator → autorizare → fereastră principală → configurare inițială a fost serializat și verificat;
    * inițializarea incompletă poate fi reluată, iar obiectele SQL create sunt prezentate în jurnal.
* <b>Jurnalizare:</b>
    * scrierea logurilor este sigură între firele de execuție;
    * rotația zilnică, retenția, numele fișierelor și schimbarea căii au fost corectate;
    * mesajele Debug rămân vizibile și în ieșirea Qt Creator.
* <b>Interfață și mentenanță:</b>
    * au fost eliminate clase și modele vechi care nu mai erau utilizate;
    * au fost uniformizate stilurile, meniurile de tipărire și comportamentul ferestrelor;
    * a fost documentată proveniența și licențierea iconurilor distribuite cu aplicația.
* <b>Actualizare importantă:</b> înainte de migrare se recomandă copierea de siguranță a bazei principale, a bazei de imagini și a configurației aplicației. Bazele partajate trebuie actualizate de un singur client, fără utilizarea simultană a versiunilor vechi și noi.

## USG v4.0.1
* <b>Numerotarea documentelor:</b> numerele comenzilor și rapoartelor ecografice sunt alocate separat pentru fiecare an, în formatul <code>n/YYYY</code>.
* <b>Migrarea bazei de date:</b> numerele istorice numerice sunt convertite la formatul <code>n/YYYY</code>, secvențele anuale sunt inițializate din documentele existente, iar unicitatea numerelor include anul documentului.

## USG v3.0.6
* <b>Migrare tehnică:</b> 
    * actualizarea aplicației la Qt v6.9.3.
    * actualizarea [LimeReport](http://limereport.ru/en/index.php) la versiunea 1.7.14
* <b>Raport ecografic:</b> codul documentului a fost reorganizat modular, pentru a facilita dezvoltarea și întreținerea ulterioară.
* <b>Funcționalitate nouă:</b> adăugarea sistemului „Examinarea țesuturilor moi și a ganglionilor limfatici” în formularul documentului Raport ecografic.
* <b>Tipărire:</b> introducerea șablonului de tipar pentru noul sistem de examinare (vezi punctul de mai sus).
* <b>Bug fix:</b> corectarea erorii la adăugarea descrierilor formațiunilor renale.
* <b>Interfață:</b> ajustarea stilului aplicației pentru tema <u>„DARK”</u>.

## USG v3.0.5
* implementată sincronizarea (cu serverul cloud) în fundal a următoarelor documente:
    * Comanda ecografică
    * Raport ecografic
    * Imagini asociate
* adaugată forma pentru crearea arhivului bazei de date <b>.sqlite</b>
* corectat stilul aplicației pentru tema <b>„DARK”</b>.

## USG v3.0.4
* întrodusă forma <b><u>Asistentul de configurare inițială</u></b>, care are rolul de a facilita completarea corectă a bazei de date și de a preveni apariția erorilor de configurare inițială.
* <b><u>Jurnalul de logare</u></b> – a fost implementată filtrarea mesajelor în funcție de nivelul de informație:
    * Info     - informații generale
    * Warning  - mesaje de atenționare
    * Critical - mesaje critice
    * Debug    - informații pentru depanare
    * THREAD   - mesaje provenite din fire de execuție diferite
    * SYNC     - informații privind sincronizarea
* adăugată forma <b><u>Configurare (setare) cloud server</u></b>, care conține datele necesare pentru conectarea și sincronizarea cu <b><u>serverul cloud</u></b>, atunci când este necesar.
* implementată sincronizarea în fundal la validarea datelor pacientului din forma documentului <b><u>Comanda ecografică</u></b>.
* optimizat stilul și fontul aplicației pentru tema <b>„DARK”</b>.
* corectată funcția de trimitere a e-mailului către pacient.
* optimizată funcția de inserare a imaginilor atașate documentelor (cu îmbunătățirea calității acestora).
* fixarea bag-lor minore

## USG v3.0.3
* corectarea erorii din modulele de cautare a pacientilor cu corectarea performantei solicitarilor
* implementarea agentului <b><u>sendEmail</u></b> cu următoarele functionalități de atașare a documentelor:   
    * Comanda ecografică
    * Raport ecografic
    * Imaginile asociate
* ajustarea tabelelor <b><u>tableGestation0</u></b> și <b><u>tableGestation1</u></b>: adaugarea câmpului LMP pentru fixarea datei și calculul automat a vârstei.
* trecerea la standard limbajului C++20
* fixarea bag-lor minore

## USG v3.0.2
* normograme - au fost adaugate datele percentilelor(5,10,25,50,75,90,95):
    * a.uterine 
    * a.ombelicale
    * a.cerebrală medie
    * masa fătului
* document 'Raport ecografic':
    * adaugată descifrarea doppler-ului în dependeță de valoarea a percentilei
    * realizată calcularea automată vârstei gestaționale și a datei probabile a nașterii
* blocarea programei -  a fost implementat mecanismul de blocare a aplicației de către utilizator în timpul pauzei
* fixate bug-rile minore

## USG v3.0.1
* migrarea aplicatiei de la versiunea Qt:5.15.2 la versiunea Qt:6.5.3
* integrarea compatibilității aplicației cu sistemul de operare MacOS (începând cu MacOS Ventura și ulterioare)
* revizuirea radicală a stilului aplicației
* adaugate imaginile de pornire (splash) a aplicației noi 
* în forma lista documentelor <b><u>'Comanda ecografica'</u></b> a fost adaugată colonița '<u>Trimis de ...</u>'
* în documentul 'Raport ecografic' modificate următoarele compartimente:
    * <b><u>organele interne</u></b> - adaugată descrierea <u>anselor intestinale</u>
    * <b><u>sistemul urinar</u></b> - adaugata descrierea <u>glandelor suprarenale</u>
* actualizat <u>generatorul de rapoarte</u> [LimeReport](https://github.com/fralx/LimeReport) până la <U>versiunea 1.7.7</U>
* actualizat driverul [OpenSSL](https://openssl.org/) până la <u>versiunea 3.0.7</u> (este o biblioteca de software pentru criptografie de uz general 
și comunicare sigură ce ţine cont de securitate și confidențialitate a datelor)
* optimizat codul solicitărilor de validarea și completare a documentului 'Raport ecografic'.
* pentru a micșora durata de execuție a interogărilor solicitărilor cu baza de date au fost create indexurile specifice.
* a fost realizata paginarea prezentarii listei de documente <b><u>'Comanda ecografica'</u></b>.
* adaugata posibilitatea pastrarii in sablon a descrierii formatiunilor
* optimizat fontul sabloanelor de tipar 
* a fost adaugat clasificator localităților Republicii Moldova (pentru autocompletarea la întroducerea adresei pacienților).
* adaugată forma de tipar a documentului 'Formarea prețurilor'.

## USG v2.0.9
* realizată descărcarea versiunii noi a aplicației cu prezentarea progress bar-ului în status bar
* adăugată opțiunea de a lansa documente (Formarea prețurilor, Comanda ecografică și Raport ecografic) în fereastra aparte de aplicația (opțiunea în <b><u>'Preferințele utilizatorului'</u></b>)
* realizată minimizarea aplicației în tray (opțiunea în <b><u>'Preferințele utilizatorului'</u></b>)
* corectată întroducerea termenului în sistemul ginecologic și sarcinile (în caz când termenul conține cifre întregi - exemplu: sarcina 10 săptămâni)
* corectată funcția de redactare în documentul <b><u>'Programarea pacienților'</u></b>
* în catalogul <b><u>'Clasificatorul investigațiilor'</u></b> adaugat rechizit nou 'Grupa' pentru gruparea investigațiilor și prezentarea în catalogul <b><u>'Arbore investigațiilor'</u></b>
* adaugată forma catalogului <b><u>'Arbore investigațiilor'</u></b> cu forma liberă de tipar a arborelui
* in catalogul <b><u>'Pacienți'</u></b> modificată lungimea rechizitului <u>'Polița medicală'</u> de la 12 simboluiri până la 20.
* în documentul <b><u>'Raport ecografic'</u></b> corectată masca întroducerii datelor la sistemul obstetrical (vârsta gestațională, fătul corespunde vârstei etc.)

## USG v2.0.8
* fixat bug-ul la prezentarea <b><u>'User Manual'</u></b> (lansarea programei)
* adaugată funcția de prezentare logării prin interpretorul liniei de comandă (cmd) - <b><u>'USG /debug'</u></b>
* documentul <b><u>'Raport ecogrfic'</u></b> - optimizat codul la atașare fișierelor video
* optimizat fontul în lista documentelor (OS Windows)
* fixată problema cu caracterele și simbolurile pentru limba română (OS Windows)
* adaugate imagini pentru metadate (package Linux)
* fixate bug-urile minore

## USG v2.0.7
* fixat bug-ul la printarea formelor de tipar în trimestrul II și III a sarcinei din baza de date MySQL
* realizată vizualizarea istoriei versiunilor (offline)
* adaugată funcția nouă de atașare a fișierelor video la documentul <b><u>'Raport ecografic'</u></b>
* adaugată prezentarea informației suplimentare despre lucrul cu fișiere video și descrierea rapoartelor
* în preferințele utilizatorului adaugată tabela cu alegerea prezentării mesajelor informaționale suplimentare (fișiere video și descrierea rapoartelor)
* modificate datele normogramei de evaluare a translucenței nucale (sursa - <a href="https://fetalmedicine.org/research/assess/nt"><span style=" text-decoration: underline; color:#8ab4f8;">fetalmedicine.org</span></a>)
* adaugate normograme obstetricale noi:
    * doppler a.uterine (sursa - <a href="https://fetalmedicine.org/research/utp"><span style=" text-decoration: underline; color:#8ab4f8;">fetalmedicine.org</span></a>)
    * doppler a.ombelicale (sursa - <a href="https://fetalmedicine.org/research/doppler"><span style=" text-decoration: underline; color:#8ab4f8;">fetalmedicine.org</span></a>)

## USG v2.0.6
* adaugat catalogul cu normograme obstetricale:
    * normograma translucența nucală
    * normograma oasele nazale
    * index lichidului amniotic
* în documentul <b><u>'Raport ecografic'</u></b> este posibil de consultat normogramele

## USG v2.0.5
* modificat documentul <b><u>'Raport ecografic'</u></b> - în document a fost adaugat examen ecografic în trimestru II și III de sarcină
* adaugată forma de tipar trimestru II și III de sarcină (blancul corespunde raportului ecografic al IMSP Institutul Mamei şi Copilului)
* fixarea bug-lui la inchiderea <b><u>'Rapoarte'</u></b> - crash application
* adaugată informație suplimentară în asistentul sfaturlor aplicației
* fixate bug-urile minore

## USG v2.0.4  
* verificarea versiunei noi a aplicației la lansarea aplicației 
* corectarea drumului spre șabloanele de tipar la prima lansare (pentru OS Windows)
* revăzută forma setărilor/preferințelor utilizatorului
* în baza de date adaugată tabela nouă 'userPreferences' pentru păstrarea setărilor utilizatorilor
* adaugat asistentul sfaturlor aplicației cu posibilitatea prezentării la lansarea aplicației 

## USG v2.0.3
* adaugat raport nou <b><u>Structura patologiilor</u></b>  
* optimizată prezentarea elementelor generatorului de rapoarte în dependență de tipul raportului 
* adaugată informația despre licență  
* adaugată raportarea bug-urilor aplicației (online GitHub) în meniu principal a aplicației
* traducerea finală interfeței aplicației în limbra rusă
* adăugat fișierul splash     
* fixate bug-urile minore

## USG v2.0.2  
* redenumirea fișierelor de logare după denumirea bazei de date.
* vizualizarea fișierului de logare în timpul conectării la baza de date MySQL.
* transferarea istoriei versiunilor aplicației online (GitHub).
* lista de documente: Comanda ecografică - realizată prezentarea/ascunderea secțiilor.
* fixarea bug-ului în timpul previzualizării șablonului de tipar în caz când nu este prezentat logotipul, ștampila, semnătura.

## USG v2.0.1
Optimizat codul aplicației compatibil cu [Qt5](https://doc.qt.io/qt-5/qt5-intro.html) / [Qt6](https://doc.qt.io/qt-6/whatsnewqt6.html) cu suportul 
multi-platformă atât în OS Linux cât și OS Windows.  
* Adaugate fonturi (OS Windows):
* Cantarell Bold.ttf  
* Cantarell BoldOblique.ttf  
* Cantarell Oblique.ttf  
* Cantarell Regular.ttf  
... pentru stilul unic de prezentare a formelor de tipar și rapoartelor.
