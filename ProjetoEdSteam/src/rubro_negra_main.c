/*
 * rubro_negra_main.c - programa executavel da Rubro-Negra (independente da AVL).
 * Liga as funcoes de rubro_negra.h a interface OpsArvore e chama o menu.
 *
 * Compilar (Windows/CMD ou Linux):
 *   gcc -std=c11 -O2 -Wall -Wextra -o rubro_negra rubro_negra_main.c
 */
#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L
#endif

#include "rubro_negra.h"
#include "menu.h"

static void *rCria(void) { return rbCria(); }
static void rLibera(void *p) { rbLibera((ArvoreRB *)p); }

static int rInsere(void *p, Jogo *j, ull *comp)
{
    return rbInsere((ArvoreRB *)p, j, comp);
}

static Jogo *rBusca(void *p, long long chave, ull *comp)
{
    return rbBusca((ArvoreRB *)p, chave, comp);
}

static Jogo *rRemove(void *p, long long chave, ull *comp)
{
    return rbRemove((ArvoreRB *)p, chave, comp);
}

static size_t rTamanho(void *p) { return ((ArvoreRB *)p)->quantidade; }

static int rAltura(void *p)
{
    ArvoreRB *t = (ArvoreRB *)p;

    return rbAltura(t, t->raiz);
}

static ull rRotacoes(void *p) { return ((ArvoreRB *)p)->rotacoes; }

static int rValida(void *p, char *msg, size_t tam)
{
    return rbValida((ArvoreRB *)p, msg, tam);
}

static void rPercorre(void *p, int ordem, VisitaFn f, void *ctx)
{
    ArvoreRB *t = (ArvoreRB *)p;

    rbPercorre(t, t->raiz, ordem, f, ctx);
}

static int rRaiz(void *p, long long *chave)
{
    ArvoreRB *t = (ArvoreRB *)p;

    if (t->raiz == t->nil) return 0;
    *chave = t->raiz->chave;
    return 1;
}

static void rInfo(void *p, char *buf, size_t tam)
{
    ArvoreRB *t = (ArvoreRB *)p;

    snprintf(buf, tam,
             "Raiz: %lld (%s) | altura negra: %d\n"
             "Limite teorico de altura (2*log2(n+1)): %.1f",
             t->raiz == t->nil ? 0LL : t->raiz->chave,
             t->raiz == t->nil || t->raiz->cor == RB_PRETO ? "preta" : "vermelha",
             rbAlturaNegra(t), 2.0 * log2d((double)t->quantidade + 1.0));
}

int main(int argc, char **argv)
{
    static const OpsArvore ops = {
        "Rubro-Negra", "rb", rCria, rLibera, rInsere, rBusca, rRemove, rTamanho,
        rAltura, rRotacoes, rValida, rPercorre, rRaiz, rInfo
    };

    return programaPrincipal(&ops, argc, argv);
}
