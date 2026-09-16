# Audit `UpdateReleasesApp`

Data: 2026-09-05

## Responsabilități curente

Clasa îndeplinește în prezent patru roluri:

1. validează versiunea curentă și stabilește migrările necesare;
2. orchestrează migrările în ordine;
3. conține implementarea SQL/procedurală a tuturor migrărilor 2.x–4.x;
4. repară unele obiecte auxiliare, precum view-urile lipsă.

Salvarea versiunii finale nu este făcută de clasă, ci de `MainWindow`, numai dacă
`execUpdateCurrentRelease()` reușește. Această regulă trebuie păstrată.

## Consolidare realizată fără schimbarea migrărilor

- migrările sunt declarate într-o listă ordonată `versiune → metodă`;
- se execută numai pașii mai noi decât baza și cel mult egali cu versiunea aplicației;
- o bază cu versiune mai nouă decât aplicația este refuzată explicit;
- fiecare pas produce log de început, succes sau eșec;
- prima eroare oprește lanțul și împiedică salvarea versiunii finale;
- conexiunea, driverul și motorul selectat sunt validate înaintea migrărilor;
- înainte de 4.0.1 sunt verificate toate tabelele necesare documentelor și view-urilor,
  iar lipsurile opresc procesul înainte de primul DDL;
- după 4.0.1 se verifică păstrarea numărului de comenzi și rapoarte, existența
  `doc_sequences`, a coloanelor `docYear` și a view-urilor obligatorii;
- UUID-urile sunt validate ca prezență, lungime de 16 octeți și unicitate;
- relațiile dintre pacienți, comenzi și rapoarte sunt validate înainte de salvarea
  versiunii finale;
- ordinea și corpurile migrărilor istorice au rămas neschimbate.

## Probleme istorice care nu trebuie copiate în migrarea 4.1.0

- unele migrări vechi execută `commit()` fără pornirea explicită a tranzacției;
- unele apeluri SQL vechi ignoră rezultatul `exec()` și continuă după eroare;
- `update_3_0_1()` este compus din metode `void`, deci nu poate propaga toate erorile;
- unele resurse vechi folosesc căi istorice care nu mai corespund organizării actuale;
- MariaDB face implicit commit pentru numeroase operații DDL; o redenumire complexă
  nu poate pretinde rollback tranzacțional identic cu SQLite;
- repararea view-urilor și migrarea versiunii sunt încă în aceeași clasă.

## Cerințe pentru migrarea 4.1.0

- metodă distinctă `update_4_1_0()`;
- verificări pre-migrare înainte de primul DDL;
- fiecare instrucțiune verificată și eroarea returnată imediat;
- SQLite: tranzacție unică și `PRAGMA foreign_key_check` înainte de commit;
- MariaDB: plan pe faze, verificări după fiecare DDL și reluare idempotentă;
- verificări post-migrare pentru număr de rânduri, UUID-uri și relații;
- recrearea controlată a indexurilor, FK-urilor, view-urilor și triggerelor;
- salvarea versiunii numai după toate verificările post-migrare;
- mesaje de progres distincte pentru fiecare fază.
