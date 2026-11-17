#include "TxdFile.h"
#include <cstring>
#include <algorithm>
static uint32_t U32(const uint8_t* p) { uint32_t v; memcpy(&v, p, 4); return v; }
static uint16_t U16(const uint8_t* p) { uint16_t v; memcpy(&v, p, 2); return v; }
static void C565(uint16_t c, uint8_t* o) { o[0] = ((c >> 11) & 31) * 255 / 31; o[1] = ((c >> 5) & 63) * 255 / 63; o[2] = (c & 31) * 255 / 31; }

static void DecodeDXT(const uint8_t* s, int w, int h, int type, std::vector<uint8_t>& rgba) {
    rgba.assign((size_t)w * h * 4, 0);
    int bw = (w + 3) / 4, bh = (h + 3) / 4;
    for (int by = 0; by < bh; by++) for (int bx = 0; bx < bw; bx++) {
        uint8_t alpha[16]; for (int i = 0; i < 16; i++) alpha[i] = 255;
        if (type == 3) {
            for (int i = 0; i < 8; i++) { alpha[i * 2] = (s[i] & 15) * 17; alpha[i * 2 + 1] = (s[i] >> 4) * 17; }
            s += 8;
        } else if (type == 5) {
            uint8_t a0 = s[0], a1 = s[1], pal[8]; pal[0] = a0; pal[1] = a1;
            if (a0 > a1) for (int i = 1; i < 7; i++) pal[i + 1] = ((7 - i) * a0 + i * a1) / 7;
            else { for (int i = 1; i < 5; i++) pal[i + 1] = ((5 - i) * a0 + i * a1) / 5; pal[6] = 0; pal[7] = 255; }
            uint64_t bits = 0; for (int i = 0; i < 6; i++) bits |= (uint64_t)s[2 + i] << (8 * i);
            for (int i = 0; i < 16; i++) alpha[i] = pal[(bits >> (3 * i)) & 7];
            s += 8;
        }
        uint16_t c0 = U16(s), c1 = U16(s + 2); uint32_t idx = U32(s + 4); s += 8;
        uint8_t col[4][4]; C565(c0, col[0]); C565(c1, col[1]); col[0][3] = col[1][3] = 255;
        if (c0 > c1 || type != 1) for (int k = 0; k < 3; k++) { col[2][k] = (2 * col[0][k] + col[1][k]) / 3; col[3][k] = (col[0][k] + 2 * col[1][k]) / 3; }
        else for (int k = 0; k < 3; k++) { col[2][k] = (col[0][k] + col[1][k]) / 2; col[3][k] = 0; }
        col[2][3] = 255; col[3][3] = (type == 1 && c0 <= c1) ? 0 : 255;
        for (int py = 0; py < 4; py++) for (int px = 0; px < 4; px++) {
            int x = bx * 4 + px, y = by * 4 + py; if (x >= w || y >= h) continue;
            int ci = (idx >> (2 * (py * 4 + px))) & 3; uint8_t* o = &rgba[((size_t)y * w + x) * 4];
            o[0] = col[ci][0]; o[1] = col[ci][1]; o[2] = col[ci][2];
            o[3] = (type == 1) ? col[ci][3] : alpha[py * 4 + px];
        }
    }
}
namespace TxdFile {
    bool Parse(const std::vector<uint8_t>& d, std::vector<TxdTexture>& out, std::string& err) {
        out.clear();
        if (d.size() < 28 || U32(d.data()) != 0x16) { err = "arquivo nao e um TXD"; return false; }
        size_t pos = 12; // dentro do TextureDictionary
        if (U32(&d[pos]) != 1) { err = "TXD sem struct"; return false; }
        uint16_t count = U16(&d[pos + 12]); pos += 12 + U32(&d[pos + 4]);
        for (int i = 0; i < count && pos + 12 <= d.size(); i++) {
            if (U32(&d[pos]) != 0x15) break;
            size_t texEnd = pos + 12 + U32(&d[pos + 4]); size_t s = pos + 12; // struct header
            size_t p = s + 12; const uint8_t* b = &d[p];
            if (p + 88 > d.size()) break;
            uint32_t platform = U32(b), rfmt = U32(b + 72), d3dfmt = U32(b + 76);
            TxdTexture t; char nm[33] = {}; memcpy(nm, b + 8, 32); t.name = nm;
            t.width = U16(b + 80); t.height = U16(b + 82);
            int depth = b[84]; uint8_t flagsByte = b[87];
            const uint8_t* q = b + 88; size_t avail = d.size() - (p + 88);
            bool pal8 = rfmt & 0x2000, pal4 = rfmt & 0x4000; const uint8_t* pal = nullptr;
            if (pal8) { pal = q; q += 1024; } else if (pal4) { pal = q; q += 128; }
            if ((size_t)(q - &d[0]) + 4 > d.size()) break;
            uint32_t sz = U32(q); q += 4;
            if ((size_t)(q - &d[0]) + sz > d.size()) { pos = texEnd; continue; }
            int dxt = 0;
            if (platform == 9) { if (d3dfmt == 0x31545844) dxt = 1; else if (d3dfmt == 0x33545844 || d3dfmt == 0x32545844) dxt = 3; else if (d3dfmt == 0x35545844 || d3dfmt == 0x34545844) dxt = 5; }
            else { if (flagsByte == 1) dxt = 1; else if (flagsByte == 2 || flagsByte == 3) dxt = 3; else if (flagsByte >= 4) dxt = 5; }
            (void)avail;
            int w = t.width, h = t.height; bool ok = true;
            if (dxt) DecodeDXT(q, w, h, dxt, t.rgba);
            else if (pal) {
                t.rgba.assign((size_t)w * h * 4, 0);
                for (int k = 0; k < w * h; k++) {
                    int idx = pal8 ? q[k] : ((k & 1) ? q[k / 2] >> 4 : q[k / 2] & 15);
                    memcpy(&t.rgba[(size_t)k * 4], pal + idx * 4, 4);
                }
            } else {
                t.rgba.assign((size_t)w * h * 4, 255); int fmt = rfmt & 0xF00;
                for (int k = 0; k < w * h; k++) {
                    uint8_t* o = &t.rgba[(size_t)k * 4];
                    if (depth == 32) { const uint8_t* s4 = q + (size_t)k * 4; o[0] = s4[2]; o[1] = s4[1]; o[2] = s4[0]; o[3] = (fmt == 0x600) ? 255 : s4[3]; }
                    else if (depth == 16) {
                        uint16_t v = U16(q + (size_t)k * 2);
                        if (fmt == 0x200) C565(v, o);
                        else if (fmt == 0x100) { o[0] = ((v >> 10) & 31) * 255 / 31; o[1] = ((v >> 5) & 31) * 255 / 31; o[2] = (v & 31) * 255 / 31; o[3] = (v & 0x8000) ? 255 : 0; }
                        else { o[3] = ((v >> 12) & 15) * 17; o[0] = ((v >> 8) & 15) * 17; o[1] = ((v >> 4) & 15) * 17; o[2] = (v & 15) * 17; }
                    } else ok = false;
                }
            }
            if (ok && w > 0 && h > 0) out.push_back(std::move(t));
            pos = texEnd;
        }
        if (out.empty()) { err = "nenhuma textura decodificada"; return false; }
        return true;
    }
}
