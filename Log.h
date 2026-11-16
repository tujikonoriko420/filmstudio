#pragma once
#include <cstdio>
#include <cstdarg>
inline void FSLog(const char* fmt, ...) {
    FILE* f = fopen("FilmStudio.log", "a"); if (!f) return;
    va_list a; va_start(a, fmt); vfprintf(f, fmt, a); va_end(a);
    fputc('\n', f); fclose(f);
}
