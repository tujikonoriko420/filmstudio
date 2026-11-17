#include "AnimationManager.h"
#include "AnimData.h"
#include "BehaviorManager.h"
#include "Log.h"
#include "plugin.h"
#include "CPools.h"
#include "CStreaming.h"
#include "CAnimManager.h"
#include "CPedIntelligence.h"
#include "CTaskManager.h"
#include "CTaskSimpleRunNamedAnim.h"
#include "CTaskSimpleStandStill.h"
#include <map>
#include <cstring>
static const int IFP_BASE = 25575;          // primeiro "modelo" IFP no streaming (SA 1.0)
static std::map<int, int> g_pedBlock;       // handle do ped -> bloco com ref nossa
static char g_err[96] = "";

namespace AnimationManager {
    const char* LastError() { return g_err; }

    static void ReleaseRef(int handle) {
        auto it = g_pedBlock.find(handle);
        if (it != g_pedBlock.end()) { CAnimManager::RemoveAnimBlockRef(it->second); g_pedBlock.erase(it); }
    }

    bool Play(CPed* ped, int libIndex, int animIdx, bool loop) {
        g_err[0] = 0;
        if (!ped || !ped->m_pIntelligence) { strcpy(g_err, "NPC invalido"); return false; }
        if (libIndex < 0 || libIndex >= kAnimLibTotal || animIdx < 0 || animIdx >= kAnimTotal) { strcpy(g_err, "animacao invalida"); return false; }
        const char* lib = kAnimLibs[libIndex].name;
        const char* name = kAnimNames[animIdx];
        int blk = CAnimManager::GetAnimationBlockIndex(lib);
        if (blk < 0) { snprintf(g_err, sizeof g_err, "bloco %s nao existe", lib); FSLog("%s", g_err); return false; }
        CStreaming::RequestModel(IFP_BASE + blk, GAME_REQUIRED);
        CStreaming::LoadAllRequestedModels(false);
        if (CStreaming::ms_aInfoForModel[IFP_BASE + blk].m_nLoadState != LOADSTATE_LOADED) {
            snprintf(g_err, sizeof g_err, "bloco %s nao carregou", lib); FSLog("%s", g_err); return false; }
        int h = CPools::GetPedRef(ped);
        ReleaseRef(h);
        CAnimManager::AddAnimBlockRef(blk);
        g_pedBlock[h] = blk;
        // flags: 0x1 STARTED | 0x2 LOOPED | 0x20 FREEZE_TRANSLATION (impede deslocamento proprio da animacao)
        int flags = 0x1 | 0x20 | (loop ? 0x2 : 0);
        auto* task = new CTaskSimpleRunNamedAnim(name, lib, flags, 4.0f, -1, false, false, false, false);
        ped->m_pIntelligence->m_TaskMgr.SetTask(task, TASK_PRIMARY_PRIMARY, false);
        return true;
    }

    void Stop(CPed* ped) {
        if (!ped || !ped->m_pIntelligence) return;
        int h = CPools::GetPedRef(ped);
        // limpa a tarefa; BehaviorManager recoloca StandStill/Wander conforme o modo
        ped->m_pIntelligence->m_TaskMgr.SetTask(nullptr, TASK_PRIMARY_PRIMARY, false);
        if (BehaviorManager::GetMode(ped) == NPCMode::Static)
            ped->m_pIntelligence->m_TaskMgr.SetTask(new CTaskSimpleStandStill(-1, true, false, 8.0f), TASK_PRIMARY_PRIMARY, false);
        ReleaseRef(h);
    }
}
