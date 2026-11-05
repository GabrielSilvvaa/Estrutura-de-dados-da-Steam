/*
 * arvore_ops.h - "interface" comum de uma arvore de busca.
 * Cada programa (AVL ou Rubro-Negra) preenche uma OpsArvore com ponteiros
 * para as SUAS funcoes. Menu, testes e benchmark so conhecem essa interface,
 * entao rodam identicos nas duas arvores (comparacao justa) sem que uma
 * dependa do codigo da outra.
 */
#ifndef ARVORE_OPS_H
#define ARVORE_OPS_H

#include "jogo.h"

typedef struct {
    const char *nome;       /* "AVL" ou "Rubro-Negra" */
    const char *prefixo;    /* usado no nome do arquivo de resultados */
    void *(*cria)(void);
    void (*libera)(void *arv);                           /* libera nos, nao os Jogos */
    int (*insere)(void *arv, Jogo *j, ull *comp);        /* 1 = inseriu, 0 = duplicado */
    Jogo *(*busca)(void *arv, long long chave, ull *comp);
    Jogo *(*removeChave)(void *arv, long long chave, ull *comp); /* NULL se nao existe */
    size_t (*tamanho)(void *arv);
    int (*altura)(void *arv);                            /* em nos; vazia = 0 */
    ull (*rotacoes)(void *arv);                          /* acumuladas desde cria() */
    int (*valida)(void *arv, char *msg, size_t tam);     /* 1 = ok; msg explica a falha */
    void (*percorre)(void *arv, int ordem, VisitaFn f, void *ctx); /* 0 em, 1 pre, 2 pos */
    int (*raiz)(void *arv, long long *chave);            /* 0 se vazia */
    void (*info)(void *arv, char *buf, size_t tam);      /* texto extra de balanceamento */
} OpsArvore;

#endif
