#pragma once
namespace CameraManager {
    bool Active();
    void Toggle();
    void Disable();                 // restaura a camera original do jogo
    void Update(bool menuOpen, bool textInput);   // todo frame (processScriptsEvent)
    struct Params { float pos[3]; float yawDeg, pitchDeg, fov, speed, sens; bool hideHud; };
    Params& P();
    void ApplyNumeric();            // chamado ao editar pos/rotacao pelos campos
}
