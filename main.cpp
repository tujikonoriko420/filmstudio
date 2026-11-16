#include <windows.h>
#include "plugin.h"
#include "Hooks.h"
#include "Settings.h"
#include "Log.h"
#include "NPCDatabase.h"
#include "UIManager.h"

class FilmStudio {
public:
    FilmStudio() {
        Settings::Load();
        FSLog("---- GTA SA Film Studio iniciado ----");
        UIManager::Init();
        plugin::Events::initGameEvent += [] { NPCDatabase::Load(); }; // apos o jogo carregar modelos
        Hooks::Install();
    }
} g_filmStudio;

BOOL APIENTRY DllMain(HMODULE, DWORD, LPVOID) { return TRUE; }
