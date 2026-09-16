function Component() {}

Component.prototype.createOperations = function() {

    component.createOperations();

    // Qt IFW creează directorul țintă. Nu înregistrăm un Mkdir suplimentar:
    // directorul trebuie păstrat la dezinstalare (RemoveTargetDir=false).

    // variabila
    var applicationsDir = "@HomeDir@/.local/share/applications";
    var desktopFile = applicationsDir + "/org.alovada.usg.desktop";

    component.addOperation("Mkdir", applicationsDir);

    // content pentru desktop file
    var content = [
        "[Desktop Entry]",
        "Version=1.0",
        "Type=Application",
        "Name=USG-Evidența investigațiilor ecografice v4.1.1",
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

    // La actualizare eliminăm shortcutul vechi. La instalarea curată nu
    // înregistrăm o operație Delete fără fișier-sursă, deoarece anularea ei
    // în timpul dezinstalării ar încerca să restaureze un backup inexistent.
    if (installer.fileExists(desktopFile))
        component.addOperation("Delete", desktopFile);

    // adaugă conținutul specificat în fișierul desktopFile
    component.addOperation("AppendFile", desktopFile, content);

}
