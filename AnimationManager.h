#pragma once
class CPed;
namespace AnimationManager {
    // Carrega o bloco IFP (se preciso), toca a animacao no ped. false se algo nao estiver disponivel.
    bool Play(CPed* ped, int libIndex, int animGlobalIndex, bool loop);
    void Stop(CPed* ped);
    const char* LastError();
}
