## USG v3.0.3
* corectarea erorii din modulele de cautare a pacientilor cu corectarea performantei solicitarilor
* implementarea agentului <b><u>sendEmail</u></b> cu următoarele functionalități de atașare a documentelor:   
    * Comanda ecografică
    * Raport ecografic
    * Imaginile asociate
* ajustarea tabelelor <b><u>tableGestation0</u></b> și <b><u>tableGestation1</u></b>: adaugarea câmpului LMP pentru fixarea datei și calculul automat a vârstei.
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
