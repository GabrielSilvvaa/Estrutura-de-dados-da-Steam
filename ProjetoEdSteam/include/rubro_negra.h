/*
 * rubro_negra.h - Arvore Rubro-Negra (construcao).
 * Implementacao classica (Cormen et al., cap. 13) com:
 *   - ponteiro para o pai em cada no;
 *   - UM no sentinela NIL por arvore (preto), usado como folha e como pai da raiz.
 *     Vantagem: nunca ha teste "ponteiro == NULL" nos casos de correcao.
 *
 * Propriedades:
 *   1) todo no e vermelho ou preto;   2) a raiz e preta;
 *   3) todo NIL e preto;              4) no vermelho nao tem filho vermelho;
 *   5) todo caminho de um no ate os NIL descendentes tem o mesmo numero de pretos.
 *
 * Chave: AppID. O no aponta para um Jogo que pertence ao vetor do CSV
 * (a arvore nunca libera o Jogo).
 */
#ifndef RUBRO_NEGRA_H
#define RUBRO_NEGRA_H

#include "jogo.h"

typedef enum { RB_VERMELHO = 0, RB_PRETO = 1 } CorRB;

typedef struct NoRB {
    long long chave;
    Jogo *jogo;
    CorRB cor;
    struct NoRB *esq, *dir, *pai;
} NoRB;

typedef struct {
    NoRB *raiz;          /* == nil quando vazia */
    NoRB *nil;           /* sentinela preta */
    size_t quantidade;
    ull rotacoes;
} ArvoreRB;

/* ------------------------- criacao / liberacao ---------------------------- */

static ArvoreRB *rbCria(void)
{
    ArvoreRB *t = (ArvoreRB *)xmalloc(sizeof(*t));

    t->nil = (NoRB *)xmalloc(sizeof(*t->nil));
    t->nil->chave = 0;
    t->nil->jogo = NULL;
    t->nil->cor = RB_PRETO;
    t->nil->esq = t->nil->dir = t->nil->pai = t->nil;
    t->raiz = t->nil;
    t->quantidade = 0;
    t->rotacoes = 0;
    return t;
}

static void rbLiberaNos(ArvoreRB *t, NoRB *no)
{
    if (no == t->nil) return;
    rbLiberaNos(t, no->esq);
    rbLiberaNos(t, no->dir);
    free(no);
}

static void rbLibera(ArvoreRB *t)
{
    if (!t) return;
    rbLiberaNos(t, t->raiz);
    free(t->nil);
    free(t);
}

/* ------------------------------ rotacoes ---------------------------------- */
/*
 * Rotacao a esquerda em x:        x               y
 *                                / \             / \
 *                               a   y    ==>    x   c
 *                                  / \         / \
 *                                 b   c       a   b
 * Preserva a ordem da BST; so muda ponteiros (O(1)). A direita e o espelho.
 */
static void rbRotEsq(ArvoreRB *t, NoRB *x)
{
    NoRB *y = x->dir;

    x->dir = y->esq;
    if (y->esq != t->nil) y->esq->pai = x;
    y->pai = x->pai;
    if (x->pai == t->nil) t->raiz = y;
    else if (x == x->pai->esq) x->pai->esq = y;
    else x->pai->dir = y;
    y->esq = x;
    x->pai = y;
    t->rotacoes++;
}

static void rbRotDir(ArvoreRB *t, NoRB *y)
{
    NoRB *x = y->esq;

    y->esq = x->dir;
    if (x->dir != t->nil) x->dir->pai = y;
    x->pai = y->pai;
    if (y->pai == t->nil) t->raiz = x;
    else if (y == y->pai->esq) y->pai->esq = x;
    else y->pai->dir = x;
    x->dir = y;
    y->pai = x;
    t->rotacoes++;
}

/* ------------------------------- busca ------------------------------------ */

static NoRB *rbBuscaNo(const ArvoreRB *t, long long chave, ull *comp)
{
    NoRB *x = t->raiz;

    while (x != t->nil) {
        if (comp) (*comp)++;
        if (chave == x->chave) return x;
        x = chave < x->chave ? x->esq : x->dir;
    }
    return NULL;
}

