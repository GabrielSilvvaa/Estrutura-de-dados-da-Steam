/*
 * jogo.h - tipos e utilitarios comuns (usados pela AVL e pela Rubro-Negra).
 * Biblioteca de cabecalho (header-only): so declaracoes "static".
 * Os registros Jogo pertencem ao VETOR carregado do CSV; as arvores apenas
 * apontam para eles e NUNCA liberam um Jogo ao remover um no.
 */
#ifndef JOGO_H
#define JOGO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long long ull;

typedef struct {
    long long app_id;   /* chave unica das arvores */
    char *nome;
    double preco;
    char *genero;
} Jogo;

/* Funcao chamada em cada no durante um percurso. */
typedef void (*VisitaFn)(const Jogo *jogo, void *ctx);

static void *xmalloc(size_t tamanho)
{
    void *p = malloc(tamanho ? tamanho : 1);

    if (!p) {
        fprintf(stderr, "Sem memoria.\n");
        exit(1);
    }
    return p;
}

static void *xrealloc(void *p, size_t tamanho)
{
    void *novo = realloc(p, tamanho ? tamanho : 1);

    if (!novo) {
        fprintf(stderr, "Sem memoria.\n");
        exit(1);
    }
    return novo;
}

static char *duplica(const char *texto)
{
    char *copia = (char *)xmalloc(strlen(texto) + 1);

    strcpy(copia, texto);
    return copia;
}

static void liberaJogos(Jogo *jogos, size_t quantidade)
{
    size_t i;

    for (i = 0; i < quantidade; i++) {
        free(jogos[i].nome);
        free(jogos[i].genero);
    }
    free(jogos);
}

/* Gerador pseudoaleatorio (splitmix64): reprodutivel e igual em qualquer SO. */
static ull proximoAleatorio(ull *estado)
{
    ull z = (*estado += 0x9E3779B97F4A7C15ULL);

    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

/* log2 sem depender de math.h/-lm (so para exibir limites teoricos). */
static double log2d(double x)
{
    double e = 0.0, y, y2, termo, soma = 0.0;
    int k;

    if (x < 1.0) return 0.0;
    while (x >= 2.0) {
        x /= 2.0;
        e += 1.0;
    }
    y = (x - 1.0) / (x + 1.0);
    y2 = y * y;
    termo = y;
    for (k = 0; k < 12; k++) {
        soma += termo / (2 * k + 1);
        termo *= y2;
    }
    return e + 2.0 * soma / 0.6931471805599453;
}

#endif
