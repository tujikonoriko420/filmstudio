#pragma once
#include <string>
#include <vector>
#include <cstdint>
struct TxdTexture { std::string name; int width = 0, height = 0; std::vector<uint8_t> rgba; };
namespace TxdFile {
    // Decodifica o mip 0 de cada textura de um TXD (D3D8/D3D9: DXT1/3/5, 8888, 888, 565, 1555, 4444, PAL8).
    bool Parse(const std::vector<uint8_t>& data, std::vector<TxdTexture>& out, std::string& err);
}
