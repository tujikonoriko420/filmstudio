#include "PositionTab.h"
#include "NPCSelector.h"
#include "NPCPositionController.h"
#include "BehaviorManager.h"
#include "plugin.h"
#include "CPed.h"
#include "imgui.h"
namespace PositionTab {
    static bool g_mouseMove = false;
    bool MouseMoveActive() { return g_mouseMove; }
    void Draw() {
        CPed* ped = NPCSelector::Selected();
        if (!ped) { ImGui::TextDisabled("Selecione um NPC na aba NPC Manager."); g_mouseMove = false; return; }
        CVector pos = ped->GetPosition();
        float v[3] = { pos.x, pos.y, pos.z };
        ImGui::TextUnformatted("Coordenadas (digite o valor ou arraste)");
        if (ImGui::InputFloat3("X Y Z", v, "%.2f", ImGuiInputTextFlags_EnterReturnsTrue) ||
            ImGui::IsItemDeactivatedAfterEdit())
            NPCPositionController::SetPosition(ped, v[0], v[1], v[2]);
        const char* ax[3] = { "X", "Y", "Z" };
        for (int i = 0; i < 3; i++) {
            ImGui::PushID(i);
            if (ImGui::Button("-")) { v[i] -= 0.1f; NPCPositionController::SetPosition(ped, v[0], v[1], v[2]); }
            ImGui::SameLine();
            if (ImGui::Button("+")) { v[i] += 0.1f; NPCPositionController::SetPosition(ped, v[0], v[1], v[2]); }
            ImGui::SameLine(); ImGui::Text("%s  (passo 0.1)", ax[i]);
            ImGui::PopID();
        }
        float rot = NPCPositionController::GetHeadingDeg(ped);
        if (ImGui::SliderFloat("Rotacao", &rot, -360.0f, 360.0f, "%.1f graus"))
            NPCPositionController::SetHeadingDeg(ped, rot);

        ImGui::Separator();
        ImGui::Checkbox("Mover com o mouse", &g_mouseMove);
        ImGui::TextDisabled("Segure o botao esquerdo no cenario para arrastar. Roda do mouse = altura.");

        ImGui::Separator();
        ImGui::TextUnformatted("Comportamento");
        int m = (int)BehaviorManager::GetMode(ped);
        bool ch = false;
        ch |= ImGui::RadioButton("Static", &m, 0); ImGui::SameLine();
        ch |= ImGui::RadioButton("Walking", &m, 1); ImGui::SameLine();
        ch |= ImGui::RadioButton("Normal", &m, 2);
        if (ch) BehaviorManager::SetMode(ped, (NPCMode)m);
    }
}
