# Înlocuirea jurnalelor de comenzi și rapoarte

## Stare

Înlocuirea este finalizată pentru versiunea 4.1.0:

- `MainWindow` deschide `OrderView` pentru comenzi și `ReportView` pentru
  rapoarte;
- `ListDocReportOrder`, `DocOrderEcho` și `DocReportEcho` au fost eliminate;
- comenzile folosesc `OrderJournalModel` și `OrderJournalLoader`;
- rapoartele folosesc `ReportJournalModel` și `ReportJournalLoader`;
- adaptoarele tranzitorii `JournalModel` și `JournalLoader`, rămase fără
  consumatori, au fost eliminate înaintea release-ului.

S-a ales păstrarea a două view-uri explicite în locul unei ierarhii UI comune.
Modelele și încărcătoarele separă dialectele SQL și paginarea de widgeturi, iar
diferențele funcționale dintre comenzi și rapoarte rămân vizibile în clasele lor.

## Responsabilități

### `OrderView`

- filtrarea după perioadă, organizație, contract, utilizator și pacient;
- paginarea și păstrarea configurației coloanelor;
- crearea, deschiderea, modificarea și eliminarea comenzilor;
- deschiderea raportului asociat și afișarea concluziei;
- preview/designer pentru comandă și raport, conform preferinței utilizatorului;
- pregătirea și expedierea e-mailului cu PDF-uri și imagini.

### `ReportView`

- filtrarea, paginarea și păstrarea perioadei și a coloanelor;
- crearea și deschiderea rapoartelor;
- accesul controlat la comanda și pacientul asociat;
- eliminarea raportului, cu opțiunea explicită de eliminare a comenzii-părinte;
- preview-ul raportului și al imaginilor atașate;
- ștergerea cloud identificată prin UUID, fără solicitarea UUID-ului de la
  utilizator.

Expedierea e-mailului rămâne intenționat numai în `OrderView`, unde sunt reunite
comanda, raportul și atașamentele.

## Reguli de date

- raportul este copilul Comenzii ecografice;
- operațiile folosesc ID-urile locale numai în baza curentă;
- sincronizarea și ștergerea obiectelor cloud folosesc UUID-ul;
- SQL-ul SQLite și MariaDB este separat în încărcătoare atunci când dialectele
  diferă;
- concluzia nu este coloană permanentă în `ReportView`; este încărcată la
  previzualizarea rândului selectat;
- indicatorul imaginilor este actualizat după validarea raportului asociat.

## Verificări efectuate

- deschidere, filtrare, păstrarea perioadei și paginare;
- creare, modificare, revalidare și eliminare controlată;
- redeschiderea documentelor și încărcarea secțiilor;
- toate sistemele de raport ecografic;
- preview și tipărire pe SQLite și MariaDB;
- previzualizarea și sincronizarea imaginilor;
- export PDF și e-mail prin `OrderView`;
- închiderea aplicației fără coruperea memoriei din fluxul e-mail.

Testele funcționale principale pe SQLite și MariaDB au fost confirmate de
utilizator. Matricea completă a candidatului de release rămâne descrisă în
[`release_4_1_0_plan.md`](release_4_1_0_plan.md).
