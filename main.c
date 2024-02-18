#include "app.h"

int main(int argc, char *argv[]) {
    struct App app = { 0 };
    init_app(&app);
    app_run(&app);
    destroy_app(&app);
    return 0;
}