static Jogo *rbBusca(const ArvoreRB *t, long long chave, ull *comp)
{
    NoRB *n = rbBuscaNo(t, chave, comp);

    return n ? n->jogo : NULL;
}

/* ------------------------------- insercao --------------------------------- */
/*
 * O no novo nasce VERMELHO (nao altera a altura negra). Isso so pode violar:
 *   - a raiz ser vermelha  (corrigido no fim, pintando-a de preto);
 *   - pai vermelho + filho vermelho. Seja z o no, p o pai, g o avo, u o tio:
 *     caso 1: tio vermelho -> recolore p e u de preto, g de vermelho; sobe z = g.
 *     caso 2: tio preto, z "dobrado" (z e filho interno) -> rotaciona p, vira caso 3.
 *     caso 3: tio preto, z "em linha" -> p preto, g vermelho, rotaciona g.
 */
static void rbInsereFix(ArvoreRB *t, NoRB *z)
{
    while (z->pai->cor == RB_VERMELHO) {
        if (z->pai == z->pai->pai->esq) {
            NoRB *tio = z->pai->pai->dir;

            if (tio->cor == RB_VERMELHO) {                       /* caso 1 */
                z->pai->cor = RB_PRETO;
                tio->cor = RB_PRETO;
                z->pai->pai->cor = RB_VERMELHO;
                z = z->pai->pai;
            } else {
                if (z == z->pai->dir) {                          /* caso 2 */
                    z = z->pai;
                    rbRotEsq(t, z);
                }
                z->pai->cor = RB_PRETO;                          /* caso 3 */
                z->pai->pai->cor = RB_VERMELHO;
                rbRotDir(t, z->pai->pai);
            }
        } else {                                                 /* espelho */
            NoRB *tio = z->pai->pai->esq;

            if (tio->cor == RB_VERMELHO) {
                z->pai->cor = RB_PRETO;
                tio->cor = RB_PRETO;
                z->pai->pai->cor = RB_VERMELHO;
                z = z->pai->pai;
            } else {
                if (z == z->pai->esq) {
                    z = z->pai;
                    rbRotDir(t, z);
                }
                z->pai->cor = RB_PRETO;
                z->pai->pai->cor = RB_VERMELHO;
                rbRotEsq(t, z->pai->pai);
            }
        }
    }
    t->raiz->cor = RB_PRETO;
}

/* Retorna 1 se inseriu; 0 se o AppID ja existia (nada e alterado). */
static int rbInsere(ArvoreRB *t, Jogo *jogo, ull *comp)
{
    NoRB *pai = t->nil, *x = t->raiz, *z;

    while (x != t->nil) {
        pai = x;
        if (comp) (*comp)++;
        if (jogo->app_id < x->chave) x = x->esq;
        else if (jogo->app_id > x->chave) x = x->dir;
        else return 0;
    }

    z = (NoRB *)xmalloc(sizeof(*z));
    z->chave = jogo->app_id;
    z->jogo = jogo;
    z->cor = RB_VERMELHO;
    z->esq = z->dir = t->nil;
    z->pai = pai;
    if (pai == t->nil) t->raiz = z;
    else if (z->chave < pai->chave) pai->esq = z;
    else pai->dir = z;

    t->quantidade++;
    rbInsereFix(t, z);
    return 1;
}

/* -------------------------------- remocao --------------------------------- */

/* Coloca a subarvore v no lugar da subarvore u (ajusta so o pai de u). */
static void rbTransplanta(ArvoreRB *t, NoRB *u, NoRB *v)
{
    if (u->pai == t->nil) t->raiz = v;
    else if (u == u->pai->esq) u->pai->esq = v;
    else u->pai->dir = v;
    v->pai = u->pai;   /* vale mesmo se v for NIL: o fix usa nil->pai */
}

static NoRB *rbMinimo(const ArvoreRB *t, NoRB *x)
{
    while (x->esq != t->nil) x = x->esq;
    return x;
}

