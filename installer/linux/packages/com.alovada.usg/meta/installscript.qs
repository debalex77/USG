function Component() {}

Component.prototype.createOperations = function() {
    component.createOperations();

    // Crearea directorului țintă
    component.addOperation("Mkdir", "@TargetDir@");

    var desktopFile = "@TargetDir@/org.alovada.usg.desktop";
    var applicationsDir = "@HomeDir@/.local/share/applications/org.alovada.usg.desktop";

    // Copierea fișierului .desktop în ~/.local/share/applications/
    component.addOperation("Copy", desktopFile, applicationsDir);

    // Setarea permisiunilor de execuție
    component.addOperation("Execute", "chmod", "+x", applicationsDir);

    // Reîmprospătarea bazei de date a meniului
    component.addOperation("Execute", "update-desktop-database", "@HomeDir@/.local/share/applications/");

}
