#include "NPCSelector.h"
#include "plugin.h"
#include "CPools.h"
#include "CSprite.h"
#include "CWorld.h"
#include "imgui.h"
#include <cmath>
static int g_sel = -1;
static std::set<int> g_spawned;
namespace NPCSelector {
    CPed* Selected() {
        if (g_sel < 0) return nullptr;
        CPed* p = CPools::GetPed(g_sel);   // nullptr se foi removido (handle invalido)
        if (!p) g_sel = -1;
        return p;
    }
    void Select(CPed* p) { g_sel = p ? CPools::GetPedRef(p) : -1; }
    void Clear() { g_sel = -1; }
    void MarkSpawned(int h) { g_spawned.insert(h); }
    bool IsSpawned(CPed* p) {
        int h = CPools::GetPedRef(p);
        if (g_spawned.count(h) && !CPools::GetPed(h)) g_spawned.erase(h);
        return g_spawned.count(h) > 0;
    }
    bool WorldToScreen(const CVector& w, float& sx, float& sy) {
        RwV3d in = { w.x, w.y, w.z }, out; float fw, fh;
        if (!CSprite::CalcScreenCoords(in, &out, &fw, &fh, true, true)) return false;
        ImVec2 ds = ImGui::GetIO().DisplaySize;
        sx = out.x * ds.x / (float)RsGlobal.maximumWidth;
        sy = out.y * ds.y / (float)RsGlobal.maximumHeight;
        return true;
    }
    std::vector<CPed*> NearbyPeds(float maxDist) {
        std::vector<CPed*> v; CPed* pl = FindPlayerPed(-1); if (!pl) return v;
        auto pool = CPools::ms_pPedPool;
        for (int i = 0; i < pool->m_nSize; i++) {
            CPed* p = pool->GetAt(i);
            if (!p || p == pl) continue;
            if ((p->GetPosition() - pl->GetPosition()).Magnitude() <= maxDist) v.push_back(p);
        }
        return v;
    }
    void PickAt(float mx, float my) {
        CPed* best = nullptr; float bd = 60.0f * 60.0f; // raio de clique (px)
        CPed* pl = FindPlayerPed(-1);
        auto pool = CPools::ms_pPedPool;
        for (int i = 0; i < pool->m_nSize; i++) {
            CPed* p = pool->GetAt(i);
            if (!p || p == pl) continue;
            CVector c = p->GetPosition(); c.z += 0.2f; // centro do corpo
            float sx, sy; if (!WorldToScreen(c, sx, sy)) continue;
            float d = (sx - mx) * (sx - mx) + (sy - my) * (sy - my);
            if (d < bd) { bd = d; best = p; }
        }
        if (best) Select(best);
    }
    void DrawHighlight() {
        CPed* p = Selected(); if (!p) return;
        CVector c = p->GetPosition(); float sx, sy, tx, ty;
        CVector top = c; top.z += 1.0f; CVector bot = c; bot.z -= 1.0f;
        if (!WorldToScreen(top, tx, ty) || !WorldToScreen(bot, sx, sy)) return;
        float h = fabsf(sy - ty), w = h * 0.45f, cx = (sx + tx) * 0.5f;
        ImGui::GetBackgroundDrawList()->AddRect(ImVec2(cx - w, ty), ImVec2(cx + w, sy),
            IM_COL32(255, 220, 0, 255), 4.0f, 0, 2.5f);
    }
}
