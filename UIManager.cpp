#include "UIManager.h"
#include "NPCManagerTab.h"
#include "Hooks.h"
#include "Settings.h"
#include "NPCSelector.h"
#include "plugin.h"
#include "CPad.h"
#include "imgui.h"
#include <windows.h>
#include <cstring>

static bool g_open = false;
static bool Pressed(int vk) { // borda de subida, so quando a janela do jogo tem foco
    static bool prev[256]; bool now = (GetAsyncKeyState(vk) & 0x8000) != 0 && GetForegroundWindow() == GetActiveWindow();
    bool r = now && !prev[vk & 255]; prev[vk & 255] = now; return r;
}
static void Placeholder(const char* stage) { ImGui::TextDisabled("Em desenvolvimento (%s).", stage); }

static int g_rebind = 0; // 1=menu 2=cam
static void SettingsTab() {
    ImGui::Text("Tecla do menu: VK 0x%02X", Settings::menuKey);
    ImGui::SameLine(); if (ImGui::Button(g_rebind == 1 ? "Pressione uma tecla..." : "Alterar##m")) g_rebind = 1;
    ImGui::Text("Tecla camera livre: VK 0x%02X", Settings::camKey);
    ImGui::SameLine(); if (ImGui::Button(g_rebind == 2 ? "Pressione uma tecla..." : "Alterar##c")) g_rebind = 2;
    if (g_rebind) {
        for (int vk = 8; vk < 255; vk++) {
            if (vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_MBUTTON) continue;
            if (GetAsyncKeyState(vk) & 0x8000) {
                if (vk != VK_ESCAPE) (g_rebind == 1 ? Settings::menuKey : Settings::camKey) = vk;
                g_rebind = 0; Settings::Save(); break;
            }
        }
    }
    ImGui::TextDisabled("Salvo em FilmStudio.ini");
}

namespace UIManager {
    void SetOpen(bool o) { g_open = o; Hooks::menuOpen = o; }

    void Init() {
        Hooks::onFrame = Draw;
        // Enquanto o menu esta aberto, zera os controles do GTA (jogador e mouse-look) apos o update dos pads.
        plugin::Events::processScriptsEvent += [] {
            if (!g_open) return;
            CPad* pad = CPad::GetPad(0);
            if (pad) { memset(&pad->NewState, 0, sizeof pad->NewState);
                       memset(&pad->NewMouseControllerState, 0, sizeof pad->NewMouseControllerState); }
        };
    }

    void Draw() {
        if (g_rebind == 0 && Pressed(Settings::menuKey)) SetOpen(!g_open);
        NPCSelector::DrawHighlight();
        if (!g_open) return;
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowSize(ImVec2(430, 520), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowBgAlpha(0.88f);
        if (ImGui::Begin("GTA SA Film Studio", nullptr)) {
            if (ImGui::BeginTabBar("tabs")) {
                if (ImGui::BeginTabItem("NPC Manager")) { NPCManagerTab::Draw(); ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("Animation"))   { Placeholder("Etapa 4"); ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("Position"))    { Placeholder("Etapa 3"); ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("Textures"))    { Placeholder("Etapa 5"); ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("Camera"))      { Placeholder("Etapa 6"); ImGui::EndTabItem(); }
                if (ImGui::BeginTabItem("Settings"))    { SettingsTab(); ImGui::EndTabItem(); }
                ImGui::EndTabBar();
            }
        }
        bool overUI = io.WantCaptureMouse;
        ImGui::End();
        // clique no mundo (fora das janelas) seleciona NPC
        if (!overUI && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered() && !io.WantCaptureMouse)
            NPCSelector::PickAt(io.MousePos.x, io.MousePos.y);
    }
}
