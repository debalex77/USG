# TODO

## Idei
- [ ] In fiecarea document unde sunt forme de tipar de adaugat o forma pentru setarile de tipar
- [ ] Clasificatorul 'Investigatii' - de realizat incracarea din .xml, .xlsx, .json (daca cineva
foloseste alt clasificator)
- [ ] In **'table_sections.h'** -> namespace _'ReportSections'_ de realizat o clasa noua pu maparea
codului si flagurilor _'ReportSystem'_ (in caz daca este folosit alt clasificator)
- [ ] De modificat _globals.h_ pu lucru multiuser

------------------------------------------------------------------------------------------------

## Actualizarea v4.0.1
- [ ] De adaugat tabele:
    - [ ] cryptoSplitKey
    - [ ] reportVideo
- [ ] De modificat tabele:
    - [ ] contsOnline redenumirea -> onlineAccount + adaugarea setiei 'tag'
    - [ ] tableGestation2 -> de adaugat sectii noi(fetalPrezentation si multiplePregnancy)

## DataBase_common
-  [ ] De modificat tabele:
    - [ ] gestation0, gestation1 si gestation2 -> de adaugat sectii:
    **'multiplePregnancy'**

## MainWindow
- [ ] Reactivarea ferestrelor MDIareaContainer. Indeplinit dar nu pentru toate clase.

## OnlineAccount
- [x] De creat _OnlineAccountView_ cu vizualizarea mai multor accounturi
- [x] Redenumirea ContsOnline in _OnlineAccountDialog_ si de revazut logica
- [x] De realizat legatura clasei _OnlineAccountDialog_:
    - [x] OnlineAccountView
    - [x] MainWindow

## OrderDialog
- [X] Problema trecerii de la un element la altul in timpul apasarii pe tasta ENTER.
- [ ] Syncronizarea:
    - [x] pacient
    - [ ] documentului propriu-zis
- [x] onValidateDataPatient() - de realizat validarea datelor pacientului
- [x] adaugarea in BD a doctori noi cu actualizarea modelului _modelRefferingDoctors_ ulterioara
- [x] Clasa _'PatientDataSaverWorker'_ de redenumit in _'PatientSaverWorker'_:
    - [x] de revazut inserarea datelor fara indicarea ID (ID trebuie sa fie generat pe seama BD)
    - [x] optimizarea solicitarilor

## OrderView
- [ ] De realizat transmiterea e-mail
- [x] De modificat denumirea _'JournalOrder'_ in _'OrderView'_
- [x] De realizat logica pentru column hide/show
- [x] De realizat functia eliminare a documentului din BD

## UserDialog
- [x] De modificat denumirea catalogului din _'CatUsers'_ in _'UserDialog'_
- [x] De optimizat logica clasei, apoi de realizat logica in _'CatalogView'_

## CatalogDialog
- [ ] fix bug 'address' la deschidere nu se completeaza, posibil solicitarea si maparea sectiilor

## CatalogView
- [x] Logica pentru catalogul _'Users'_
- [x] De realizat logica pentru column hide/show
