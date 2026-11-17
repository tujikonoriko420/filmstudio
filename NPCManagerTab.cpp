#include "NPCManagerTab.h"
#include "NPCDatabase.h"
#include "NPCSpawner.h"
#include "NPCSelector.h"
#include "BehaviorManager.h"
#include "plugin.h"
#include "CPools.h"
#include "imgui.h"
#include <cstring>
#include <string>
#include <algorithm>
static int g_modelSel = -1;
static char g_filter[64] = "";

static bool Match(const NPCInfo& n, const char* f) {
    if (!*f) return true;
    std::string hay = n.model + " " + std::to_string(n.id), nd = f;
    std::transform(hay.begin(), hay.end(), hay.begin(), ::tolower);
    std::transform(nd.begin(), nd.end(), nd.begin(), ::tolower);
    return hay.find(nd) != std::string::npos;
}
namespace NPCManagerTab {
    void Draw() {
        ImGui::TextUnformatted("Spawn");
        ImGui::InputTextWithHint("##f", "Pesquisar nome ou ID...", g_filter, sizeof g_filter);
        ImGui::BeginChild("models", ImVec2(0, 190), true);
        for (auto& n : NPCDatabase::All()) {
            if (!Match(n, g_filter)) continue;
            char lbl[96]; snprintf(lbl, sizeof lbl, "%3d  %s", n.id, n.model.c_str());
            if (ImGui::Selectable(lbl, g_modelSel == n.id)) g_modelSel = n.id;
        }
        ImGui::EndChild();
        ImGui::BeginDisabled(g_modelSel < 0);
        if (ImGui::Button("Spawn")) {
            int h = NPCSpawner::Spawn(g_modelSel);
            if (h >= 0) { NPCSelector::MarkSpawned(h); NPCSelector::Select(CPools::GetPed(h));
                BehaviorManager::SetMode(CPools::GetPed(h), NPCMode::Static); // NPC criado fica parado ate voce mudar }
        }
        ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::TextUnformatted("NPCs no mundo (ate 60m)  |  ou clique no personagem na tela");
        ImGui::BeginChild("world", ImVec2(0, 150), true);
        CPed* sel = NPCSelector::Selected();
        for (CPed* p : NPCSelector::NearbyPeds(60.0f)) {
            char lbl[96]; snprintf(lbl, sizeof lbl, "%s model %d  #%d",
                NPCSelector::IsSpawned(p) ? "[meu]" : "[jogo]", (int)p->m_nModelIndex, CPools::GetPedRef(p));
            if (ImGui::Selectable(lbl, p == sel)) NPCSelector::Select(p);
        }
        ImGui::EndChild();

        if (sel) {
            ImGui::Separator();
            CVector pos = sel->GetPosition();
            ImGui::Text("Selecionado: modelo %d (%s)", (int)sel->m_nModelIndex,
                NPCSelector::IsSpawned(sel) ? "criado por voce" : "NPC do jogo");
            ImGui::Text("Pos: %.2f  %.2f  %.2f", pos.x, pos.y, pos.z);
            if (ImGui::Button("Desselecionar")) NPCSelector::Clear();
            if (NPCSelector::IsSpawned(sel)) {
                ImGui::SameLine();
                if (ImGui::Button("Remover")) { NPCSpawner::Remove(CPools::GetPedRef(sel)); NPCSelector::Clear(); }
            }
        }
    }
}
