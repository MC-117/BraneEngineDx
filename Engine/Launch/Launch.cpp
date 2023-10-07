#pragma once
#include "Core/Application.h"
#include <tchar.h>

void main(int argc, char** argv)
{
    if (Engine::windowContext.hinstance == NULL)
        Engine::windowContext.hinstance = GetModuleHandle(NULL);
    AppArguments::get().init(argc, argv);
    Application::get().main();
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    Engine::windowContext.hinstance = hInstance;
    main(__argc, __targv);
}