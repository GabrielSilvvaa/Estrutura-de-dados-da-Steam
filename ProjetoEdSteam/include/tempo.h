/*
 * tempo.h - parte dependente de sistema operacional, isolada aqui.
 * Windows (MinGW/Code::Blocks): QueryPerformanceCounter, _mkdir, console UTF-8.
 * Linux/macOS: clock_gettime, mkdir.
 */
#ifndef TEMPO_H
#define TEMPO_H

#include <errno.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <direct.h>

static double agoraMs(void)
{
    LARGE_INTEGER freq, cont;

    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&cont);
    return (double)cont.QuadPart * 1000.0 / (double)freq.QuadPart;
}

static int criaDiretorio(const char *caminho)
{
    return _mkdir(caminho) == 0 || errno == EEXIST;
}

static void prepararConsole(void)
{
    SetConsoleOutputCP(65001); /* nomes de jogos vem em UTF-8 */
}

#else
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

static double agoraMs(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

static int criaDiretorio(const char *caminho)
{
    return mkdir(caminho, 0777) == 0 || errno == EEXIST;
}

static void prepararConsole(void) {}

#endif
#endif
