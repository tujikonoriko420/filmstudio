#include "TextureManager.h"
#include "NPCDatabase.h"
#include "ImgArchive.h"
#include "TxdFile.h"
#include "Log.h"
#include "plugin.h"
#include "CPed.h"
#include "CModelInfo.h"
#include "CTxdStore.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include <cstdarg>
#include <windows.h>
#include <shellapi.h>
#include <filesystem>
#include <map>
#include <set>
#include <algorithm>
namespace fs = std::filesystem;
static char g_status[192] = "";
static std::map<std::string, std::vector<TxdTexture>> g_cache;   // TXD decodificado (original)
struct Swapped { RwTexture* tex; RwRaster* original; int txdIndex; };
static std::vector<Swapped> g_swapped;
static std::set<int> g_refd;                                     // TXDs que seguramos com AddRef
static std::string Lower(std::string s) { std::transform(s.begin(), s.end(), s.begin(), ::tolower); return s; }
static void Say(const char* fmt, ...) { va_list a; va_start(a, fmt); vsnprintf(g_status, sizeof g_status, fmt, a); va_end(a); FSLog("%s", g_status); }

static const std::vector<TxdTexture>* LoadOriginal(const std::string& txd) {
    auto it = g_cache.find(Lower(txd)); if (it != g_cache.end()) return &it->second;
    std::vector<uint8_t> bytes; std::string src;
    if (!ImgArchive::FindGameFile(txd + ".txd", bytes, &src)) { Say("TXD '%s.txd' nao encontrado (gta3.img/player.img/modloader)", txd.c_str()); return nullptr; }
    std::vector<TxdTexture> v; std::string err;
    if (!TxdFile::Parse(bytes, v, err)) { Say("Falha ao ler %s.txd (veja FilmStudio.log)", txd.c_str()); FSLog("TXD %s: %s", txd.c_str(), err.c_str()); return nullptr; }
    FSLog("TXD %s lido de %s (%d texturas)", txd.c_str(), src.c_str(), (int)v.size());
    return &(g_cache[Lower(txd)] = std::move(v));
}
namespace TextureManager {
    const char* Status() { return g_status; }
    std::string Folder(const std::string& txd) {
        char cwd[MAX_PATH]; GetCurrentDirectoryA(MAX_PATH, cwd);
        return std::string(cwd) + "\\FilmStudio\\Textures\\" + txd;
    }
    bool Resolve(CPed* ped, Target& t) {
        if (!ped) return false;
        t.modelId = ped->m_nModelIndex;
        const NPCInfo* n = NPCDatabase::Find(t.modelId);
        if (!n) { Say("modelo %d nao esta no ped.ide", t.modelId); return false; }
        t.model = n->model; t.txd = n->txd; return true;
    }
    std::vector<std::string> ListTextures(const std::string& txd) {
        std::vector<std::string> r; auto* v = LoadOriginal(txd);
        if (v) for (auto& t : *v) r.push_back(t.name);
        return r;
    }
    int Export(const std::string& txd, bool overwrite) {
        auto* v = LoadOriginal(txd); if (!v) return -1;
        std::string dir = Folder(txd); std::error_code ec; fs::create_directories(dir, ec);
        int n = 0;
        for (auto& t : *v) {
            std::string path = dir + "\\" + t.name + ".png";
            if (!overwrite && fs::exists(path, ec)) continue;   // nunca apaga suas edicoes sem pedir
            if (stbi_write_png(path.c_str(), t.width, t.height, 4, t.rgba.data(), t.width * 4)) n++;
        }
        snprintf(g_status, sizeof g_status, "Exportadas %d textura(s) para %s", n, dir.c_str());
        return n;
    }
    void OpenFolder(const std::string& txd) {
        std::string dir = Folder(txd); std::error_code ec; fs::create_directories(dir, ec);
        ShellExecuteA(nullptr, "open", dir.c_str(), nullptr, nullptr, SW_SHOWNORMAL); // apenas o Explorer
    }
    int Reload(int modelId, const std::string& txd) {
        CBaseModelInfo* mi = CModelInfo::GetModelInfo(modelId);
        if (!mi) { Say("modelo invalido"); return -1; }
        int txdIdx = mi->m_nTxdIndex;
        auto* def = CTxdStore::ms_pTxdPool->GetAt(txdIdx);
        if (!def || !def->m_pRwDictionary) { Say("TXD do modelo nao esta carregado (aproxime o NPC)"); return -1; }
        if (!g_refd.count(txdIdx)) { CTxdStore::AddRef(txdIdx); g_refd.insert(txdIdx); }  // evita descarregar enquanto trocado
        std::string dir = Folder(txd); std::error_code ec;
        if (!fs::exists(dir, ec)) { Say("Pasta nao existe: exporte primeiro"); return -1; }
        int replaced = 0, missing = 0;
        for (auto& e : fs::directory_iterator(dir, ec)) {
            if (Lower(e.path().extension().string()) != ".png") continue;
            std::string name = e.path().stem().string();
            RwTexture* tex = RwTexDictionaryFindNamedTexture(def->m_pRwDictionary, name.c_str());
            if (!tex) { missing++; continue; }
            int w, h, c; unsigned char* px = stbi_load(e.path().string().c_str(), &w, &h, &c, 4);
            if (!px) { FSLog("PNG invalido: %s", e.path().string().c_str()); continue; }
            RwRaster* r = RwRasterCreate(w, h, 32, rwRASTERTYPETEXTURE | rwRASTERFORMAT8888);
            if (!r) { stbi_image_free(px); continue; }
            unsigned char* dst = RwRasterLock(r, 0, rwRASTERLOCKWRITE);
            if (dst) {
                int stride = r->stride;
                for (int y = 0; y < h; y++) for (int x = 0; x < w; x++) {
                    const unsigned char* s = px + ((size_t)y * w + x) * 4; unsigned char* d = dst + (size_t)y * stride + x * 4;
                    d[0] = s[2]; d[1] = s[1]; d[2] = s[0]; d[3] = s[3];   // RGBA -> BGRA (D3D9)
                }
                RwRasterUnlock(r);
            }
            stbi_image_free(px);
            if (!dst) { RwRasterDestroy(r); continue; }
            bool known = false;
            for (auto& s : g_swapped) if (s.tex == tex) { RwRasterDestroy(tex->raster); known = true; break; } // ja trocada: descarta a nossa antiga
            if (!known) g_swapped.push_back({ tex, tex->raster, txdIdx });
            tex->raster = r;
            RwTextureSetFilterMode(tex, rwFILTERLINEAR);
            replaced++;
        }
        snprintf(g_status, sizeof g_status, "%d textura(s) aplicada(s)%s", replaced, missing ? " (algumas sem equivalente no jogo)" : "");
        return replaced;
    }
    void Restore() {
        for (auto& s : g_swapped) { if (s.tex->raster != s.original) RwRasterDestroy(s.tex->raster); s.tex->raster = s.original; }
        g_swapped.clear();
        for (int idx : g_refd) CTxdStore::RemoveRef(idx);
        g_refd.clear();
        Say("Texturas originais restauradas");
    }
}
