function Component() {}

Component.prototype.createOperations = function() {

    component.createOperations();

    // Crearea directorului țintă
    component.addOperation("Mkdir", "@TargetDir@");

    // variabila
    var desktopFile = "@HomeDir@/.local/share/applications/org.alovada.usg.desktop";

    // content pentru desktop file
    var content = [
        "[Desktop Entry]",
        "Version=3.0.3",
        "Type=Application",
        "Name=USG-Evidența investigațiilor ecografice v3.0.3",
        "Comment[en]=USG-Evidence of ultrasound investigations v3.0.3",
        "Comment[ru]=УЗИ-Учёт ультразвуковых исследований v3.0.3",
        "Comment[ro]=USG-Evidența investigațiilor ecografice v3.0.3",
        "Exec=@TargetDir@/USG.sh",
        "Icon=@TargetDir@/icons/eco_248x248.ico",
        "Terminal=false",
        "Categories=Utility;Database;MedicalSoftware;Qt;",
        "StartupNotify=true",
        "StartupWMClass=USG",
        "Encoding=UTF-8"
    ].join("\n");

    // eliminam desktopFile
    component.addOperation("Delete", desktopFile);

    // adaugă conținutul specificat în fișierul desktopFile
    component.addOperation("AppendFile", desktopFile, content);

    // Setarea permisiunilor de execuție
    component.addOperation("Execute", "chmod", "+x", desktopFile);

    // Reîmprospătarea bazei de date a meniului
    component.addOperation("Execute", "update-desktop-database", "@HomeDir@/.local/share/applications/");

}
