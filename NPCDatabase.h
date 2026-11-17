#pragma once
#include <string>
#include <vector>
struct NPCInfo { int id; std::string model; std::string txd; };
namespace NPCDatabase {
    void Load();                              // le data\ped.ide + verifica CModelInfo
    const std::vector<NPCInfo>& All();
}
namespace NPCDatabase { const NPCInfo* Find(int modelId); }  // CJ (id 0) = "player"
