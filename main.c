#include "app.h"

int main(int argc, char *argv[]) {
    struct App *app = create_app();
    app_run(app);
    destroy_app(app);
    return 0;
}