/*
 * Remover um no PRETO tira um preto de um caminho: o no x que ocupou o lugar
 * fica com "preto duplo". Sendo w o irmao de x:
 *   caso 1: w vermelho            -> recolore e rotaciona o pai; cai no 2, 3 ou 4.
 *   caso 2: w preto, filhos pretos -> w vira vermelho; o "preto extra" sobe (x = pai).
 *   caso 3: w preto, filho distante preto e proximo vermelho -> rotaciona w; vira caso 4.
 *   caso 4: w preto, filho distante vermelho -> recolore e rotaciona o pai; termina.
 * Termina quando x e vermelho (basta pinta-lo de preto) ou x e a raiz.
 */
static void rbRemoveFix(ArvoreRB *t, NoRB *x)
{
    while (x != t->raiz && x->cor == RB_PRETO) {
        if (x == x->pai->esq) {
            NoRB *w = x->pai->dir;

            if (w->cor == RB_VERMELHO) {                         /* caso 1 */
                w->cor = RB_PRETO;
                x->pai->cor = RB_VERMELHO;
                rbRotEsq(t, x->pai);
                w = x->pai->dir;
            }
            if (w->esq->cor == RB_PRETO && w->dir->cor == RB_PRETO) { /* caso 2 */
                w->cor = RB_VERMELHO;
                x = x->pai;
            } else {
                if (w->dir->cor == RB_PRETO) {                   /* caso 3 */
                    w->esq->cor = RB_PRETO;
                    w->cor = RB_VERMELHO;
                    rbRotDir(t, w);
                    w = x->pai->dir;
                }
                w->cor = x->pai->cor;                            /* caso 4 */
                x->pai->cor = RB_PRETO;
                w->dir->cor = RB_PRETO;
                rbRotEsq(t, x->pai);
                x = t->raiz;
            }
        } else {                                                 /* espelho */
            NoRB *w = x->pai->esq;

            if (w->cor == RB_VERMELHO) {
                w->cor = RB_PRETO;
                x->pai->cor = RB_VERMELHO;
                rbRotDir(t, x->pai);
                w = x->pai->esq;
            }
            if (w->dir->cor == RB_PRETO && w->esq->cor == RB_PRETO) {
                w->cor = RB_VERMELHO;
                x = x->pai;
            } else {
                if (w->esq->cor == RB_PRETO) {
                    w->dir->cor = RB_PRETO;
                    w->cor = RB_VERMELHO;
                    rbRotEsq(t, w);
                    w = x->pai->esq;
                }
                w->cor = x->pai->cor;
                x->pai->cor = RB_PRETO;
                w->esq->cor = RB_PRETO;
                rbRotDir(t, x->pai);
                x = t->raiz;
            }
        }
    }
    x->cor = RB_PRETO;
}

/* Remove o AppID. Retorna o Jogo (sem libera-lo) ou NULL se nao existir. */
static Jogo *rbRemove(ArvoreRB *t, long long chave, ull *comp)
{
    NoRB *z = rbBuscaNo(t, chave, comp), *y, *x;
    CorRB corOriginal;
    Jogo *removido;

    if (!z) return NULL;
    removido = z->jogo;
    y = z;
    corOriginal = y->cor;

    if (z->esq == t->nil) {                 /* folha ou so filho direito */
        x = z->dir;
        rbTransplanta(t, z, z->dir);
    } else if (z->dir == t->nil) {          /* so filho esquerdo */
        x = z->esq;
        rbTransplanta(t, z, z->esq);
    } else {                                /* dois filhos: usa o sucessor */
        y = rbMinimo(t, z->dir);
        corOriginal = y->cor;
        x = y->dir;
        if (y->pai == z) {
            x->pai = y;
        } else {
            rbTransplanta(t, y, y->dir);
            y->dir = z->dir;
            y->dir->pai = y;
        }
        rbTransplanta(t, z, y);
        y->esq = z->esq;
        y->esq->pai = y;
        y->cor = z->cor;
    }
    free(z);
    t->quantidade--;
    if (corOriginal == RB_PRETO) rbRemoveFix(t, x);
    t->nil->pai = t->nil;
    return removido;
}

