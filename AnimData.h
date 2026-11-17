#pragma once
// Gerado a partir do AnimSelector (anim.img + ped.ifp do jogo)
struct AnimLib { const char* name; int first; int count; };
extern const int kAnimLibTotal;
extern const int kAnimTotal;
extern const AnimLib kAnimLibs[];
extern const char* const kAnimNames[];
