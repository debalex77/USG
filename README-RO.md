![Platform](https://img.shields.io/badge/platform-Windows%20x64-blue)
![Qt](https://img.shields.io/badge/Qt-6.9.3-brightgreen)
![Latest Release](https://img.shields.io/github/v/release/debalex77/USG)
![Downloads](https://img.shields.io/github/downloads/debalex77/USG/total)
![Status](https://img.shields.io/badge/status-active-success)
![Installer](https://img.shields.io/badge/installer-Inno%20Setup-lightblue)
[![Sponsor](https://img.shields.io/badge/Sponsor-GitHub-ea4aaa?logo=github)](https://github.com/sponsors/debalex77)

---

# 🩺 USG – Evidența investigațiilor ecografice

**USG** este o aplicație desktop open-source destinată gestionării **investigațiilor ecografice** și a **datelor pacienților**.  
Proiectul este orientat spre **stocare locală a datelor**, **structurarea informațiilor medicale** și **generarea de rapoarte medicale personalizate**.

Aplicația este concepută pentru **medici**, **cabinete medicale** și **clinici mici**, care doresc o soluție **offline**, fără dependențe cloud sau servicii SaaS.

---

## 📸 Capturi de ecran

![Fereastra principală](https://github.com/user-attachments/assets/89b3964d-31d1-44ed-bf22-5c2b13642851)
![Evidență pacienți](https://github.com/user-attachments/assets/667abdf9-c456-49e2-91bc-4981ce476da9)

---

## 🎯 Scopul aplicației

USG a fost creat pentru a răspunde necesităților reale din practica ecografică zilnică:

- Crearea unei **baze de date locale și independente** de pacienți
- Stocarea datelor aferente **investigațiilor ecografice**
- **Atașarea imaginilor** (formațiuni volumetrice, calculi, modificări patologice etc.)
- Formarea **istoricului medical ecografic** al pacienților
- Generarea de **formulare și buletine de investigație personalizate**
- Generarea de **rapoarte statistice și financiare**  
  (utile în cazul activității comerciale sau contractuale)

---

## 📖 Despre proiect

USG oferă instrumente pentru:

- gestionarea pacienților
- înregistrarea investigațiilor ecografice
- organizarea locală a datelor medicale
- generarea de rapoarte medicale și formulare tipărite

Aplicația este proiectată cu accent pe **extensibilitate** și poate fi adaptată diferitelor fluxuri de lucru din diagnosticul ecografic.

---

## 🚀 Funcționalități principale

- **Gestionare pacienți** (creare, editare, căutare)
- **Evidența investigațiilor ecografice**
- **Bază de date locală**
- **Atașare imagini ecografice**
- **Rapoarte medicale și administrative**
- **Formulare medicale tipăribile**
- **Șabloane de rapoarte personalizate** folosind LimeReport

---

## 🛠 Tehnologii utilizate

- **C++**
- **Qt Framework**
- Aplicație desktop (**arhitectură cross-platform**)
- **LimeReport** – sistem de raportare pentru:
  - rapoarte medicale
  - formulare tipăribile
  - export PDF
  - layout-uri personalizate

---

## 🧰 Cerințe

| Componentă       | Detalii |
|------------------|---------|
| Qt Framework     | Qt 5.x / Qt 6.x |
| Compilator C++   | C++11 sau mai nou |
| LimeReport       | Necesare pentru rapoarte |
| Sistem operare   | Windows / Linux |

---

## ⚙️ Compilare & Instalare

```bash
git clone https://github.com/debalex77/USG.git
cd USG
