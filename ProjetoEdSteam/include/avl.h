/*
 * avl.h - Arvore AVL (construcao). Codigo da AVL do trabalho original,
 * preservado; acrescentados apenas: contador de rotacoes, percursos e
 * validacao com mensagem (para menu, testes e benchmark).
 * Chave: AppID. O no aponta para um Jogo que pertence ao vetor do CSV.
 */
#ifndef AVL_H
#define AVL_H

#include "jogo.h"

typedef struct No {
    long long chave;
    Jogo *jogo;
    int altura;
    struct No *esq, *dir;
} No;

typedef struct {
    No *raiz;
    size_t quantidade;
} Arvore;

/* Total de rotacoes feitas (zerado quando uma arvore nova e criada). */
static ull avlRotacoes = 0;

static int maior(int a, int b) { return a > b ? a : b; }
static int altura(No *no) { return no ? no->altura : 0; }
static int fator(No *no) { return no ? altura(no->esq) - altura(no->dir) : 0; }

static void atualiza(No *no)
{
    no->altura = 1 + maior(altura(no->esq), altura(no->dir));
}

static No *rotacionaDireita(No *y)
{
    No *x = y->esq, *b = x->dir;

    x->dir = y;
    y->esq = b;
    atualiza(y);
    atualiza(x);
    avlRotacoes++;
    return x;
}

static No *rotacionaEsquerda(No *x)
{
    No *y = x->dir, *b = y->esq;

    y->esq = x;
    x->dir = b;
    atualiza(x);
    atualiza(y);
    avlRotacoes++;
    return y;
}

static No *balanceia(No *no)
{
    int fb;

    atualiza(no);
    fb = fator(no);
    if (fb > 1) {
        if (fator(no->esq) < 0) no->esq = rotacionaEsquerda(no->esq);
        return rotacionaDireita(no);
    }
    if (fb < -1) {
        if (fator(no->dir) > 0) no->dir = rotacionaDireita(no->dir);
        return rotacionaEsquerda(no);
    }
    return no;
}

static No *insereNo(No *no, Jogo *jogo, ull *comp, int *novo)
{
    if (!no) {
        No *n = (No *)xmalloc(sizeof(*n));

        n->chave = jogo->app_id;
        n->jogo = jogo;
        n->altura = 1;
        n->esq = n->dir = NULL;
        *novo = 1;
        return n;
    }

    if (comp) (*comp)++;
    if (jogo->app_id < no->chave)
        no->esq = insereNo(no->esq, jogo, comp, novo);
    else if (jogo->app_id > no->chave)
        no->dir = insereNo(no->dir, jogo, comp, novo);
    else {
        *novo = 0;
        return no;
    }
    return balanceia(no);
}

static void insere(Arvore *arvore, Jogo *jogo, ull *comp)
{
    int novo = 0;

    arvore->raiz = insereNo(arvore->raiz, jogo, comp, &novo);
    arvore->quantidade += novo;
}

static Jogo *busca(No *no, long long chave, ull *comp)
{
    while (no) {
        if (comp) (*comp)++;
        if (chave == no->chave) return no->jogo;
        no = chave < no->chave ? no->esq : no->dir;
    }
    return NULL;
}

static No *menorNo(No *no)
{
    while (no->esq) no = no->esq;
    return no;
}

static No *removeMenor(No *no)
{
    No *dir;

    if (!no->esq) {
        dir = no->dir;
        free(no);
        return dir;
    }
    no->esq = removeMenor(no->esq);
    return balanceia(no);
}

static No *removeNo(No *no, long long chave, ull *comp, Jogo **removido)
{
    if (!no) return NULL;

    if (comp) (*comp)++;
    if (chave < no->chave)
        no->esq = removeNo(no->esq, chave, comp, removido);
    else if (chave > no->chave)
        no->dir = removeNo(no->dir, chave, comp, removido);
    else {
        *removido = no->jogo;
        if (!no->esq || !no->dir) {
            No *filho = no->esq ? no->esq : no->dir;

            free(no);
            return filho;
        }
        {
            No *sucessor = menorNo(no->dir);

            no->chave = sucessor->chave;
            no->jogo = sucessor->jogo;
            no->dir = removeMenor(no->dir);
        }
    }
    return balanceia(no);
}

static Jogo *removeJogo(Arvore *arvore, long long chave, ull *comp)
{
    Jogo *removido = NULL;

    arvore->raiz = removeNo(arvore->raiz, chave, comp, &removido);
    if (removido) arvore->quantidade--;
    return removido;
}

static void liberaArvore(No *no)
{
    if (!no) return;
    liberaArvore(no->esq);
    liberaArvore(no->dir);
    free(no);
}

/* Verificacao estrutural original: ordem BST, alturas e fator em -1..+1. */
static int confereNo(No *no, No *limiteMenor, No *limiteMaior)
{
    int he, hd;

    if (!no) return 1;
    if ((limiteMenor && no->chave <= limiteMenor->chave) ||
        (limiteMaior && no->chave >= limiteMaior->chave))
        return 0;

    he = altura(no->esq);
    hd = altura(no->dir);
    if (no->altura != 1 + maior(he, hd) || he - hd < -1 || he - hd > 1)
        return 0;

    return confereNo(no->esq, limiteMenor, no) &&
           confereNo(no->dir, no, limiteMaior);
}

static int confere(No *raiz) { return confereNo(raiz, NULL, NULL); }

/* ----------------- acrescimos: validacao com mensagem --------------------- */

static int avlValidaNo(No *no, const long long *lo, const long long *hi,
                       const char **erro, size_t *cont)
{
    int he, hd;

    if (!no) return 1;
    (*cont)++;
    if ((lo && no->chave <= *lo) || (hi && no->chave >= *hi)) {
        *erro = "ordem da arvore de busca violada";
        return 0;
    }
    he = altura(no->esq);
    hd = altura(no->dir);
    if (no->altura != 1 + maior(he, hd)) {
        *erro = "altura armazenada no no esta incorreta";
        return 0;
    }
    if (he - hd < -1 || he - hd > 1) {
        *erro = "fator de balanceamento fora de -1..+1";
        return 0;
    }
    return avlValidaNo(no->esq, lo, &no->chave, erro, cont) &&
           avlValidaNo(no->dir, &no->chave, hi, erro, cont);
}

/* Retorna 1 se a AVL esta correta; senao 0 e escreve o motivo em msg. */
static int avlValida(const Arvore *arvore, char *msg, size_t tam)
{
    const char *erro = "ok";
    size_t cont = 0;
    int ok = avlValidaNo(arvore->raiz, NULL, NULL, &erro, &cont);

    if (ok && cont != arvore->quantidade) {
        erro = "quantidade de nos difere do contador da arvore";
        ok = 0;
    }
    if (ok && !confere(arvore->raiz)) {
        erro = "verificacao estrutural original falhou";
        ok = 0;
    }
    if (msg && tam) {
        strncpy(msg, ok ? "AVL valida" : erro, tam - 1);
        msg[tam - 1] = '\0';
    }
    return ok;
}

/* Percursos: ordem 0 = em ordem, 1 = pre-ordem, 2 = pos-ordem. */
static void avlPercorre(No *no, int ordem, VisitaFn visita, void *ctx)
{
    if (!no) return;
    if (ordem == 1) visita(no->jogo, ctx);
    avlPercorre(no->esq, ordem, visita, ctx);
    if (ordem == 0) visita(no->jogo, ctx);
    avlPercorre(no->dir, ordem, visita, ctx);
    if (ordem == 2) visita(no->jogo, ctx);
}

#endif
