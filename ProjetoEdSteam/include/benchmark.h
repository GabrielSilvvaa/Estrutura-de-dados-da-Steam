/*
 * benchmark.h - benchmark generico (mesmo codigo para AVL e Rubro-Negra).
 * Mesmos registros, mesmas ordens de insercao (seed fixa), mesmas consultas.
 * Cenarios de insercao: mediana (recursiva), crescente, decrescente, aleatorio.
 * Apos inserir: busca de todas as chaves, busca de chaves inexistentes e
 * remocao de todas as chaves em ordem aleatoria. Mede tempo, comparacoes,
 * rotacoes e altura. Nada e impresso dentro da regiao cronometrada.
 */
#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "arvore_ops.h"
#include "tempo.h"

typedef struct {
    ull compIns, compBusca, compInex, compRem, rotIns, rotRem;
    double msIns, msBusca, msInex, msRem;
    int altura;
} Medida;

static void embaralha(Jogo **vetor, size_t n, ull seed)
{
    ull estado = seed;
    size_t i;

    for (i = n; i > 1; i--) {
        size_t j = (size_t)(proximoAleatorio(&estado) % i);
        Jogo *tmp = vetor[i - 1];

        vetor[i - 1] = vetor[j];
        vetor[j] = tmp;
    }
}

/* Ordena por AppID; empate resolvido pela posicao (ordem estavel). */
static int comparaJogo(const void *a, const void *b)
{
    Jogo *ja = *(Jogo *const *)a;
    Jogo *jb = *(Jogo *const *)b;

    if (ja->app_id != jb->app_id) return ja->app_id > jb->app_id ? 1 : -1;
    return (ja > jb) - (ja < jb);
}

/* Cenario "mediana": insere a mediana, depois as medianas das metades... */
static void preencheMediana(Jogo **v, size_t lo, size_t hi, Jogo **ordem, size_t *pos)
{
    size_t meio;

    if (lo >= hi) return;
    meio = lo + (hi - lo) / 2;
    ordem[(*pos)++] = v[meio];
    preencheMediana(v, lo, meio, ordem, pos);
    preencheMediana(v, meio + 1, hi, ordem, pos);
}

static void erroBench(const OpsArvore *ops, const char *msg)
{
    fprintf(stderr, "Erro de consistencia no benchmark da %s: %s\n", ops->nome, msg);
    exit(1);
}

static void executaCenario(const OpsArvore *ops, Jogo **base, size_t n,
                           const long long *ausentes, size_t nAus, int cenario,
                           ull seed, Medida *m)
{
    Jogo **ordem = (Jogo **)xmalloc(n * sizeof(*ordem));
    Jogo **remocao = (Jogo **)xmalloc(n * sizeof(*remocao));
    void *arv;
    char msg[160];
    size_t i;
    double t0;
    ull rot0;

    memset(m, 0, sizeof(*m));
    memcpy(ordem, base, n * sizeof(*ordem));
    if (cenario == 0) {
        size_t pos = 0;

        preencheMediana(base, 0, n, ordem, &pos);
    } else if (cenario == 2) {
        for (i = 0; i < n; i++) ordem[i] = base[n - 1 - i];
    } else if (cenario == 3) {
        embaralha(ordem, n, seed);
    }
    memcpy(remocao, base, n * sizeof(*remocao));
    embaralha(remocao, n, seed ^ 0xA5A5A5A5A5A5A5A5ULL);

    arv = ops->cria();

    t0 = agoraMs();
    for (i = 0; i < n; i++) ops->insere(arv, ordem[i], &m->compIns);
    m->msIns = agoraMs() - t0;
    m->rotIns = ops->rotacoes(arv);
    m->altura = ops->altura(arv);

    if (ops->tamanho(arv) != n) erroBench(ops, "quantidade apos insercao");
    if (!ops->valida(arv, msg, sizeof(msg))) erroBench(ops, msg);

    t0 = agoraMs();
    for (i = 0; i < n; i++)
        if (!ops->busca(arv, base[i]->app_id, &m->compBusca)) erroBench(ops, "chave existente nao achada");
    m->msBusca = agoraMs() - t0;

    t0 = agoraMs();
    for (i = 0; i < nAus; i++)
        if (ops->busca(arv, ausentes[i], &m->compInex)) erroBench(ops, "chave inexistente achada");
    m->msInex = agoraMs() - t0;

    rot0 = ops->rotacoes(arv);
    t0 = agoraMs();
    for (i = 0; i < n; i++)
        if (!ops->removeChave(arv, remocao[i]->app_id, &m->compRem)) erroBench(ops, "remocao falhou");
    m->msRem = agoraMs() - t0;
    m->rotRem = ops->rotacoes(arv) - rot0;

    if (ops->tamanho(arv) != 0) erroBench(ops, "arvore nao ficou vazia");
    ops->libera(arv);
    free(ordem);
    free(remocao);
}

