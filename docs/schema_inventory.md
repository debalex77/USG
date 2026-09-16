# Inventarul schemei și al dependențelor — înainte de refactorizare

Data inventarierii: 2026-09-05
Surse analizate: proiectul `USG`, resursele SQL și codul C++/Qt.
Limită: bazele de date ale utilizatorului nu au fost accesate sau modificate.

## 1. Obiectele declarate în schemă

Ambele motoare conțin 57 instrucțiuni `CREATE TABLE`, organizate în aceleași 52 de
obiecte logice:

`cloudServer`, `conclusionTemplates`, `constants`, `contracts`, `cryptoSplitKey`,
`doc_sequences`, `doctors`, `formationsSystemTemplates`, `fullNameDoctors`,
`fullNameNurses`, `fullNamePacients`, `imagesReports`, `investigations`,
`investigationsGroup`, `normograms`, `nurses`, `onlineAccount`, `orderEcho`,
`orderEchoPresentation`, `orderEchoTable`, `organizations`, `pacients`, `pricings`,
`pricingsPresentation`, `pricingsTable`, `registrationPatients`, `reportEcho`,
`reportEchoPresentation`, `reportVideo`, `settingsUsers`, `tableBladder`,
`tableBreast`, `tableCholecist`, `tableGestation0`, `tableGestation1`,
`tableGestation2`, `tableGestation2_SNC`, `tableGestation2_abdomen`,
`tableGestation2_biometry`, `tableGestation2_cranium`, `tableGestation2_doppler`,
`tableGestation2_heart`, `tableGestation2_other`, `tableGestation2_thorax`,
`tableGestation2_urinarySystem`, `tableGynecology`, `tableIntestinalLoop`,
`tableKidney`, `tableLiver`, `tablePancreas`, `tableProstate`,
`tableSofTissuesLymphNodes`, `tableSpleen`, `tableThyroid`, `typesPrices`,
`userPreferences`, `users`.

Cele șapte view-uri declarate pe fiecare motor sunt:

- `v_contracts_listView_active`;
- `v_doctors_active`;
- `v_nurses_active`;
- `v_organizations_active`;
- `v_pacients_completer_active`;
- `v_types_prices_active`;
- `v_users_combo_active`.

Cele 12 triggere sunt perechile insert/update pentru prezentarea comenzilor,
rapoartelor și prețurilor și pentru numele complete ale pacienților, doctorilor și
asistentelor.

## 2. Domeniul pacientului — schema actuală

Tabela `pacients` are aceeași ordine logică a coloanelor pe ambele motoare:

| Coloană actuală | SQLite | MariaDB | NULL/implicit declarat |
|---|---|---|---|
| `id` | `INTEGER`, PK autoincrement | `BIGINT UNSIGNED`, PK auto increment | obligatoriu |
| `deletionMark` | `INTEGER` | `TINYINT(1)` | `NOT NULL DEFAULT 0` |
| `IDNP` | `TEXT` | `VARCHAR(20)` | permis |
| `name` | `TEXT` | `VARCHAR(80)` | `NOT NULL` |
| `fName` | `TEXT` | `VARCHAR(50)` | permis |
| `mName` | `TEXT` | `VARCHAR(50)` | permis |
| `medicalPolicy` | `TEXT` | `VARCHAR(20)` | permis |
| `birthday` | `TEXT` | `DATE` | `NOT NULL` |
| `address` | `TEXT` | `VARCHAR(255)` | permis |
| `telephone` | `TEXT` | `VARCHAR(100)` | permis |
| `email` | `TEXT` | `VARCHAR(100)` | permis |
| `comment` | `TEXT` | `VARCHAR(255)` | permis |
| `uuid` | `BLOB` | `BINARY(16)` | `NOT NULL`, unic |

Obiecte cu relație directă către pacient:

| Obiect | Cheie actuală | Comportament declarat |
|---|---|---|
| `fullNamePacients` | `id_pacients` | FK, `ON DELETE CASCADE` |
| `orderEcho` | `id_pacients` | FK către `pacients.id` |
| `reportEcho` | `id_pacients` | FK către `pacients.id` |
| `imagesReports` | `id_patients` | FK numai în schema MariaDB declarată |
| `v_pacients_completer_active` | `pacients.id` | view pentru căutare/completer |
| triggerele pacientului | `NEW.id` → `fullNamePacients.id_pacients` | materializează textele de căutare |

Indexurile specifice pacientului includ UUID, IDNP, polița medicală, nume/prenume,
telefon și cheile externe din comenzile, rapoartele și imaginile asociate.

## 3. Dependențe în resurse și cod

- 28 de fișiere SQL conțin referințe directe la `pacients`, `id_pacients` sau
  view-ul pacientului.
- 38 de fișiere C++/header/UI conțin identificatori din familia
  `pacient/pacients/patient/patients`.
