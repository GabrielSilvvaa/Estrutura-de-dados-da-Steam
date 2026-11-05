/*
 * csv.h - leitor de CSV portatil (sem getline/strcasecmp, funciona no Windows).
 * Reconhece delimitador , ; |  aspas, aspas escapadas ("") e BOM UTF-8.
 * Ignora linhas antes do cabecalho (a que contem a coluna AppID).
 */
#ifndef CSV_H
#define CSV_H

#include <ctype.h>
#include <errno.h>
#include "jogo.h"

#define MAX_CAMPOS 32

/* Le uma linha inteira (qualquer tamanho). Retorna -1 no fim do arquivo. */
static long lerLinhaArquivo(FILE *arq, char **linha, size_t *cap)
{
    char parte[4096];
    size_t n = 0;

    if (!*linha) {
        *cap = 512;
        *linha = (char *)xmalloc(*cap);
    }
    (*linha)[0] = '\0';
    while (fgets(parte, sizeof(parte), arq)) {
        size_t len = strlen(parte);

        if (n + len + 1 > *cap) {
            while (n + len + 1 > *cap) *cap *= 2;
            *linha = (char *)xrealloc(*linha, *cap);
        }
        memcpy(*linha + n, parte, len + 1);
        n += len;
        if (len && parte[len - 1] == '\n') break;
    }
    return n ? (long)n : -1;
}

/* Remove aspas e espacos de um campo. Modifica a string no lugar. */
static char *campo(char *texto)
{
    char *ini = texto, *fim;

    while (*ini && isspace((unsigned char)*ini)) ini++;
    if (*ini == '"') {
        char *origem = ini + 1, *destino = ini;

        while (*origem) {
            if (*origem == '"') {
                if (origem[1] == '"') {
                    *destino++ = '"';
                    origem += 2;
                } else {
                    origem++;
                    break;
                }
            } else {
                *destino++ = *origem++;
            }
        }
        *destino = '\0';
        return ini;
    }

    fim = ini + strlen(ini);
    while (fim > ini && isspace((unsigned char)fim[-1])) fim--;
    *fim = '\0';
    return ini;
}

/* Separa uma linha em campos, respeitando aspas. */
static int separa(char *linha, char delim, char **campos, int max)
{
    int qtd = 0, aspas = 0;
    char *p = linha;

    campos[qtd++] = p;
    while (*p) {
        if (*p == '"') {
            if (aspas && p[1] == '"') p++;
            else aspas = !aspas;
        } else if (*p == delim && !aspas) {
            *p = '\0';
            if (qtd < max) campos[qtd++] = p + 1;
        }
        p++;
    }
    return qtd;
}

static char detectaDelimitador(const char *linha)
{
    const char *candidatos = ",;|";
    int contagem[3] = {0, 0, 0}, aspas = 0, melhor = 0, i;

    for (i = 0; linha[i]; i++) {
        if (linha[i] == '"') {
            if (aspas && linha[i + 1] == '"') i++;
            else aspas = !aspas;
        } else if (!aspas) {
            const char *p = strchr(candidatos, linha[i]);
            if (p) contagem[p - candidatos]++;
        }
    }
    for (i = 1; i < 3; i++)
        if (contagem[i] > contagem[melhor]) melhor = i;
    return contagem[melhor] ? candidatos[melhor] : ',';
}

/* Comparacao sem diferenciar maiusculas (substitui strcasecmp, que e POSIX). */
static int igualSemCaso(const char *a, const char *b)
{
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++;
        b++;
    }
    return *a == *b;
}

/* Se a linha for o cabecalho (tem coluna AppID), devolve 1 e preenche colunas. */
static int analisaCabecalho(const char *linha, char *delim, int *colId,
                            int *colNome, int *colPreco, int *colGenero)
{
    char *copia = duplica(linha);
    char *campos[MAX_CAMPOS];
    int total, i;

    *delim = detectaDelimitador(copia);
    total = separa(copia, *delim, campos, MAX_CAMPOS);
    *colId = *colNome = *colPreco = *colGenero = -1;
    for (i = 0; i < total; i++) {
        char *nome = campo(campos[i]);

        if (igualSemCaso(nome, "AppID")) *colId = i;
        else if (igualSemCaso(nome, "Name")) *colNome = i;
        else if (igualSemCaso(nome, "Price")) *colPreco = i;
        else if (igualSemCaso(nome, "Genres")) *colGenero = i;
    }
    free(copia);
    return *colId >= 0;
}

