#include "Settings.h"
#include <windows.h>
namespace Settings {
    int menuKey = VK_F9;
    int camKey = VK_F8;
    static const char* ini = ".\\FilmStudio.ini";
    void Load() {
        menuKey = GetPrivateProfileIntA("Keys", "Menu", VK_F9, ini);
        camKey  = GetPrivateProfileIntA("Keys", "FreeCam", VK_F8, ini);
    }
    void Save() {
        char b[16];
        wsprintfA(b, "%d", menuKey); WritePrivateProfileStringA("Keys", "Menu", b, ini);
        wsprintfA(b, "%d", camKey);  WritePrivateProfileStringA("Keys", "FreeCam", b, ini);
    }
}
