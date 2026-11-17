#include "CameraManager.h"
#include "Log.h"
#include "plugin.h"
#include "CCamera.h"
#include "CPad.h"
#include "CHud.h"
#include "CTimer.h"
#include <windows.h>
#include <cmath>
static bool g_active = false;
static CameraManager::Params g_p = { {0,0,0}, 0, 0, 70.0f, 6.0f, 0.15f, false };
static const float DEG = 3.14159265f / 180.0f;
namespace CameraManager {
    Params& P() { return g_p; }
    bool Active() { return g_active; }

    static void Apply() {
        float y = g_p.yawDeg * DEG, p = g_p.pitchDeg * DEG;
        CVector pos(g_p.pos[0], g_p.pos[1], g_p.pos[2]);
        CVector dir(-sinf(y) * cosf(p), cosf(y) * cosf(p), sinf(p));
        CVector target = pos + dir * 10.0f;
        // mesma sequencia dos opcodes 015F (fixed camera) e 0160 (point at) usados pelo Simple Free Cam
        TheCamera.SetCamPositionForFixedMode(pos, CVector(0, 0, 0));
        TheCamera.TakeControlNoEntity(target, 2 /*jump cut*/, 1);
        TheCamera.m_aCams[TheCamera.m_nActiveCam].m_fFOV = g_p.fov;
        CHud::m_Wants_To_Draw_Hud = !g_p.hideHud;
    }
    void ApplyNumeric() { if (g_active) Apply(); }

    void Toggle() {
        if (g_active) { Disable(); return; }
        CVector c = TheCamera.GetPosition(), f = TheCamera.GetForward();   // comeca de onde a camera esta
        g_p.pos[0] = c.x; g_p.pos[1] = c.y; g_p.pos[2] = c.z;
        g_p.yawDeg = atan2f(-f.x, f.y) / DEG;
        g_p.pitchDeg = asinf(f.z) / DEG;
        g_active = true; FSLog("camera livre ligada");
        Apply();
    }
    void Disable() {
        if (!g_active) return;
        g_active = false;
        CHud::m_Wants_To_Draw_Hud = true;
        TheCamera.RestoreWithJumpCut();   // opcode 015A: volta a camera normal
        FSLog("camera livre desligada");
    }
    void Update(bool menuOpen, bool textInput) {
        if (!g_active) return;
        CPad* pad = CPad::GetPad(0);
        if (!menuOpen && pad) {  // rotacao pelo mouse (delta do DirectInput do proprio jogo)
            g_p.yawDeg -= pad->NewMouseControllerState.X * g_p.sens;
            g_p.pitchDeg -= pad->NewMouseControllerState.Y * g_p.sens;
            if (g_p.pitchDeg > 89) g_p.pitchDeg = 89; if (g_p.pitchDeg < -89) g_p.pitchDeg = -89;
        }
        if (!textInput && GetForegroundWindow() == GetActiveWindow()) {
            float dt = CTimer::ms_fTimeStep / 50.0f;             // ~segundos
            float sp = g_p.speed * dt * ((GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 4.0f : 1.0f);
            float y = g_p.yawDeg * DEG, p = g_p.pitchDeg * DEG;
            float fx = -sinf(y) * cosf(p), fy = cosf(y) * cosf(p), fz = sinf(p), rx = cosf(y), ry = sinf(y);
            auto K = [](int vk) { return (GetAsyncKeyState(vk) & 0x8000) != 0; };
            if (K('W')) { g_p.pos[0] += fx * sp; g_p.pos[1] += fy * sp; g_p.pos[2] += fz * sp; }
            if (K('S')) { g_p.pos[0] -= fx * sp; g_p.pos[1] -= fy * sp; g_p.pos[2] -= fz * sp; }
            if (K('D')) { g_p.pos[0] += rx * sp; g_p.pos[1] += ry * sp; }
            if (K('A')) { g_p.pos[0] -= rx * sp; g_p.pos[1] -= ry * sp; }
            if (K('E')) g_p.pos[2] += sp;
            if (K('Q')) g_p.pos[2] -= sp;
        }
        Apply();
    }
}
