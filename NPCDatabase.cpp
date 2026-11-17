#include "NPCDatabase.h"
#include "Log.h"
#include "plugin.h"
#include "CModelInfo.h"
#include <fstream>
#include <sstream>
#include <algorithm>
static std::vector<NPCInfo> g_list;
namespace NPCDatabase {
    static void ParseFile(const char* path) {
        std::ifstream f(path); if (!f) { FSLog("nao achei %s", path); return; }
        bool inPeds = false; std::string line;
        while (std::getline(f, line)) {
            if (line.rfind("peds", 0) == 0) { inPeds = true; continue; }
            if (line.rfind("end", 0) == 0) { inPeds = false; continue; }
            if (!inPeds || line.empty() || line[0] == '#') continue;
            std::replace(line.begin(), line.end(), ',', ' ');
            std::istringstream s(line); NPCInfo n;
            if (!(s >> n.id >> n.model >> n.txd)) continue;
            g_list.push_back(n);
        }
    }
    void Load() {
        g_list.clear();
        ParseFile("data\\ped.ide");
        // remove modelos que nao existem de fato / duplicados
        g_list.erase(std::remove_if(g_list.begin(), g_list.end(), [](const NPCInfo& n) {
            return n.id < 0 || n.id >= 20000 || !CModelInfo::GetModelInfo(n.id); }), g_list.end());
        std::sort(g_list.begin(), g_list.end(), [](auto& a, auto& b) { return a.id < b.id; });
        FSLog("NPCDatabase: %d modelos", (int)g_list.size());
    }
    const std::vector<NPCInfo>& All() { return g_list; }
}
namespace NPCDatabase {
    const NPCInfo* Find(int id) {
        static NPCInfo cj{ 0, "player", "player" };
        if (id == 0) return &cj;
        for (auto& n : g_list) if (n.id == id) return &n;
        return nullptr;
    }
}
