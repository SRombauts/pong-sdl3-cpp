#include "Application.h"

int main()
{
    Application app("Pong SDL3 C++", 800, 600);
    if (!app.init())
    {
        return 1;
    }

    return app.run();
}
