#include "application.h"

int main(int argc, char *argv[]) {
    Application *app = createApplication();
    applicationRun(app);
    destroyApplication(app);

    return 0;
}
