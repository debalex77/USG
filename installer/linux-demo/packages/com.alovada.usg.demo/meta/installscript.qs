function Component() {}

Component.prototype.createOperations = function() {
    component.createOperations();

    var applicationsDir = "@HomeDir@/.local/share/applications";
    var desktopFile = applicationsDir + "/org.alovada.usg.demo.desktop";

    component.addOperation("Mkdir", applicationsDir);

    var desktopEntry = [
        "Version=1.0",
        "Type=Application",
        "Name=USG Demo",
        "Name[ro]=USG Demo",
        "Name[ru]=USG Демо",
        "Comment=Versiune demonstrativă pentru evidența examinărilor ecografice",
        "Comment[en]=Demo version for ultrasound examination records",
        "Comment[ru]=Демонстрационная версия для учёта ультразвуковых исследований",
        "Exec=@TargetDir@/USG-Demo.sh",
        "Icon=@TargetDir@/icons/eco_248x248.ico",
        "Terminal=false",
        "Categories=Science;MedicalSoftware;Qt;",
        "StartupNotify=true",
        "StartupWMClass=USG"
    ].join("\n");

    component.addOperation("CreateDesktopEntry", desktopFile, desktopEntry);

    // Profilul este creat după copierea fișierelor pachetului. Helperul face
    // o copie de siguranță dacă exista deja un profil demo.
    component.addOperation("Execute",
                           "@TargetDir@/demo-tools/configure-demo-profile.sh",
                           "@TargetDir@",
                           "--force");
}
