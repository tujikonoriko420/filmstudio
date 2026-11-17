#include "CameraTab.h"
#include "CameraManager.h"
#include "Settings.h"
#include "imgui.h"
namespace CameraTab {
    void Draw() {
        auto& p = CameraManager::P();
        bool on = CameraManager::Active();
        if (ImGui::Button(on ? "Desativar camera livre" : "Ativar camera livre")) CameraManager::Toggle();
        ImGui::SameLine(); ImGui::TextDisabled("(atalho: tecla definida em Settings)");
        ImGui::TextWrapped("Com o menu FECHADO: WASD move, Q/E desce/sobe, Shift acelera, mouse gira. "
                           "O CJ e os NPCs nao reagem aos controles.");
        ImGui::Separator();
        bool ch = false;
        ch |= ImGui::InputFloat3("X Y Z", p.pos, "%.2f");
        ch |= ImGui::SliderFloat("Yaw", &p.yawDeg, -360, 360, "%.1f");
        ch |= ImGui::SliderFloat("Pitch", &p.pitchDeg, -89, 89, "%.1f");
        ch |= ImGui::SliderFloat("FOV", &p.fov, 10, 120, "%.0f");
        ImGui::SliderFloat("Velocidade", &p.speed, 0.5f, 40.0f, "%.1f m/s");
        ImGui::SliderFloat("Sensibilidade do mouse", &p.sens, 0.02f, 0.6f, "%.2f");
        ch |= ImGui::Checkbox("Esconder HUD", &p.hideHud);
        if (ch) CameraManager::ApplyNumeric();
    }
}
