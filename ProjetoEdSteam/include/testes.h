/*
 * testes.h - bateria de testes generica: roda igual na AVL e na Rubro-Negra
 * (usa apenas a interface OpsArvore). Apos CADA operacao das sequencias
 * validamos as propriedades da arvore.
 */
#ifndef TESTES_H
#define TESTES_H

#include "arvore_ops.h"
#include "csv.h"

#define EXIGE(cond, texto)                                         \
    do {                                                           \
        if (!(cond)) {                                             \
            printf("    falhou: %s (linha %d)\n", texto, __LINE__); \
            ok = 0;                                                \
            goto fim;                                              \
        }                                                          \
    } while (0)

/* Limite generoso valido para as duas: 2*log2(n+1) cobre a Rubro-Negra. */
static int limiteAlturaTeste(size_t n)
{
    int h = 0;
    size_t v = n + 1;

    while (v > 1) {
        v >>= 1;
        h++;
    }
    return 2 * (h + 1);
}

/* Valida; em caso de falha mostra a mensagem da arvore. */
static int validaOuMostra(const OpsArvore *ops, void *arv)
{
    char msg[160];

    if (ops->valida(arv, msg, sizeof(msg))) return 1;
    printf("    arvore invalida: %s\n", msg);
    return 0;
}

static Jogo *criaJogosTeste(int n)
{
    Jogo *v = (Jogo *)xmalloc((size_t)n * sizeof(*v));
    int i;

    for (i = 0; i < n; i++) {
        v[i].app_id = i + 1;
        v[i].nome = (char *)"Jogo";
        v[i].preco = 0.0;
        v[i].genero = (char *)"Teste";
    }
    return v;
}

/* tipo: 0 = crescente, 1 = decrescente, 2 = aleatoria (indices 0..n-1) */
static void geraOrdemTeste(int *ord, int n, int tipo, ull seed)
{
    int i;

    for (i = 0; i < n; i++) ord[i] = tipo == 1 ? n - 1 - i : i;
    if (tipo == 2) {
        ull estado = seed;

        for (i = n; i > 1; i--) {
            int j = (int)(proximoAleatorio(&estado) % (ull)i), t = ord[i - 1];

            ord[i - 1] = ord[j];
            ord[j] = t;
        }
    }
}

static int testeVazia(const OpsArvore *ops)
{
    void *arv = ops->cria();
    long long k;
    int ok = 1;

    EXIGE(ops->tamanho(arv) == 0 && ops->altura(arv) == 0, "arvore vazia: tamanho/altura");
    EXIGE(ops->busca(arv, 1, NULL) == NULL, "busca em arvore vazia");
    EXIGE(ops->removeChave(arv, 1, NULL) == NULL, "remocao em arvore vazia");
    EXIGE(!ops->raiz(arv, &k), "arvore vazia nao tem raiz");
    EXIGE(validaOuMostra(ops, arv), "arvore vazia valida");
fim:
    ops->libera(arv);
    return ok;
}

static int testeUnicoEDuplicado(const OpsArvore *ops)
{
    Jogo *j = criaJogosTeste(1);
    void *arv = ops->cria();
    long long k = 0;
    int ok = 1;

    EXIGE(ops->insere(arv, &j[0], NULL) == 1, "inserir um elemento");
    EXIGE(ops->tamanho(arv) == 1 && ops->altura(arv) == 1, "tamanho 1, altura 1");
    EXIGE(ops->raiz(arv, &k) && k == 1, "raiz e a chave 1");
    EXIGE(ops->busca(arv, 1, NULL) == &j[0], "busca do unico elemento");
    EXIGE(validaOuMostra(ops, arv), "valida com 1 elemento");
    EXIGE(ops->insere(arv, &j[0], NULL) == 0, "duplicado deve ser recusado");
    EXIGE(ops->tamanho(arv) == 1, "duplicado nao altera o tamanho");
    EXIGE(ops->removeChave(arv, 99, NULL) == NULL, "remover inexistente");
    EXIGE(ops->removeChave(arv, 1, NULL) == &j[0], "remover o unico elemento");
    EXIGE(ops->tamanho(arv) == 0 && ops->altura(arv) == 0, "volta a ficar vazia");
    EXIGE(validaOuMostra(ops, arv), "valida apos esvaziar");
fim:
    ops->libera(arv);
    free(j);
    return ok;
}

