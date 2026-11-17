#include "ImgArchive.h"
#include "Log.h"
#include <fstream>
#include <cstring>
#include <iterator>
#include <map>
#include <filesystem>
#include <algorithm>
namespace fs = std::filesystem;
struct ImgEntry { uint32_t offset, size; };
struct ImgDir { std::map<std::string, ImgEntry> files; bool ok = false; };
static std::map<std::string, ImgDir> g_dirs;
static std::string Lower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; }

static ImgDir& GetDir(const std::string& path) {
    auto it = g_dirs.find(path); if (it != g_dirs.end()) return it->second;
    ImgDir& d = g_dirs[path];
    std::ifstream f(path, std::ios::binary); if (!f) return d;
    char magic[4]; uint32_t count; f.read(magic, 4); f.read((char*)&count, 4);
    if (memcmp(magic, "VER2", 4) != 0 || count > 200000) { FSLog("img invalido: %s", path.c_str()); return d; }
    for (uint32_t i = 0; i < count; i++) {
        struct { uint32_t off; uint16_t streaming, inArchive; char name[24]; } e;
        f.read((char*)&e, 32); if (!f) break;
        char nm[25] = {}; memcpy(nm, e.name, 24);
        uint32_t sectors = e.inArchive ? e.inArchive : e.streaming;
        d.files[Lower(nm)] = { e.off * 2048u, sectors * 2048u };
    }
    d.ok = true; return d;
}
namespace ImgArchive {
    bool Read(const std::string& imgPath, const std::string& fileName, std::vector<uint8_t>& out) {
        ImgDir& d = GetDir(imgPath); if (!d.ok) return false;
        auto it = d.files.find(Lower(fileName)); if (it == d.files.end()) return false;
        std::ifstream f(imgPath, std::ios::binary); f.seekg(it->second.offset);
        out.resize(it->second.size); f.read((char*)out.data(), out.size());
        return (bool)f || f.gcount() > 0;
    }
    bool FindGameFile(const std::string& fileName, std::vector<uint8_t>& out, std::string* source) {
        std::string ln = Lower(fileName);
        std::error_code ec;
        if (fs::exists("modloader", ec)) {
            for (auto& p : fs::recursive_directory_iterator("modloader", fs::directory_options::skip_permission_denied, ec)) {
                if (!p.is_regular_file()) continue;
                if (Lower(p.path().filename().string()) == ln) {
                    std::ifstream f(p.path(), std::ios::binary);
                    out.assign(std::istreambuf_iterator<char>(f), {});
                    if (source) *source = "modloader: " + p.path().string();
                    return !out.empty();
                }
            }
        }
        for (const char* img : { "models\\gta3.img", "models\\player.img" })
            if (Read(img, fileName, out)) { if (source) *source = img; return true; }
        return false;
    }
}
