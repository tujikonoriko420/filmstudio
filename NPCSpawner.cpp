#include "NPCSpawner.h"
#include "Log.h"
#include "plugin.h"
#include "CStreaming.h"
#include "CModelInfo.h"
#include "CWorld.h"
#include "CPools.h"
#include "CPopulation.h"
#include "CCivilianPed.h"
#include "CTimer.h"
namespace NPCSpawner {
    int Spawn(int modelId) {
        CPed* player = FindPlayerPed(-1);
        if (!player) return -1;
        if (modelId < 0 || !CModelInfo::GetModelInfo(modelId)) { FSLog("modelo %d invalido", modelId); return -1; }
        CStreaming::RequestModel(modelId, GAME_REQUIRED);
        CStreaming::LoadAllRequestedModels(false);
        if (CStreaming::ms_aInfoForModel[modelId].m_nLoadState != LOADSTATE_LOADED) {
            FSLog("modelo %d nao carregou", modelId); return -1; }
        // 2m a frente do CJ, evitando sobreposicao
        CVector pos = player->TransformFromObjectSpace(CVector(0.0f, 2.0f, 0.0f));
        ePedType type = CPopulation::IsFemale(modelId) ? PED_TYPE_CIVFEMALE : PED_TYPE_CIVMALE;
        CPed* ped = new CCivilianPed(type, modelId);
        if (!ped) return -1;
        ped->SetPosn(pos);
        ped->SetOrientation(0.0f, 0.0f, 0.0f);
        ped->m_fCurrentRotation = ped->m_fAimingRotation = player->GetHeading() + 3.14159f;
        ped->m_nCreatedBy = 2; // PED_MISSION: nao e removido pelo gerenciador de populacao
        CWorld::Add(ped);
        ped->PositionAnyPedOutOfCollision();
        CStreaming::SetModelIsDeletable(modelId);  // libera o recurso temporario
        return CPools::GetPedRef(ped);
    }
    void Remove(int handle) {
        CPed* p = CPools::GetPed(handle);
        if (!p || p == FindPlayerPed(-1)) return;
        CWorld::Remove(p);
        delete p;
    }
}
