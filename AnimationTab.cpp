#include "AnimationTab.h"
#include "AnimData.h"
#include "AnimationManager.h"
#include "NPCSelector.h"
#include "imgui.h"
#include <string>
#include <algorithm>
#include <cctype>
static int g_libSel = -1, g_animSel = -1; static bool g_loop = true; static char g_filter[64] = "";
static std::string Lower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; }
namespace AnimationTab {
    void Draw() {
        CPed* ped = NPCSelector::Selected();
        if (!ped) ImGui::TextDisabled("Selecione um NPC na aba NPC Manager.");
        ImGui::InputTextWithHint("##af", "Pesquisar animacao ou arquivo (ex: dance, sit, PED)...", g_filter, sizeof g_filter);
        std::string f = Lower(g_filter);
        ImGui::BeginChild("anims", ImVec2(0, 260), true);
        for (int l = 0; l < kAnimLibTotal; l++) {
            for (int a = 0; a < kAnimLibs[l].count; a++) {
                int gi = kAnimLibs[l].first + a;
                if (!f.empty() && Lower(kAnimNames[gi]).find(f) == std::string::npos &&
                    Lower(kAnimLibs[l].name).find(f) == std::string::npos) continue;
                char lbl[96]; snprintf(lbl, sizeof lbl, "%s / %s", kAnimLibs[l].name, kAnimNames[gi]);
                if (ImGui::Selectable(lbl, g_animSel == gi)) { g_animSel = gi; g_libSel = l; }
            }
        }
        ImGui::EndChild();
        ImGui::Checkbox("Loop", &g_loop);
        ImGui::BeginDisabled(!ped || g_animSel < 0);
        if (ImGui::Button("Reproduzir")) AnimationManager::Play(ped, g_libSel, g_animSel, g_loop);
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(!ped);
        if (ImGui::Button("Parar")) AnimationManager::Stop(ped);
        ImGui::EndDisabled();
        if (AnimationManager::LastError()[0]) ImGui::TextColored(ImVec4(1, 0.4f, 0.3f, 1), "%s", AnimationManager::LastError());
    }
}