static void executaBenchmark(const OpsArvore *ops, Jogo *jogos, size_t total, int reps,
                             ull seed, const char *diretorio)
{
    static const char *nomes[4] = {"mediana", "crescente", "decrescente", "aleatorio"};
    Jogo **ordenado = (Jogo **)xmalloc((total ? total : 1) * sizeof(*ordenado));
    size_t unicos = 0, tamanhos[4], qtd = 0, candidatos[4], i, c, r;
    FILE *csv;
    char arquivo[1024];

    for (i = 0; i < total; i++) ordenado[i] = &jogos[i];
    qsort(ordenado, total, sizeof(*ordenado), comparaJogo);
    /* Mantem so a primeira ocorrencia de cada AppID. */
    for (i = 0; i < total; i++)
        if (!unicos || ordenado[i]->app_id != ordenado[unicos - 1]->app_id)
            ordenado[unicos++] = ordenado[i];
    if (!unicos) {
        printf("Nenhum registro para o benchmark.\n");
        free(ordenado);
        return;
    }

    candidatos[0] = 1000;
    candidatos[1] = 5000;
    candidatos[2] = 10000;
    candidatos[3] = unicos;
    for (i = 0; i < 4; i++) {
        size_t cand = candidatos[i] <= unicos ? candidatos[i] : unicos;

        if (!qtd || cand != tamanhos[qtd - 1]) tamanhos[qtd++] = cand;
    }

    if (!criaDiretorio(diretorio)) {
        fprintf(stderr, "Erro ao criar a pasta '%s'.\n", diretorio);
        free(ordenado);
        return;
    }
    snprintf(arquivo, sizeof(arquivo), "%s/resultados_%s.csv", diretorio, ops->prefixo);
    csv = fopen(arquivo, "w");
    if (!csv) {
        fprintf(stderr, "Erro ao criar '%s'.\n", arquivo);
        free(ordenado);
        return;
    }
    fprintf(csv, "arvore,n,cenario,repeticoes,altura,"
                 "insercao_comp,insercao_ms,insercao_rotacoes,"
                 "busca_comp,busca_ms,busca_inexistente_comp,busca_inexistente_ms,"
                 "remocao_comp,remocao_ms,remocao_rotacoes\n");

    printf("Benchmark da %s\n", ops->nome);
    printf("  registros=%llu  chaves_unicas=%llu  repeticoes=%d  seed=%llu\n",
           (ull)total, (ull)unicos, reps, seed);

    for (i = 0; i < qtd; i++) {
        size_t n = tamanhos[i], nAus = 0, k;
        Jogo **base = (Jogo **)xmalloc(n * sizeof(*base));
        long long *ausentes = (long long *)xmalloc(n * sizeof(*ausentes));

        memcpy(base, ordenado, n * sizeof(*base));
        /* Chaves que NAO estao na amostra: o "buraco" logo apos cada AppID. */
        for (k = 0; k + 1 < n && nAus < n; k++)
            if (base[k]->app_id + 1 < base[k + 1]->app_id)
                ausentes[nAus++] = base[k]->app_id + 1;

        for (c = 0; c < 4; c++) {
            double somaAlt = 0, mi = 0, mb = 0, mx = 0, mr = 0;
            double ci = 0, cb = 0, cx = 0, cr = 0, ri = 0, rr = 0;

            for (r = 0; r < (size_t)reps; r++) {
                Medida m;

                executaCenario(ops, base, n, ausentes, nAus, (int)c,
                               seed + (r + 1) * 0x9E3779B97F4A7C15ULL, &m);
                somaAlt += m.altura;
                mi += m.msIns; mb += m.msBusca; mx += m.msInex; mr += m.msRem;
                ci += (double)m.compIns; cb += (double)m.compBusca;
                cx += (double)m.compInex; cr += (double)m.compRem;
                ri += (double)m.rotIns; rr += (double)m.rotRem;
            }
            fprintf(csv, "%s,%llu,%s,%d,%.1f,%.1f,%.6f,%.1f,%.1f,%.6f,%.1f,%.6f,%.1f,%.6f,%.1f\n",
                    ops->prefixo, (ull)n, nomes[c], reps, somaAlt / reps,
                    ci / reps, mi / reps, ri / reps, cb / reps, mb / reps,
                    cx / reps, mx / reps, cr / reps, mr / reps, rr / reps);
            printf("  n=%-7llu %-11s alt=%5.1f  ins=%9.3f ms (rot %9.1f)  busca=%8.3f ms  rem=%9.3f ms (rot %9.1f)\n",
                   (ull)n, nomes[c], somaAlt / reps, mi / reps, ri / reps, mb / reps,
                   mr / reps, rr / reps);
        }
        free(base);
        free(ausentes);
    }
    fclose(csv);
    free(ordenado);
    printf("Arquivo gerado: %s\n", arquivo);
}

#endif
