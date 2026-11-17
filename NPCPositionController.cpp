#include "NPCPositionController.h"
#include "BehaviorManager.h"
#include "NPCSelector.h"
#include "plugin.h"
#include "CPed.h"
#include <cmath>
static float g_dragZ = 0.0f; static bool g_wasDragging = false;
namespace NPCPositionController {
    void SetPosition(CPed* p, float x, float y, float z) {
        if (!p) return;
        p->SetPosn(x, y, z);
        p->m_vecMoveSpeed = CVector(0, 0, 0);
        BehaviorManager::SetAnchor(p);
    }
    void SetHeadingDeg(CPed* p, float deg) {
        if (!p) return;
        float r = deg * 3.14159265f / 180.0f;
        p->m_fCurrentRotation = r; p->m_fAimingRotation = r;
        p->SetHeading(r);
    }
    float GetHeadingDeg(CPed* p) { return p ? p->m_fCurrentRotation * 180.0f / 3.14159265f : 0.0f; }

    // Newton 2D: acha (x,y) no plano z tal que a projecao em tela coincide com o mouse
    static bool ScreenToPlane(float mx, float my, float z, CVector start, CVector& out) {
        CVector p(start.x, start.y, z);
        for (int it = 0; it < 14; it++) {
            float sx, sy, ax, ay, bx, by;
            if (!NPCSelector::WorldToScreen(p, sx, sy)) return false;
            float ex = mx - sx, ey = my - sy;
            if (ex * ex + ey * ey < 0.25f) break;
            if (!NPCSelector::WorldToScreen(CVector(p.x + 1, p.y, z), ax, ay)) return false;
            if (!NPCSelector::WorldToScreen(CVector(p.x, p.y + 1, z), bx, by)) return false;
            float a = ax - sx, b = bx - sx, c = ay - sy, d = by - sy, det = a * d - b * c;
            if (fabsf(det) < 1e-4f) return false;
            float dx = (d * ex - b * ey) / det, dy = (-c * ex + a * ey) / det;
            float m = sqrtf(dx * dx + dy * dy); if (m > 20.0f) { dx *= 20.0f / m; dy *= 20.0f / m; }
            p.x += dx; p.y += dy;
        }
        out = p; return true;
    }
    void UpdateMouseDrag(CPed* ped, bool active, float mx, float my, bool dragging, float wheel) {
        if (!ped || !active) { g_wasDragging = false; return; }
        CVector pos = ped->GetPosition();
        if (!g_wasDragging && dragging) g_dragZ = pos.z;
        if (dragging && wheel != 0.0f) g_dragZ += wheel * 0.1f;   // altura
        g_wasDragging = dragging;
        if (!dragging) return;
        CVector out;
        if (ScreenToPlane(mx, my, g_dragZ, pos, out)) SetPosition(ped, out.x, out.y, g_dragZ);
    }
}
