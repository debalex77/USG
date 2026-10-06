# Pachetul Linux demo

Acest director conține numai configurația instalatorului demo. Nu este folosit
de scripturile și instalatoarele de producție.

Pachetul este construit cu:

```bash
./build_scripts/build_demo
```

Intrări implicite:

- `demo/usg_demo.sqlite3` — baza principală demo;
- `demo/usg_demo_images.sqlite3` — baza opțională cu imaginile demo.

Dacă baza de imagini nu există, scriptul creează în pachet o bază goală cu
schema curentă `imagesReports`. Baza principală sursă nu este modificată.

Fișierul de imagini este inclus numai dacă toate rândurile lui indică pacienți,
comenzi și rapoarte existente în baza principală demo. Verificarea nu deschide
și nu afișează imaginile. Un export/copiere brută din baza de producție nu
trebuie redenumită și inclusă înainte de anonimizare și verificare.

La instalare este creat automat profilul:

```text
~/.config/USG/usg_demo.conf
```

Profilul indică bazele din directorul de instalare și memorează loginul
`admin`. Contul demo nu are parolă.
