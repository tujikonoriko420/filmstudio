#pragma once
#include <string>
#include <vector>
#include <cstdint>
namespace ImgArchive {
    // Le um arquivo de dentro de um .img (formato VER2 do SA). Nome sem diferenciar maiusculas.
    bool Read(const std::string& imgPath, const std::string& fileName, std::vector<uint8_t>& out);
    // Procura "fileName" em modloader\ (recursivo), depois em models\gta3.img e models\player.img.
    bool FindGameFile(const std::string& fileName, std::vector<uint8_t>& out, std::string* source = nullptr);
}