/* Insere N chaves na ordem pedida validando a cada insercao. */
static int testeInsercao(const OpsArvore *ops, int tipo)
{
    enum { N = 1000 };
    Jogo *j = criaJogosTeste(N);
    int *ord = (int *)xmalloc(N * sizeof(*ord));
    void *arv = ops->cria();
    int i, ok = 1;

    geraOrdemTeste(ord, N, tipo, 12345);
    for (i = 0; i < N; i++) {
        EXIGE(ops->insere(arv, &j[ord[i]], NULL) == 1, "insercao de chave nova");
        EXIGE(validaOuMostra(ops, arv), "propriedades apos insercao");
    }
    EXIGE(ops->tamanho(arv) == N, "tamanho final = 1000");
    EXIGE(ops->altura(arv) <= limiteAlturaTeste(N), "altura dentro do limite logaritmico");
    for (i = 1; i <= N; i++)
        EXIGE(ops->busca(arv, i, NULL) == &j[i - 1], "busca de chave existente");
    EXIGE(ops->busca(arv, 0, NULL) == NULL && ops->busca(arv, N + 1, NULL) == NULL &&
              ops->busca(arv, -5, NULL) == NULL,
          "busca de chave inexistente");
    for (i = 0; i < N; i++)
        EXIGE(ops->insere(arv, &j[i], NULL) == 0, "reinsercao recusada");
    EXIGE(ops->tamanho(arv) == N, "reinsercoes nao alteram o tamanho");
fim:
    ops->libera(arv);
    free(ord);
    free(j);
    return ok;
}

/* Monta 1..N em ordem crescente e remove todas na ordem pedida, validando. */
static int testeRemocao(const OpsArvore *ops, int tipo)
{
    enum { N = 1000 };
    Jogo *j = criaJogosTeste(N);
    int *ord = (int *)xmalloc(N * sizeof(*ord));
    void *arv = ops->cria();
    int i, ok = 1;

    for (i = 0; i < N; i++) ops->insere(arv, &j[i], NULL);
    geraOrdemTeste(ord, N, tipo, 777);
    for (i = 0; i < N; i++) {
        EXIGE(ops->removeChave(arv, ord[i] + 1, NULL) == &j[ord[i]], "remocao devolve o jogo");
        EXIGE(ops->busca(arv, ord[i] + 1, NULL) == NULL, "removido nao e mais encontrado");
        EXIGE(ops->tamanho(arv) == (size_t)(N - 1 - i), "tamanho apos remocao");
        EXIGE(validaOuMostra(ops, arv), "propriedades apos remocao");
        EXIGE(ops->removeChave(arv, ord[i] + 1, NULL) == NULL, "remocao repetida falha");
    }
    EXIGE(ops->tamanho(arv) == 0 && ops->altura(arv) == 0, "arvore vazia no fim");
fim:
    ops->libera(arv);
    free(ord);
    free(j);
    return ok;
}

/* Remove varias vezes a propria raiz (inclui nos com 0, 1 e 2 filhos). */
static int testeRemoveRaiz(const OpsArvore *ops)
{
    enum { N = 1000 };
    Jogo *j = criaJogosTeste(N);
    int *ord = (int *)xmalloc(N * sizeof(*ord));
    void *arv = ops->cria();
    long long k;
    int i, ok = 1;

    geraOrdemTeste(ord, N, 2, 99);
    for (i = 0; i < N; i++) ops->insere(arv, &j[ord[i]], NULL);
    for (i = 0; i < N; i++) {
        EXIGE(ops->raiz(arv, &k), "existe raiz");
        EXIGE(ops->removeChave(arv, k, NULL) != NULL, "remover a raiz");
        EXIGE(validaOuMostra(ops, arv), "propriedades apos remover a raiz");
    }
    EXIGE(ops->tamanho(arv) == 0, "vazia apos remover todas as raizes");
fim:
    ops->libera(arv);
    free(ord);
    free(j);
    return ok;
}

