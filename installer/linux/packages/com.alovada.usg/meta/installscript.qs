function Component() {}

Component.prototype.createOperations = function() {
    component.createOperations();

    // Crearea directorului țintă
    component.addOperation("Mkdir", "@TargetDir@");

    var desktopFile = "@HomeDir@/.local/share/applications/org.alovada.usg.desktop";
    var content = [
        "[Desktop Entry]",
        "Version=3.0.3",
        "Type=Application",
        "Name=USG-Evidența investigațiilor ecografice",
        "Comment[en]=USG-Evidence of ultrasound investigations",
        "Comment[ru]=УЗИ-Учёт ультразвуковых исследований",
        "Comment[ro]=USG-Evidența investigațiilor ecografice",
        "Exec=@TargetDir@/USG.sh",
        "Icon=@TargetDir@/icons/eco_248x248.ico",
        "Terminal=false",
        "Categories=Utility;Database;MedicalSoftware;Qt;",
        "StartupNotify=true",
        "StartupWMClass=USG",
        "Encoding=UTF-8"
    ].join("\n");

    // adaugă conținutul specificat în fișierul desktopFile
    component.addOperation("AppendFile", desktopFile, content);

    // Setarea permisiunilor de execuție
    component.addOperation("Execute", "chmod", "+x", desktopFile);

    // Reîmprospătarea bazei de date a meniului
    component.addOperation("Execute", "update-desktop-database", "@HomeDir@/.local/share/applications/");

}
