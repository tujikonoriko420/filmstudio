#pragma once
class CPed;
namespace NPCPositionController {
    void SetPosition(CPed* ped, float x, float y, float z);
    void SetHeadingDeg(CPed* ped, float deg);
    float GetHeadingDeg(CPed* ped);
    // Movimento pelo mouse: chamar todo frame com o ped selecionado.
    // Segura o botao esquerdo fora do menu para arrastar; roda do mouse altera a altura.
    void UpdateMouseDrag(CPed* ped, bool active, float mx, float my, bool dragging, float wheel);
}
