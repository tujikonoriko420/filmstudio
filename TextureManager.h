#pragma once
#include <string>
#include <vector>
class CPed;
namespace TextureManager {
    struct Target { int modelId = -1; std::string model, txd; };
    bool Resolve(CPed* ped, Target& t);                       // modelo -> nome do TXD
    std::vector<std::string> ListTextures(const std::string& txd);  // nomes dentro do TXD original
    int  Export(const std::string& txd, bool overwrite);      // grava PNGs em FilmStudio\Textures\<txd>\ ; -1 = erro
    void OpenFolder(const std::string& txd);                  // abre SO o Explorer na pasta
    int  Reload(int modelId, const std::string& txd);         // aplica os PNGs da pasta no jogo ; -1 = erro
    void Restore();                                           // devolve as texturas originais em memoria
    std::string Folder(const std::string& txd);
    const char* Status();
}