/* Operacoes aleatorias misturadas, conferidas contra um vetor "gabarito". */
static int testeMisto(const OpsArvore *ops)
{
    enum { N = 400, OPS = 6000 };
    Jogo *j = criaJogosTeste(N);
    char *presente = (char *)calloc(N, 1);
    void *arv = ops->cria();
    ull estado = 2024;
    size_t esperado = 0;
    int i, ok = 1;

    for (i = 0; i < OPS; i++) {
        int id = (int)(proximoAleatorio(&estado) % N);
        int op = (int)(proximoAleatorio(&estado) % 3);

        if (op == 0) { /* inserir */
            EXIGE(ops->insere(arv, &j[id], NULL) == (presente[id] ? 0 : 1), "insercao coerente");
            if (!presente[id]) esperado++;
            presente[id] = 1;
        } else if (op == 1) { /* remover */
            Jogo *r = ops->removeChave(arv, id + 1, NULL);

            EXIGE((r != NULL) == (presente[id] != 0), "remocao coerente");
            if (presente[id]) esperado--;
            presente[id] = 0;
        } else { /* buscar */
            EXIGE((ops->busca(arv, id + 1, NULL) != NULL) == (presente[id] != 0), "busca coerente");
        }
        EXIGE(ops->tamanho(arv) == esperado, "tamanho coerente");
        EXIGE(validaOuMostra(ops, arv), "propriedades apos operacao mista");
    }
fim:
    ops->libera(arv);
    free(presente);
    free(j);
    return ok;
}

typedef struct { long long anterior; int ordenado; ull n; } CtxOrdem;

static void visitaOrdem(const Jogo *j, void *c)
{
    CtxOrdem *x = (CtxOrdem *)c;

    if (x->n && j->app_id <= x->anterior) x->ordenado = 0;
    x->anterior = j->app_id;
    x->n++;
}

static int testePercursos(const OpsArvore *ops)
{
    enum { N = 500 };
    Jogo *j = criaJogosTeste(N);
    int *ord = (int *)xmalloc(N * sizeof(*ord));
    void *arv = ops->cria();
    CtxOrdem c;
    int i, o, ok = 1;

    geraOrdemTeste(ord, N, 2, 5);
    for (i = 0; i < N; i++) ops->insere(arv, &j[ord[i]], NULL);
    for (o = 0; o < 3; o++) {
        c.anterior = 0;
        c.ordenado = 1;
        c.n = 0;
        ops->percorre(arv, o, visitaOrdem, &c);
        EXIGE(c.n == N, "percurso visita todos os nos");
        if (o == 0) EXIGE(c.ordenado, "percurso em ordem sai crescente por AppID");
    }
fim:
    ops->libera(arv);
    free(ord);
    free(j);
    return ok;
}

static int rodaTeste(const char *nome, int resultado)
{
    printf("  [%s] %s\n", resultado ? "OK" : "FALHA", nome);
    return resultado;
}

static int testaTudo(const OpsArvore *ops)
{
    int ok = 1;

    printf("Testes da %s\n", ops->nome);
    ok &= rodaTeste("leitor de CSV (virgula em aspas, aspas escapadas, delimitador |)", testaCSV());
    ok &= rodaTeste("arvore vazia", testeVazia(ops));
    ok &= rodaTeste("um elemento e chave duplicada", testeUnicoEDuplicado(ops));
    ok &= rodaTeste("insercao crescente (1000, valida a cada passo)", testeInsercao(ops, 0));
    ok &= rodaTeste("insercao decrescente (1000)", testeInsercao(ops, 1));
    ok &= rodaTeste("insercao aleatoria (1000)", testeInsercao(ops, 2));
    ok &= rodaTeste("remocao em ordem crescente", testeRemocao(ops, 0));
    ok &= rodaTeste("remocao em ordem decrescente", testeRemocao(ops, 1));
    ok &= rodaTeste("remocao em ordem aleatoria", testeRemocao(ops, 2));
    ok &= rodaTeste("remocao repetida da raiz", testeRemoveRaiz(ops));
    ok &= rodaTeste("operacoes mistas aleatorias (6000)", testeMisto(ops));
    ok &= rodaTeste("percursos (em ordem sai ordenado)", testePercursos(ops));
    printf(ok ? "Testes: OK\n" : "Testes: HOUVE FALHAS\n");
    return ok;
}

#endif
