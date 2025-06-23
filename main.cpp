#include <common/appcontroller.h>
#include <common/appmetatypes.h>

int main(int argc, char *argv[])
{
    // Înregistrează metatipurile la runtime
    qRegisterMetaType<DatesForAgentEmail>("DatesForAgentEmail");

    AppController controller;
    return controller.run(argc, argv);
}