- Zone afectate: catalogul pacienților, istoricul pacientului, comenzile, rapoartele,
  imaginile rapoartelor, tipărirea, modelele jurnalelor, căutarea/completerele,
  salvarea asincronă și sincronizarea locală/cloud.
- Clase/fluxuri cu risc ridicat: `CatalogDialog`, `PatientHistory`, `OrderDialog`,
  `ReportDialog`, implementările vechi `DocOrderEcho`/`DocReportEcho`,
  `PatientDataSaverWorker`, `PatientSaverWorker`, `SyncPatientDataWorker`,
  `SyncPatientWorker`, `SyncOrderWorker`, `SyncDocsWorker` și `DocSyncWorker`.
- În resurse există SQL separat pe motor pentru căutarea și citirea pacientului, dar
  mai există și interogări comune, plus SQL construit direct în C++.

## 4. Diferențe și riscuri confirmate

1. SQLite stochează data nașterii ca text, MariaDB ca `DATE`; migrarea nu poate copia
   expresia SQL textual între motoare.
2. SQLite declară indexul compus `name, fName, IDNP`; acesta lipsește din declarația
   MariaDB a tabelei `pacients`.
3. SQLite declară mai multe indexuri compuse pentru `imagesReports`; MariaDB declară
   doar indexurile simple.
4. MariaDB declară chei externe pentru `imagesReports`; fișierul SQLite nu declară
   chei externe pentru acest tabel.
5. Cheia pacientului nu este uniformă: `id_pacients` în documente și
   `fullNamePacients`, dar `id_patients` în `imagesReports`; codul mai folosește și
   `idPacient`, `idPatient`, `m_idPacient` și `m_idPatient`.
6. `fullNamePacients` dublează date derivate din `pacients` și depinde de triggere;
   utilitatea sa trebuie demonstrată înainte de păstrare sau eliminare.
7. `mName` este declarat, dar conform utilizării comunicate este permanent `NULL`;
   este candidat pentru eliminare numai după auditul datelor și al importurilor.
8. În schema MariaDB, indexul unic UUID din `contracts` poartă greșit numele
   `uq_pacients_uuid`; obiectul indexează `contracts.uuid`, dar denumirea induce în
   eroare și poate provoca conflicte la migrare.
9. Casing-ul `fName`/`fname`, denumirile camelCase și pluralurile română/engleză sunt
   amestecate în SQL, C++ și UI.
10. View-urile și triggerele folosesc sintaxe diferite pe motor și trebuie recreate,
    nu doar redenumite textual.
11. Sincronizarea identifică entități și prin UUID; redenumirea trebuie să păstreze
    UUID-urile exact, nu să le regenereze.

## 5. Valori de control deja confirmate de utilizator

| Copie testată | Pacienți | Comenzi | Rapoarte | Imagini rapoarte |
|---|---:|---:|---:|---:|
| SQLite | 6.358 | 8.110 | 7.476 | neînregistrat |
| MariaDB | 5.485 | 6.629 | 6.010 | 297 |

În MariaDB au fost detectate 15 grupuri istorice duplicate în `orderEcho` și un grup
în `reportEcho`. Valorile au fost păstrate. Numărul relațiilor orfane și numărul
rândurilor din toate tabelele dependente nu sunt încă măsurate, deoarece baza nu a
fost oferită pentru acces direct.

## 6. Hartă preliminară pentru etapa de denumire

Aceasta este o propunere de lucru, nu o migrare aprobată:

| Obiect actual | Obiect propus | Consumatori principali |
|---|---|---|
| `pacients` | `patients` | toate interogările pacientului, documente, sync, print |
| `id_pacients` | `patient_id` | `fullNamePacients`, `orderEcho`, `reportEcho`, cod C++ |
| `id_patients` | `patient_id` | `imagesReports`, codul imaginilor |
| `fullNamePacients` | de decis | triggere, completere/căutare |
| `v_pacients_completer_active` | `v_patients_completer_active` | modele și comboboxuri |
| `IDNP` | `idnp` | căutare, catalog, documente, print, sync |
| `fName` | de clarificat semantic | aproape toate afișările numelui |
| `mName` | de verificat/eliminat | catalog, import/sync posibil |
| `medicalPolicy` | `medical_policy` | catalog, documente, sync |
| `deletionMark` | `deletion_mark` | toate filtrele de înregistrări active |

## 7. Condiții înainte de migrare

- aprobarea semanticii pentru `name`, `fName` și `mName`;
- obținerea prin interogări executate de utilizator a numărului de rânduri dependente
  și a relațiilor orfane;
- alegerea versiunii de schemă ulterioare lui 4.0.1;
- definirea ordinii exacte de recreare a FK-urilor, indexurilor, view-urilor și
  triggerelor pentru fiecare motor;
- aprobarea dicționarului final `actual → propus`;
- testarea exclusiv pe copiile pregătite de utilizator.
