#pragma once
#include <set>
#include <vector>
class CPed;
#include "CVector.h"
namespace NPCSelector {
    bool WorldToScreen(const CVector& w, float& sx, float& sy);
    CPed* Selected();                    // nullptr se nada / invalido
    void Select(CPed* p);
    void Clear();
    void MarkSpawned(int handle);
    bool IsSpawned(CPed* p);
    void PickAt(float mx, float my);     // coordenadas em pixels da janela
    void DrawHighlight();                // ImGui: marcador sobre o NPC selecionado
    std::vector<CPed*> NearbyPeds(float maxDist);
}