/* ------------------------ altura, validacao, percursos -------------------- */

static int rbAltura(const ArvoreRB *t, const NoRB *no)
{
    int he, hd;

    if (no == t->nil) return 0;
    he = rbAltura(t, no->esq);
    hd = rbAltura(t, no->dir);
    return 1 + (he > hd ? he : hd);
}

/* Altura negra: pretos no caminho mais a esquerda (sem contar o NIL). */
static int rbAlturaNegra(const ArvoreRB *t)
{
    const NoRB *x = t->raiz;
    int h = 0;

    while (x != t->nil) {
        if (x->cor == RB_PRETO) h++;
        x = x->esq;
    }
    return h;
}

/* Devolve 1 se valido. *bh recebe a altura negra da subarvore (NIL conta 1). */
static int rbValidaNo(const ArvoreRB *t, const NoRB *no, const long long *lo,
                      const long long *hi, int *bh, const char **erro, size_t *cont)
{
    int bhE, bhD;

    if (no == t->nil) {
        if (no->cor != RB_PRETO) {
            *erro = "propriedade 3 violada: NIL nao e preto";
            return 0;
        }
        *bh = 1;
        return 1;
    }
    (*cont)++;
    if ((lo && no->chave <= *lo) || (hi && no->chave >= *hi)) {
        *erro = "propriedade de arvore de busca violada (ordem das chaves)";
        return 0;
    }
    if ((no->esq != t->nil && no->esq->pai != no) ||
        (no->dir != t->nil && no->dir->pai != no)) {
        *erro = "ponteiro para o pai inconsistente";
        return 0;
    }
    if (no->cor == RB_VERMELHO &&
        (no->esq->cor == RB_VERMELHO || no->dir->cor == RB_VERMELHO)) {
        *erro = "propriedade 4 violada: no vermelho com filho vermelho";
        return 0;
    }
    if (!rbValidaNo(t, no->esq, lo, &no->chave, &bhE, erro, cont)) return 0;
    if (!rbValidaNo(t, no->dir, &no->chave, hi, &bhD, erro, cont)) return 0;
    if (bhE != bhD) {
        *erro = "propriedade 5 violada: alturas negras diferentes";
        return 0;
    }
    *bh = bhE + (no->cor == RB_PRETO ? 1 : 0);
    return 1;
}

/* Retorna 1 se a Rubro-Negra respeita todas as propriedades; senao 0 + motivo. */
static int rbValida(const ArvoreRB *t, char *msg, size_t tam)
{
    const char *erro = "ok";
    size_t cont = 0;
    int bh = 0, ok = 1;

    if (t->nil->cor != RB_PRETO) {
        erro = "propriedade 3 violada: NIL nao e preto";
        ok = 0;
    } else if (t->raiz != t->nil && t->raiz->cor != RB_PRETO) {
        erro = "propriedade 2 violada: a raiz nao e preta";
        ok = 0;
    } else if (t->raiz != t->nil && t->raiz->pai != t->nil) {
        erro = "o pai da raiz deveria ser NIL";
        ok = 0;
    } else {
        ok = rbValidaNo(t, t->raiz, NULL, NULL, &bh, &erro, &cont);
    }
    if (ok && cont != t->quantidade) {
        erro = "quantidade de nos difere do contador da arvore";
        ok = 0;
    }
    if (msg && tam) {
        strncpy(msg, ok ? "Rubro-Negra valida" : erro, tam - 1);
        msg[tam - 1] = '\0';
    }
    return ok;
}

/* Percursos: ordem 0 = em ordem, 1 = pre-ordem, 2 = pos-ordem. */
static void rbPercorre(const ArvoreRB *t, const NoRB *no, int ordem,
                       VisitaFn visita, void *ctx)
{
    if (no == t->nil) return;
    if (ordem == 1) visita(no->jogo, ctx);
    rbPercorre(t, no->esq, ordem, visita, ctx);
    if (ordem == 0) visita(no->jogo, ctx);
    rbPercorre(t, no->dir, ordem, visita, ctx);
    if (ordem == 2) visita(no->jogo, ctx);
}

#endif
