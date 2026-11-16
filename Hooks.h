#pragma once
#include <functional>
namespace Hooks {
    void Install();                        // cria thread que instala os hooks D3D9
    extern std::function<void()> onFrame;  // chamado dentro do EndScene (contexto ImGui)
    extern bool menuOpen;                  // controla captura de input
}
