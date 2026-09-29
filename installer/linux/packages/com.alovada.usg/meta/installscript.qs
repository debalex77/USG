function Component() {}

Component.prototype.createOperations = function() {

    component.createOperations();

    // Qt IFW creează directorul țintă. Nu înregistrăm un Mkdir suplimentar:
    // directorul trebuie păstrat la dezinstalare (RemoveTargetDir=false).

    // variabila
    var applicationsDir = "@HomeDir@/.local/share/applications";
    var desktopFile = applicationsDir + "/org.alovada.usg.desktop";

    component.addOperation("Mkdir", applicationsDir);

    // CreateDesktopEntry înlocuiește intrarea ca un singur fișier gestionat de
    // installer. AppendFile nu este potrivit aici: la fiecare actualizare
    // concatenează încă o secțiune [Desktop Entry] și lasă copii de rollback.
    var desktopEntry = [
        "Version=1.0",
        "Type=Application",
        "Name=USG - Evidența investigațiilor ecografice",
        "Comment=Gestionarea pacienților și a rapoartelor ecografice",
        "Comment[en]=Manage patients and ultrasound examination reports",
        "Comment[ru]=Управление пациентами и протоколами УЗИ",
        "Comment[ro]=Gestionarea pacienților și a rapoartelor ecografice",
        "Exec=@TargetDir@/USG.sh",
        "Icon=@TargetDir@/icons/eco_248x248.ico",
        "Terminal=false",
        "Categories=Science;MedicalSoftware;Qt;",
        "StartupNotify=true",
        "StartupWMClass=USG"
    ].join("\n");

    component.addOperation("CreateDesktopEntry", desktopFile, desktopEntry);

}
