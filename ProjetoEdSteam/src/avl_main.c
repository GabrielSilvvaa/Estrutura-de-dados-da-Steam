/*
 * avl_main.c - programa executavel da AVL (independente da Rubro-Negra).
 * Liga as funcoes de avl.h a interface OpsArvore e chama o menu.
 *
 * Compilar (Windows/CMD ou Linux):
 *   gcc -std=c11 -O2 -Wall -Wextra -o avl avl_main.c
 */
#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "avl.h"
#include "menu.h"

static void *aCria(void)
{
    Arvore *a = (Arvore *)xmalloc(sizeof(*a));

    a->raiz = NULL;
    a->quantidade = 0;
    avlRotacoes = 0;
    return a;
}

static void aLibera(void *p)
{
    Arvore *a = (Arvore *)p;

    if (!a) return;
    liberaArvore(a->raiz);
    free(a);
}

static int aInsere(void *p, Jogo *j, ull *comp)
{
    Arvore *a = (Arvore *)p;
    size_t antes = a->quantidade;

    insere(a, j, comp);
    return a->quantidade > antes;
}

static Jogo *aBusca(void *p, long long chave, ull *comp)
{
    return busca(((Arvore *)p)->raiz, chave, comp);
}

static Jogo *aRemove(void *p, long long chave, ull *comp)
{
    return removeJogo((Arvore *)p, chave, comp);
}

static size_t aTamanho(void *p) { return ((Arvore *)p)->quantidade; }
static int aAltura(void *p) { return altura(((Arvore *)p)->raiz); }
static ull aRotacoes(void *p) { (void)p; return avlRotacoes; }

static int aValida(void *p, char *msg, size_t tam)
{
    return avlValida((Arvore *)p, msg, tam);
}

static void aPercorre(void *p, int ordem, VisitaFn f, void *ctx)
{
    avlPercorre(((Arvore *)p)->raiz, ordem, f, ctx);
}

static int aRaiz(void *p, long long *chave)
{
    Arvore *a = (Arvore *)p;

    if (!a->raiz) return 0;
    *chave = a->raiz->chave;
    return 1;
}

static void aInfo(void *p, char *buf, size_t tam)
{
    Arvore *a = (Arvore *)p;

    snprintf(buf, tam,
             "Raiz: %lld | fator de balanceamento da raiz: %d (permitido: -1..+1)\n"
             "Limite teorico de altura (~1,44*log2(n+2)): %.1f",
             a->raiz ? a->raiz->chave : 0LL, fator(a->raiz),
             1.44 * log2d((double)a->quantidade + 2.0));
}

int main(int argc, char **argv)
{
    static const OpsArvore ops = {
        "AVL", "avl", aCria, aLibera, aInsere, aBusca, aRemove, aTamanho,
        aAltura, aRotacoes, aValida, aPercorre, aRaiz, aInfo
    };

    return programaPrincipal(&ops, argc, argv);
}