static int carregaCSV(const char *caminho, Jogo **saida, size_t *quantidade,
                      size_t *invalidas)
{
    FILE *arq;
    char *linha = NULL;
    size_t tamLinha = 0, cap = 1024, n = 0, ruins = 0;
    long lidos;
    char delim = ',';
    char *campos[MAX_CAMPOS];
    int totalCampos, colId = -1, colNome = -1, colPreco = -1, colGenero = -1;
    int achouCab = 0, tentativas = 0;
    Jogo *jogos;

    arq = fopen(caminho, "rb");
    if (!arq) {
        fprintf(stderr, "Erro ao abrir '%s': %s\n", caminho, strerror(errno));
        return 0;
    }

    /* Procura o cabecalho nas primeiras linhas (pula titulos/linhas em branco). */
    while (tentativas++ < 50 && (lidos = lerLinhaArquivo(arq, &linha, &tamLinha)) >= 0) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if ((unsigned char)linha[0] == 0xEF && (unsigned char)linha[1] == 0xBB &&
            (unsigned char)linha[2] == 0xBF)
            memmove(linha, linha + 3, strlen(linha + 3) + 1);
        if (analisaCabecalho(linha, &delim, &colId, &colNome, &colPreco, &colGenero)) {
            achouCab = 1;
            break;
        }
    }
    if (!achouCab) {
        fprintf(stderr, "Coluna AppID nao encontrada em '%s'.\n", caminho);
        free(linha);
        fclose(arq);
        return 0;
    }

    jogos = (Jogo *)xmalloc(cap * sizeof(*jogos));
    while ((lidos = lerLinhaArquivo(arq, &linha, &tamLinha)) >= 0) {
        char *textoId, *fim;
        long long id;

        linha[strcspn(linha, "\r\n")] = '\0';
        if (!linha[0]) continue;

        totalCampos = separa(linha, delim, campos, MAX_CAMPOS);
        if (colId >= totalCampos) { ruins++; continue; }
        textoId = campo(campos[colId]);
        if (!*textoId) { ruins++; continue; }

        errno = 0;
        id = strtoll(textoId, &fim, 10);
        if (fim == textoId || *fim || errno == ERANGE) { ruins++; continue; }

        if (n == cap) {
            cap *= 2;
            jogos = (Jogo *)xrealloc(jogos, cap * sizeof(*jogos));
        }

        jogos[n].app_id = id;
        jogos[n].nome = duplica(colNome >= 0 && colNome < totalCampos
                                    ? campo(campos[colNome]) : "");
        jogos[n].preco = colPreco >= 0 && colPreco < totalCampos
                             ? strtod(campo(campos[colPreco]), NULL) : 0.0;
        jogos[n].genero = duplica(colGenero >= 0 && colGenero < totalCampos
                                      ? campo(campos[colGenero]) : "");
        n++;
    }

    free(linha);
    fclose(arq);
    *saida = jogos;
    *quantidade = n;
    if (invalidas) *invalidas = ruins;
    return 1;
}

/* Procura o CSV nos lugares usuais (Code::Blocks costuma rodar em bin/Debug). */
static const char *localizaCSV(void)
{
    static const char *candidatos[] = {
        "steam_games_clean.csv", "dataset/steam_games_clean.csv",
        "../steam_games_clean.csv", "../dataset/steam_games_clean.csv",
        "../../steam_games_clean.csv", "../../dataset/steam_games_clean.csv"
    };
    size_t i;

    for (i = 0; i < sizeof(candidatos) / sizeof(*candidatos); i++) {
        FILE *f = fopen(candidatos[i], "rb");

        if (f) {
            fclose(f);
            return candidatos[i];
        }
    }
    return NULL;
}

static int testaCSV(void)
{
    char linha1[] = "1,\"Jogo \"\"Top\"\", com virgula\",9.99,\"Acao, Aventura\"";
    char linha2[] = "10|Counter-Strike|1.99|Action";
    char *campos1[8], *campos2[8];

    return separa(linha1, ',', campos1, 8) == 4 &&
           separa(linha2, '|', campos2, 8) == 4 &&
           !strcmp(campo(campos1[0]), "1") &&
           !strcmp(campo(campos1[1]), "Jogo \"Top\", com virgula") &&
           !strcmp(campo(campos1[3]), "Acao, Aventura") &&
           !strcmp(campo(campos2[1]), "Counter-Strike");
}

#endif
