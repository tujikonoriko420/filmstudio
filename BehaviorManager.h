#pragma once
class CPed; class CVector;
enum class NPCMode { Static = 0, Walking = 1, Normal = 2 };
namespace BehaviorManager {
    void SetMode(CPed* ped, NPCMode m);   // aplica o modo (e guarda a ancora para Static)
    NPCMode GetMode(CPed* ped);           // Normal se o ped nao foi registrado
    void SetAnchor(CPed* ped);            // reancora na posicao atual (Static)
    void Update();                        // chamar todo frame (processScriptsEvent)
    void Forget(int handle);
}
