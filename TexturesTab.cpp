#include "TexturesTab.h"
#include "TextureManager.h"
#include "NPCSelector.h"
#include "plugin.h"
#include "CPed.h"
#include "imgui.h"
static int g_target = 0;   // 0 = NPC selecionado, 1 = CJ
static bool g_overwrite = false;
namespace TexturesTab {
    void Draw() {
        ImGui::RadioButton("NPC selecionado", &g_target, 0); ImGui::SameLine();
        ImGui::RadioButton("CJ", &g_target, 1);
        CPed* ped = g_target == 1 ? FindPlayerPed(-1) : NPCSelector::Selected();
        TextureManager::Target t;
        if (!ped) { ImGui::TextDisabled("Selecione um NPC na aba NPC Manager."); return; }
        if (!TextureManager::Resolve(ped, t)) { ImGui::TextColored(ImVec4(1, .4f, .3f, 1), "%s", TextureManager::Status()); return; }
        ImGui::Text("Modelo: %s (id %d)   TXD: %s.txd", t.model.c_str(), t.modelId, t.txd.c_str());
        ImGui::TextDisabled("Pasta: %s", TextureManager::Folder(t.txd).c_str());
        if (g_target == 1)
            ImGui::TextWrapped("CJ: aqui editamos as texturas do modelo base. Roupas (player.img) sao montadas em tempo real pelo jogo e ainda nao sao suportadas.");

        ImGui::BeginChild("texlist", ImVec2(0, 150), true);
        for (auto& n : TextureManager::ListTextures(t.txd)) ImGui::BulletText("%s", n.c_str());
        ImGui::EndChild();

        if (ImGui::Button("Exportar PNG")) TextureManager::Export(t.txd, g_overwrite);
        ImGui::SameLine(); ImGui::Checkbox("sobrescrever existentes", &g_overwrite);
        if (ImGui::Button("Open Texture Folder")) TextureManager::OpenFolder(t.txd);
        ImGui::SameLine(); if (ImGui::Button("Reload Texture")) TextureManager::Reload(t.modelId, t.txd);
        ImGui::SameLine(); if (ImGui::Button("Restaurar originais")) TextureManager::Restore();
        if (TextureManager::Status()[0]) ImGui::TextWrapped("%s", TextureManager::Status());
        ImGui::TextDisabled("Texturas compartilhadas mudam em todos os NPCs que usam o mesmo TXD.");
    }
}
