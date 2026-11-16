#pragma once
namespace UIManager {
    void Init();          // registra callbacks do jogo e do ImGui
    void Draw();          // chamado a cada frame (EndScene)
    void SetOpen(bool);
}
