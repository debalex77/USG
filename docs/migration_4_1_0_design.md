# Proiectarea migrării 4.1.0 — domeniul pacientului

Stare: implementată în `UpdateReleasesApp::update_4_1_0()` și verificată prin
aplicație pe copii de test SQLite și MariaDB. Documentul păstrează contractul de
siguranță și deciziile folosite la implementare.

## 1. Domeniul migrării

Migrarea 4.1.0 schimbă numai domeniul pacientului:

- `pacients` → `patients`;
- `name` → `last_name`;
- `fName` → `first_name`;
- `mName` → `middle_name` și coloana se păstrează;
- `deletionMark` → `deletion_mark`;
- `IDNP` → `idnp`;
- `medicalPolicy` → `medical_policy`;
- `id_pacients`/`id_patients` → `patient_id` în obiectele dependente;
- eliminarea `fullNamePacients` și a celor două triggere care îl întrețin;
- recrearea indexurilor, FK-urilor și view-ului aferent.

Nu intră în această migrare redenumirea tabelelor pentru comenzi, rapoarte, imagini,
utilizatori sau organizații.

## 2. Valori capturate înainte de primul DDL

- numărul de rânduri din `pacients`, `fullNamePacients`, `orderEcho`, `reportEcho` și
  `imagesReports`;
- valorile maxime ale cheilor primare;
- numărul UUID-urilor nule, invalide și duplicate;
- numărul relațiilor orfane pentru fiecare tabel dependent;
- numărul valorilor nenule pentru coloanele opționale;
- lista FK-urilor, indexurilor, view-urilor și triggerelor existente.

Orice relație orfană, UUID invalid sau coloană obligatorie lipsă oprește migrarea
înainte de modificarea schemei. Curățarea datelor nu va fi implicită.

## 3. SQLite — strategie tranzacțională

SQLite va folosi reconstrucția controlată a tabelelor într-o singură tranzacție:

1. `PRAGMA foreign_keys` este citit și memorat;
2. începe tranzacția;
3. sunt eliminate view-ul și triggerele dependente;
4. se creează `patients_new` cu schema finală;
5. se copiază explicit coloanele din `pacients`, păstrând `id` și `uuid`;
6. se reconstruiesc pe rând `orderEcho`, `reportEcho` și `imagesReports` cu
   `patient_id`; `fullNamePacients` nu este copiat;
7. se copiază datele prin liste explicite de coloane, fără `SELECT *`;
8. se elimină tabelele vechi și se redenumesc tabelele `_new`;
9. se recreează indexurile, view-ul și triggerele;
10. se rulează verificările de număr, ID, UUID și `PRAGMA foreign_key_check`;
11. se execută commit numai dacă toate verificările reușesc;
12. starea inițială `PRAGMA foreign_keys` este restabilită în afara tranzacției.

La orice eroare se execută rollback. Numele temporare fixe sunt detectate înainte de
start; prezența lor indică o tentativă veche incompletă și oprește migrarea cu mesaj
explicit.

## 4. MariaDB — strategie reluabilă pe faze

MariaDB nu oferă rollback atomic pentru toate operațiile DDL. Migrarea trebuie să fie
idempotentă și verificată după fiecare fază:

1. verificare pre-migrare completă;
2. eliminarea controlată a view-ului și triggerelor dependente;
3. eliminarea FK-urilor către `pacients` după numele descoperite din
   `information_schema`, nu după presupuneri;
4. redenumirea tabelei și coloanelor numai dacă există numele vechi și lipsește cel
   nou;
5. redenumirea cheilor din tabelele dependente cu aceeași regulă idempotentă;
6. eliminarea `fullNamePacients` după confirmarea că nu mai are consumatori;
7. recrearea FK-urilor, indexurilor și view-ului cu denumiri canonice;
8. verificări post-migrare complete;
9. salvarea versiunii numai după succes.

Reluarea determină starea reală din `information_schema`:

- obiect vechi prezent, nou absent → execută faza;
- obiect vechi absent, nou prezent → faza este deja executată;
- ambele prezente sau ambele absente → stare ambiguă, oprire fără corecție automată;
- constrângere/index existent cu definiție greșită → oprire și diagnostic, nu
  suprascriere oarbă.

## 5. Actualizarea codului în aceeași versiune

În aceeași versiune au fost actualizați consumatorii activi ai schemei:

- resurse SQL pentru catalog, căutare, inserare și actualizare;
- jurnalele comenzilor și rapoartelor pentru ambele dialecte;
- tipărire, export PDF și e-mail;
- sincronizarea pacientului, comenzii și raportului;
- modelele și enum-urile care mapează coloane după poziție;
- view-uri, triggere și schema de instalare pentru baze noi;
- dialogurile noi și implementările vechi încă incluse în proiect.

Aplicația 4.1.0 nu trebuie lansată cu o jumătate din interogări pe schema veche.

## 6. Verificări post-migrare

- toate valorile de control coincid;
- `pacients`, `id_pacients`, `id_patients` și `v_pacients_completer_active` nu mai
  există ca obiecte active ale schemei;
- `patients` și toate coloanele finale există;
- fiecare document și imagine indică un pacient valid;
- fiecare UUID are 16 octeți și este unic în tabela sa;
- view-ul poate fi interogat;
- triggerele produc/actualizează corect intrarea cache;
- inserarea, modificarea, căutarea și deschiderea pacientului funcționează;
- comenzile și rapoartele vechi se deschid și se tipăresc;
- sincronizarea este testată fără regenerarea UUID-urilor;
- a doua lansare nu repetă migrarea.

## 7. Comportament la întrerupere

- SQLite: tranzacția nefinalizată este anulată de motor; versiunea rămâne veche;
- MariaDB: versiunea rămâne veche, iar relansarea reevaluează fiecare fază din schema
  reală și continuă numai dintr-o stare neambiguă;
- niciun pas nu presupune că o eroare înseamnă automat că obiectul există deja;
- aplicația nu deschide ferestrele de lucru dacă migrarea nu a trecut verificările;
- diagnosticul include versiunea, faza, obiectul și eroarea SQL.

## 8. Starea implementării

- migrarea SQLite este executată într-o tranzacție și face rollback la eroare;
- migrarea MariaDB verifică starea obiectelor înaintea fiecărui DDL și poate fi
  reluată numai din stări neambigue;
- `middle_name` este păstrat, iar `fullNamePacients` și triggerele sale sunt
  eliminate;
- numărul pacienților, comenzilor și rapoartelor, UUID-urile și relațiile sunt
  verificate înainte de salvarea versiunii finale;
- fluxurile funcționale principale au fost confirmate pe copii SQLite și MariaDB;
- testele complete de release, inclusiv întreruperea controlată și măsurarea
  duratei pe volum reprezentativ, rămân în
  [`release_4_1_0_plan.md`](release_4_1_0_plan.md).
