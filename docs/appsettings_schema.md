# Schema configurației aplicației

Configurația principală este un fișier INI citit cu `QSettings`. Numele existente
ale grupurilor și cheilor sunt păstrate pentru compatibilitatea profilurilor create
de versiunile anterioare.

Auditul față de `USG_old` a confirmat că toate grupurile și cheile configurației
principale au aceleași denumiri. În consecință, trecerea la `AppSettingsStore` nu
necesită redenumiri sau copierea cheilor istorice; o migrare cu aliasuri ar adăuga
stări ambigue fără să rezolve o incompatibilitate reală.

## `index_init`

| Cheie | Tip | Implicit | Semnificație |
|---|---:|---:|---|
| `indexLangApp` | `int` (`0..1`) | `1` | `0` = rusă, `1` = română |
| `indexTypeSQL` | `int` (`0..2`) | `0` | necunoscut, MariaDB sau SQLite |
| `indexUnitMeasure` | `int` (`0..1`) | `0` | milimetru sau centimetru |

Valorile din afara intervalelor sunt ignorate și înlocuite cu valoarea implicită,
cu avertizare în log.

Valorile booleene acceptate sunt `true`/`false` și `1`/`0`. Alte valori sunt
înlocuite cu valoarea implicită și produc o avertizare fără expunerea datelor.

## `path_app`

| Cheie | Tip | Implicit | Semnificație |
|---|---:|---:|---|
| `pathTemplatesDocs` | `QString` | gol | directorul șabloanelor LimeReport |
| `pathReports` | `QString` | gol | directorul rapoartelor generate |
| `pathVideo` | `QString` | gol | directorul fișierelor video |

## `connect`

| Cheie | Tip | Implicit | Protecție/Semnificație |
|---|---:|---:|---|
| `MySQL_host` | `QString` | gol | codificare istorică reversibilă |
| `MySQL_name_base` | `QString` | gol | codificare istorică reversibilă |
| `MySQL_port` | `QString` | `3306` | port MariaDB |
| `MySQL_user` | `QString` | gol | codificare istorică reversibilă |
| `MySQL_passwd_user` | `QString` | gol | codificare istorică reversibilă; necesită modernizare separată |
| `MySQL_option_connect` | `QString` | gol | opțiuni driver Qt SQL |
| `sqliteNameBase` | `QString` | gol | numele bazei SQLite |
| `sqlitePathBase` | `QString` | gol | calea bazei SQLite |
| `pathDBImage` | `QString` | gol | calea bazei locale de imagini |
| `pathLogApp` | `QString` | calea calculată | fișierul log al profilului |

La salvare, fișierele existente trebuie să aibă accesul necesar, iar pentru fișierele
noi directorul părinte trebuie să existe și să permită crearea. În modul SQLite sunt
validate separat baza principală și baza locală pentru imagini.

## `on_start`

| Cheie | Tip | Implicit | Semnificație |
|---|---:|---:|---|
| `idUserApp` | `QString` codificat | gol/`0` | ID-ul utilizatorului memorat |
| `nameUserApp` | `QString` codificat | gol | loginul utilizatorului memorat |
| `memoryUser` | `bool` | `false` | activează precompletarea loginului |
| `numSavedFilesLog` | `int` (`0..99`) | `10` | numărul arhivelor log păstrate |

Cele trei valori ale utilizatorului sunt scrise împreună prin
`AppSettings::saveRememberedUser()`.
La debifarea opțiunii, ID-ul și numele codificate sunt eliminate. Dacă opțiunea este
activă, dar ID-ul sau numele sunt invalide, memorarea este dezactivată fără blocarea
autorizării manuale.

Codificarea legacy este păstrată identic pentru compatibilitate în utilitarul
`LegacySettingsCodec`; metodele statice omonime din `DataBase` deleagă către el și
nu deschid sau construiesc o conexiune SQL.
La citire, valorile codificate sunt validate prin recodificare; un profil deteriorat
este respins înainte de aplicarea sa, iar logul conține numai numele cheilor invalide.

## `show_msg`

| Cheie | Tip | Implicit | Semnificație |
|---|---:|---:|---|
| `showMsgVideo` | `bool` | `true` | afișarea informației despre video |
| `showMsgReports` | `bool` | `true` | afișarea informației despre rapoarte |

Cheile sunt scrise prin `AppSettings::saveInfoMessageVisibility()`.

## Date păstrate în alte surse

- Preferințele specifice utilizatorului sunt în tabela `userPreferences`.
- Configurația serverului cloud este în tabela `cloudServer`; parola folosește
  criptarea autentificată gestionată de `CryptoManager`.
- Starea ferestrelor și filtrelor poate fi păstrată în fișiere JSON separate din
  directorul de setări. Aceste fișiere nu fac parte din schema INI de mai sus.
