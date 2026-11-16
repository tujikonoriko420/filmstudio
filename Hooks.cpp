#include "Hooks.h"
#include "Log.h"
#include <windows.h>
#include <d3d9.h>
#include "MinHook.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace Hooks {
    std::function<void()> onFrame;
    bool menuOpen = false;

    typedef HRESULT(__stdcall* EndScene_t)(IDirect3DDevice9*);
    typedef HRESULT(__stdcall* Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
    typedef BOOL(WINAPI* SetCursorPos_t)(int, int);
    static EndScene_t oEndScene; static Reset_t oReset; static SetCursorPos_t oSetCursorPos;
    static WNDPROC oWndProc; static HWND gHwnd; static bool imguiReady;

    static LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
        if (imguiReady && menuOpen) {
            ImGui_ImplWin32_WndProcHandler(h, m, w, l);
            switch (m) { // engole input de mouse enquanto o menu esta aberto
            case WM_SETCURSOR: SetCursor(LoadCursor(nullptr, IDC_ARROW)); return TRUE;
            case WM_MOUSEMOVE: case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_RBUTTONDOWN:
            case WM_RBUTTONUP: case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MOUSEWHEEL:
                return 0;
            }
        }
        return CallWindowProcA(oWndProc, h, m, w, l);
    }

    static BOOL WINAPI hSetCursorPos(int x, int y) {
        if (menuOpen) return TRUE; // GTA recentraliza o mouse todo frame
        return oSetCursorPos(x, y);
    }

    static void InitImGui(IDirect3DDevice9* dev) {
        D3DDEVICE_CREATION_PARAMETERS cp; dev->GetCreationParameters(&cp);
        gHwnd = cp.hFocusWindow;
        ImGui::CreateContext();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui::StyleColorsDark();
        ImGui_ImplWin32_Init(gHwnd);
        ImGui_ImplDX9_Init(dev);
        oWndProc = (WNDPROC)SetWindowLongPtrA(gHwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
        imguiReady = true;
        FSLog("ImGui inicializado");
    }

    static HRESULT __stdcall hEndScene(IDirect3DDevice9* dev) {
        static bool once = false;
        if (!once) { once = true; InitImGui(dev); }
        if (imguiReady) {
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();
            ImGui::GetIO().MouseDrawCursor = menuOpen;
            if (menuOpen) { // GTA usa DirectInput; le o cursor do Windows direto
                POINT p; GetCursorPos(&p); ScreenToClient(gHwnd, &p);
                ImGui::GetIO().AddMousePosEvent((float)p.x, (float)p.y);
            }
            if (onFrame) onFrame();
            ImGui::EndFrame();
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
        }
        return oEndScene(dev);
    }

    static HRESULT __stdcall hReset(IDirect3DDevice9* dev, D3DPRESENT_PARAMETERS* pp) {
        if (imguiReady) ImGui_ImplDX9_InvalidateDeviceObjects();
        HRESULT r = oReset(dev, pp);
        if (imguiReady && SUCCEEDED(r)) ImGui_ImplDX9_CreateDeviceObjects();
        return r;
    }

    static DWORD WINAPI InstallThread(LPVOID) {
        WNDCLASSA wc{}; wc.lpfnWndProc = DefWindowProcA; wc.hInstance = GetModuleHandleA(nullptr);
        wc.lpszClassName = "FSDummy"; RegisterClassA(&wc);
        HWND w = CreateWindowA("FSDummy", "", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, 0, 0, wc.hInstance, 0);
        IDirect3D9* d3d = Direct3DCreate9(D3D_SDK_VERSION);
        if (!d3d) { FSLog("Direct3DCreate9 falhou"); return 0; }
        D3DPRESENT_PARAMETERS pp{}; pp.Windowed = TRUE; pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        pp.hDeviceWindow = w; pp.BackBufferFormat = D3DFMT_UNKNOWN;
        IDirect3DDevice9* dev = nullptr;
        if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, w,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING, &pp, &dev))) { FSLog("CreateDevice dummy falhou"); return 0; }
        void** vt = *(void***)dev;
        void* pEnd = vt[42]; void* pReset = vt[16];
        dev->Release(); d3d->Release(); DestroyWindow(w);
        MH_Initialize();
        MH_CreateHook(pEnd, (void*)&hEndScene, (void**)&oEndScene);
        MH_CreateHook(pReset, (void*)&hReset, (void**)&oReset);
        MH_CreateHookApi(L"user32", "SetCursorPos", (void*)&hSetCursorPos, (void**)&oSetCursorPos);
        MH_EnableHook(MH_ALL_HOOKS);
        FSLog("Hooks instalados");
        return 0;
    }
    void Install() { CreateThread(nullptr, 0, InstallThread, nullptr, 0, nullptr); }
}
