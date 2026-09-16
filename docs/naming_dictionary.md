# Dicționar de denumire

Stare: aplicat pentru domeniul pacientului în migrarea `4.1.0`; convențiile pentru
celelalte domenii rămân ghid pentru refactorizări ulterioare.
Principiu: domeniile se migrează vertical și separat; nu se redenumesc simultan
toate obiectele aplicației.

## 1. Convenții

- tabele SQL: engleză, plural, `snake_case`;
- coloane SQL: engleză, `snake_case`;
- chei externe: `<entitate_singular>_id`;
- indecși: `idx_<table>_<columns>`;
- indecși unici: `uq_<table>_<columns>`;
- chei externe: `fk_<child_table>_<parent_table>`;
- view-uri: `v_<entity>_<purpose>`;
- triggere: `trg_<table>_<moment>_<operation>_<purpose>`;
- clase, structuri și enum-uri C++: `PascalCase`;
- metode și variabile locale C++: `camelCase`;
- membri C++: `m_` urmat de `camelCase`;
- textele românești din UI nu sunt tratate ca identificatori tehnici și nu trebuie
  traduse automat.

## 2. Migrarea SQL — domeniul pacientului

### 2.1 Redenumiri aplicate în migrarea 4.1.0

| Vechi | Canonic | Stare | Observație |
|---|---|---|---|
| `pacients` | `patients` | aplicat | corectează pluralul englez |
| `id_pacients` | `patient_id` | aplicat | în toate tabelele dependente |
| `id_patients` | `patient_id` | aplicat | uniformizează `imagesReports` |
| `deletionMark` | `deletion_mark` | aplicat | fără schimbare semantică |
| `IDNP` | `idnp` | aplicat | acronimul rămâne IDNP în UI |
| `medicalPolicy` | `medical_policy` | aplicat | fără schimbare semantică |
| `fullNamePacients` | eliminat | aprobat | căutarea și afișarea folosesc direct `patients` |
| `v_pacients_completer_active` | `v_patients_completer_active` | aplicat | se recreează pe fiecare motor |

### 2.2 Coloanele numelui — semantică confirmată

| Actual | Propus | Condiție |
|---|---|---|
| `name` | `last_name` | reprezintă numele de familie |
| `fName` | `first_name` | reprezintă prenumele |
| `mName` | `middle_name` | reprezintă patronimicul/al doilea prenume; se păstrează |

Semantica și păstrarea celor trei componente ale numelui au fost confirmate de utilizator.

### 2.3 Coloane care se păstrează în prima etapă

| Coloană | Motiv |
|---|---|
| `id` | convenție corectă pentru cheia primară |
| `birthday` | termen clar; alternativul `birth_date` poate fi discutat separat |
| `address` | deja conform |
| `telephone` | poate deveni `phone`, dar nu este necesar pentru migrarea tabelei |
| `email` | deja conform |
| `comment` | deja conform |
| `uuid` | deja conform și critic pentru sincronizare |

## 3. Obiectele dependente

| Obiect actual | Schimbare propusă în această migrare |
|---|---|
| `orderEcho.id_pacients` | `orderEcho.patient_id` |
| `reportEcho.id_pacients` | `reportEcho.patient_id` |
| `imagesReports.id_patients` | `imagesReports.patient_id` |
| `fullNamePacients` și triggerele sale | eliminate după mutarea tuturor consumatorilor pe `patients` |
| `idx_orderEcho_pacients` | `idx_order_echo_patient_id` sau temporar nume compatibil |
| `idx_reportEcho_pacients` | `idx_report_echo_patient_id` |
| `idx_imagesReports_patients` | `idx_images_reports_patient_id` |
| `uq_reportEcho_pacients_number` | recreat cu noua coloană; politica duplicatelor rămâne explicită |
| triggerele pacientului | recreate după noua tabelă/cache |
| view-ul completerului | recreat cu numele și coloanele noi |

Numele tabelelor `orderEcho`, `reportEcho`, `imagesReports` și ale celorlalte domenii
nu se schimbă în aceeași migrare. Ele vor avea dicționare și migrări distincte.

## 4. C++/Qt — redenumiri asociate

### 4.1 Identificatori recomandați

| Actual | Propus |
|---|---|
| `CatalogType::Pacients` | `CatalogType::Patients` |
| `PacientsSections` | `PatientsSections` |
| `PacientsSearchSections` | `PatientSearchColumns` |
| `OrderSections::Id_Patients` | `OrderSections::PatientId` |
| `OrderJournal::Id_Pacient` | `OrderJournal::PatientId` |
| `idPacients` | `patientId` |
| `idPacient` | `patientId` |
| `m_idPacient` | `m_patientId` |
| `m_idPatient` | `m_patientId` |
| `modelPacients` | `patientsModel` |
| `modelPatients` | `patientsModel` |
| `comboPacient`/`comboPacients` | `comboPatient` |
| `loadPacientDetails` | `loadPatientDetails` |
| `DatesCatPatient` | eliminare/consolidare în `PatientDataStructure` |
| `name_parient` | `patientName` |

### 4.2 Identificatori deja potriviți

`PatientDataStructure`, `PatientSaverWorker`, `PatientDataSaverWorker`,
`SyncPatientWorker`, `SyncPatientDataWorker`, `PatientHistory`, `OrderDataStructure::id_patient`
au deja terminologie engleză, dar membrii lor vor fi aduși la convenția `camelCase`
într-o etapă separată de migrarea SQL.

### 4.3 Ordine obligatorie

1. se introduc constante/funcții centrale pentru numele obiectelor SQL;
2. se implementează migrarea tranzacțională pe SQLite;
3. se implementează migrarea tranzacțională pe MariaDB;
4. se actualizează toate resursele SQL;
5. se actualizează identificatorii C++ și mapările pe coloane;
6. se actualizează view-urile, triggerele, sincronizarea, exportul și tipărirea;
7. se compilează și se rulează auditul înainte/după pe ambele copii;
8. versiunea bazei se salvează numai după verificările finale.

## 5. Decizii excluse din această migrare

- redenumirea tuturor celor 52 de tabele;
- refactorizarea completă `AppSettings`;
- redenumirea comenzilor și rapoartelor;
- schimbarea formatului UUID;
- schimbarea tipului sau formatului datei de naștere;
- eliminarea coloanelor nullable doar fiindcă în copia anonimizată sunt goale;
- eliminarea tabelei cache înainte de măsurarea impactului asupra căutării;
- view de compatibilitate cu scriere sub vechiul nume `pacients`.

## 6. Compatibilitate și versiune

Redenumirea este incompatibilă cu executabilele vechi care interoghează direct
`pacients`. De aceea aplicația și schema trebuie livrate atomic într-o versiune nouă.
Propunerea de versiune pentru migrare este `4.1.0`, nu o extindere tăcută a migrării
deja lansate `4.0.1`.

## 7. Starea aplicării

- `pacients → patients`, coloanele numelui și cheile `patient_id`: aplicate;
- `middle_name`: păstrat conform deciziei utilizatorului;
- `fullNamePacients`: eliminat, iar consumatorii activi folosesc `patients`;
- view-ul, indexurile și cheile externe ale domeniului pacientului: recreate;
- identificatorii C++ strict necesari domeniului pacientului: actualizați;
- uniformizarea celorlalte domenii și a identificatorilor C++ rămași: etapă
  ulterioară, fără extinderea migrării 4.1.0.
