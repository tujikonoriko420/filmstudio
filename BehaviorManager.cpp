#include "BehaviorManager.h"
#include "Log.h"
#include "plugin.h"
#include "CPools.h"
#include "CPed.h"
#include "CPedIntelligence.h"
#include "CTaskManager.h"
#include "CTaskSimpleStandStill.h"
#include "CTaskComplexWanderStandard.h"
#include <map>
#include <cstdlib>
#include <cmath>
struct State { NPCMode mode; CVector anchor; };
static std::map<int, State> g_state;

static CTask* PrimaryTask(CPed* p) {
    if (!p->m_pIntelligence) return nullptr;
    return p->m_pIntelligence->m_TaskMgr.m_aPrimaryTasks[TASK_PRIMARY_PRIMARY];
}
static void SetPrimary(CPed* p, CTask* t) {
    if (p->m_pIntelligence) p->m_pIntelligence->m_TaskMgr.SetTask(t, TASK_PRIMARY_PRIMARY, false);
}
namespace BehaviorManager {
    void SetAnchor(CPed* ped) {
        auto it = g_state.find(CPools::GetPedRef(ped));
        if (it != g_state.end()) it->second.anchor = ped->GetPosition();
    }
    void SetMode(CPed* ped, NPCMode m) {
        if (!ped || !ped->m_pIntelligence) return;
        int h = CPools::GetPedRef(ped);
        State& s = g_state[h]; s.mode = m; s.anchor = ped->GetPosition();
        if (m == NPCMode::Static) {
            SetPrimary(ped, new CTaskSimpleStandStill(-1, true, false, 8.0f));
        } else if (m == NPCMode::Walking) {
            SetPrimary(ped, new CTaskComplexWanderStandard(PEDMOVE_WALK, (unsigned char)(rand() % 8), true));
        } else { // Normal: devolve ao comportamento padrao do jogo
            SetPrimary(ped, nullptr);
            ped->m_pIntelligence->m_TaskMgr.SetTask(
                new CTaskComplexWanderStandard(PEDMOVE_WALK, (unsigned char)(rand() % 8), true), TASK_PRIMARY_DEFAULT, false);
            g_state.erase(h); // nao controlamos mais este ped
        }
    }
    NPCMode GetMode(CPed* ped) {
        auto it = g_state.find(CPools::GetPedRef(ped));
        return it == g_state.end() ? NPCMode::Normal : it->second.mode;
    }
    void Forget(int h) { g_state.erase(h); }
    void Update() {
        for (auto it = g_state.begin(); it != g_state.end();) {
            CPed* p = CPools::GetPed(it->first);
            if (!p) { it = g_state.erase(it); continue; } // ped removido: sem referencia invalida
            State& s = it->second;
            if (s.mode == NPCMode::Static) {
                // Nao congelamos a entidade (a animacao precisa rodar). Apenas impedimos deslocamento.
                CVector pos = p->GetPosition();
                float dx = pos.x - s.anchor.x, dy = pos.y - s.anchor.y;
                if (dx * dx + dy * dy > 0.0004f) p->SetPosn(s.anchor.x, s.anchor.y, pos.z);
                p->m_vecMoveSpeed.x = 0.0f; p->m_vecMoveSpeed.y = 0.0f;
                if (!PrimaryTask(p)) SetPrimary(p, new CTaskSimpleStandStill(-1, true, false, 8.0f));
            } else if (s.mode == NPCMode::Walking) {
                if (!PrimaryTask(p))
                    SetPrimary(p, new CTaskComplexWanderStandard(PEDMOVE_WALK, (unsigned char)(rand() % 8), true));
            }
            ++it;
        }
    }
}
