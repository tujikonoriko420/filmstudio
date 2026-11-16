#pragma once
class CPed;
namespace NPCSpawner {
    // Carrega o modelo, cria o ped perto do CJ e libera o modelo. Retorna handle (-1 = falha).
    int Spawn(int modelId);
    void Remove(int handle);
}
