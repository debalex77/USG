#include <common/appcontroller.h>
#include <common/appmetatypes.h>

int main(int argc, char *argv[])
{
    // Înregistrează metatipurile la runtime
    qRegisterMetaType<DatesCatPatient>("DatesCatPatient");
    qRegisterMetaType<DatesForAgentEmail>("DatesForAgentEmail");
    qRegisterMetaType<DatesDocForExportEmail>("DatesDocForExportEmail");
    qRegisterMetaType<DatesDocsOrderReportSync>("DatesDocsOrderReportSync");

    AppController controller;
    return controller.run(argc, argv);
}
