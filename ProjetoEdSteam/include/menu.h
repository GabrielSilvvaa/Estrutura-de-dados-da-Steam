/*
 * menu.h - interface do programa (menu interativo + opcoes de linha de comando).
 * Generica: funciona com qualquer arvore descrita por OpsArvore.
 * Sem argumentos abre o MENU (ideal para Code::Blocks e duplo clique).
 */
#ifndef MENU_H
#define MENU_H

#include "arvore_ops.h"
#include "csv.h"
#include "tempo.h"
#include "testes.h"
#include "benchmark.h"

typedef struct {
    const OpsArvore *ops;
    void *arv;
    Jogo *jogos;          /* vetor lido do CSV (dono dos registros) */
    size_t total;
    Jogo **extras;        /* jogos inseridos manualmente pelo menu */
    size_t nExtras, capExtras;
    char caminho[512];
    int carregado;
    int reps;
    ull seed;
    const char *diretorio;
} Estado;

/* ------------------------------ entrada ----------------------------------- */

static int lerTexto(const char *pergunta, char *buf, size_t tam)
{
    if (pergunta) printf("%s", pergunta);
    fflush(stdout);
    if (!fgets(buf, (int)tam, stdin)) return 0;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 1;
}

/* Retorna 1 se leu um inteiro valido; 0 caso contrario (ou fim da entrada). */
static int lerInteiro(const char *pergunta, long long *valor)
{
    char buf[64], *fim;

    if (!lerTexto(pergunta, buf, sizeof(buf))) return 0;
    errno = 0;
    *valor = strtoll(buf, &fim, 10);
    return fim != buf && !*fim && errno != ERANGE;
}

static void imprimeJogo(const Jogo *j)
{
    printf("  AppID : %lld\n  Nome  : %s\n  Preco : %.2f\n  Genero: %s\n",
           j->app_id, j->nome, j->preco, j->genero);
}

/* --------------------------- estado do programa --------------------------- */

static void liberaExtras(Estado *e)
{
    size_t i;

    for (i = 0; i < e->nExtras; i++) {
        free(e->extras[i]->nome);
        free(e->extras[i]->genero);
        free(e->extras[i]);
    }
    free(e->extras);
    e->extras = NULL;
    e->nExtras = e->capExtras = 0;
}

static void estadoInicia(Estado *e, const OpsArvore *ops)
{
    memset(e, 0, sizeof(*e));
    e->ops = ops;
    e->arv = ops->cria();
    e->reps = 10;
    e->seed = 42;
    e->diretorio = "resultados";
}

static void estadoLibera(Estado *e)
{
    e->ops->libera(e->arv);     /* 1o a arvore (so os nos) ... */
    liberaExtras(e);            /* ... depois os registros que ela apontava */
    liberaJogos(e->jogos, e->total);
    e->arv = NULL;
    e->jogos = NULL;
    e->total = 0;
}

/* Le o CSV e monta a arvore com todos os jogos. Retorna 1 se deu certo. */
static int carregaEMonta(Estado *e)
{
    Jogo *novos = NULL;
    size_t qtd = 0, ruins = 0, i, duplicados = 0;
    double t0;
    void *nova;

    printf("Lendo '%s'...\n", e->caminho);
    t0 = agoraMs();
    if (!carregaCSV(e->caminho, &novos, &qtd, &ruins)) return 0;
    printf("  registros lidos: %llu (linhas ignoradas: %llu) em %.1f ms\n",
           (ull)qtd, (ull)ruins, agoraMs() - t0);

    estadoLibera(e);
    e->jogos = novos;
    e->total = qtd;
    nova = e->ops->cria();
    e->arv = nova;

    t0 = agoraMs();
    for (i = 0; i < qtd; i++)
        if (!e->ops->insere(e->arv, &e->jogos[i], NULL)) duplicados++;
    printf("  %s montada: %llu jogos, altura %d, %llu duplicados ignorados, %.1f ms\n",
           e->ops->nome, (ull)e->ops->tamanho(e->arv), e->ops->altura(e->arv),
           (ull)duplicados, agoraMs() - t0);
    e->carregado = 1;
    return 1;
}

static int garanteCSV(Estado *e)
{
    if (e->carregado) return 1;
    if (!e->caminho[0]) {
        const char *achado = localizaCSV();

        if (!achado) {
            printf("CSV nao encontrado. Use a opcao 1 e informe o caminho.\n");
            return 0;
        }
        strncpy(e->caminho, achado, sizeof(e->caminho) - 1);
    }
    return carregaEMonta(e);
}

/* ------------------------------- opcoes ----------------------------------- */

static void opCarregar(Estado *e)
{
    char buf[512];
    const char *padrao = e->caminho[0] ? e->caminho : localizaCSV();

    printf("Caminho do CSV (ENTER = %s): ", padrao ? padrao : "nenhum encontrado");
    if (!lerTexto(NULL, buf, sizeof(buf))) return;
    if (buf[0]) strncpy(e->caminho, buf, sizeof(e->caminho) - 1);
    else if (padrao) strncpy(e->caminho, padrao, sizeof(e->caminho) - 1);
    else {
        printf("Informe um caminho.\n");
        return;
    }
    e->caminho[sizeof(e->caminho) - 1] = '\0';
    carregaEMonta(e);
}

static void opInserir(Estado *e)
{
    long long id;
    char nome[256], genero[128], preco[32];
    Jogo *j;

    if (!lerInteiro("AppID (inteiro): ", &id)) { printf("AppID invalido.\n"); return; }
    if (e->ops->busca(e->arv, id, NULL)) { printf("Ja existe um jogo com o AppID %lld.\n", id); return; }
    if (!lerTexto("Nome: ", nome, sizeof(nome))) return;
    if (!lerTexto("Preco (ex.: 9.99): ", preco, sizeof(preco))) return;
    if (!lerTexto("Genero: ", genero, sizeof(genero))) return;

    j = (Jogo *)xmalloc(sizeof(*j));
    j->app_id = id;
    j->nome = duplica(nome);
    j->preco = strtod(preco, NULL);
    j->genero = duplica(genero);
    if (!e->ops->insere(e->arv, j, NULL)) { /* nao deve ocorrer: ja checamos */
        free(j->nome); free(j->genero); free(j);
        printf("Insercao recusada (AppID duplicado).\n");
        return;
    }
    if (e->nExtras == e->capExtras) {
        e->capExtras = e->capExtras ? e->capExtras * 2 : 8;
        e->extras = (Jogo **)xrealloc(e->extras, e->capExtras * sizeof(*e->extras));
    }
    e->extras[e->nExtras++] = j;
    printf("Inserido. Jogos na arvore: %llu\n", (ull)e->ops->tamanho(e->arv));
}

static void opBuscar(Estado *e)
{
    long long id;
    ull comp = 0;
    Jogo *j;

    if (!lerInteiro("AppID: ", &id)) { printf("AppID invalido.\n"); return; }
    j = e->ops->busca(e->arv, id, &comp);
    if (j) {
        printf("Encontrado (%llu comparacoes):\n", comp);
        imprimeJogo(j);
    } else {
        printf("AppID %lld nao encontrado (%llu comparacoes).\n", id, comp);
    }
}

static void opRemover(Estado *e)
{
    long long id;
    ull comp = 0;
    Jogo *j;

    if (!lerInteiro("AppID a remover: ", &id)) { printf("AppID invalido.\n"); return; }
    j = e->ops->removeChave(e->arv, id, &comp);
    if (j) printf("Removido: %s (%llu comparacoes). Restam %llu jogos.\n",
                  j->nome, comp, (ull)e->ops->tamanho(e->arv));
    else printf("AppID %lld nao existe na arvore.\n", id);
}

typedef struct { ull visitados, limite; } CtxImprime;

static void visitaImprime(const Jogo *j, void *c)
{
    CtxImprime *x = (CtxImprime *)c;

    if (x->visitados < x->limite)
        printf("  %lld | %s | %.2f | %s\n", j->app_id, j->nome, j->preco, j->genero);
    x->visitados++;
}

static void opPercursos(Estado *e)
{
    static const char *nomes[3] = {"Em ordem", "Pre-ordem", "Pos-ordem"};
    long long ordem = 0, limite = 20;
    CtxImprime ctx;

    printf("1 = em ordem  2 = pre-ordem  3 = pos-ordem\n");
    if (!lerInteiro("Percurso: ", &ordem) || ordem < 1 || ordem > 3) { printf("Opcao invalida.\n"); return; }
    if (!lerInteiro("Quantos jogos mostrar (padrao 20): ", &limite) || limite < 1) limite = 20;
    ctx.visitados = 0;
    ctx.limite = (ull)limite;
    printf("%s (primeiros %lld):\n", nomes[ordem - 1], limite);
    e->ops->percorre(e->arv, (int)ordem - 1, visitaImprime, &ctx);
    printf("Total de nos percorridos: %llu\n", ctx.visitados);
}

static void opInfo(Estado *e)
{
    char extra[300];

    printf("Estrutura : %s\n", e->ops->nome);
    printf("Jogos     : %llu\n", (ull)e->ops->tamanho(e->arv));
    printf("Altura    : %d (em nos)\n", e->ops->altura(e->arv));
    printf("Rotacoes  : %llu (desde a criacao da arvore)\n", e->ops->rotacoes(e->arv));
    e->ops->info(e->arv, extra, sizeof(extra));
    printf("%s\n", extra);
}

static void opValidar(Estado *e)
{
    char msg[160];
    int ok = e->ops->valida(e->arv, msg, sizeof(msg));

    printf("%s: %s\n", ok ? "VALIDA" : "INVALIDA", msg);
}

static void opBenchmark(Estado *e)
{
    if (!garanteCSV(e)) return;
    executaBenchmark(e->ops, e->jogos, e->total, e->reps, e->seed, e->diretorio);
}

/* Demonstracao: 20 primeiros jogos, busca e remocao (como no trabalho original). */
static void demonstra(const OpsArvore *ops, Jogo *jogos, size_t total)
{
    void *arv = ops->cria();
    size_t limite = total < 20 ? total : 20, i;
    char msg[160];
    Jogo *j;
    CtxImprime ctx;

    if (!limite) { printf("Nada para demonstrar.\n"); ops->libera(arv); return; }
    for (i = 0; i < limite; i++) ops->insere(arv, &jogos[i], NULL);
    printf("\n%s de demonstracao: %llu jogos, altura %d, %llu rotacoes\n", ops->nome,
           (ull)ops->tamanho(arv), ops->altura(arv), ops->rotacoes(arv));
    ctx.visitados = 0;
    ctx.limite = limite;
    printf("Percurso em ordem:\n");
    ops->percorre(arv, 0, visitaImprime, &ctx);

    j = ops->busca(arv, jogos[0].app_id, NULL);
    printf("Busca pelo AppID %lld: %s\n", jogos[0].app_id, j ? j->nome : "nao encontrado");
    j = ops->removeChave(arv, jogos[0].app_id, NULL);
    printf("Remocao do AppID %lld: %s\n", jogos[0].app_id, j ? "ok" : "nao encontrado");
    printf("Depois da remocao: %llu jogos, altura %d\n", (ull)ops->tamanho(arv), ops->altura(arv));
    ops->valida(arv, msg, sizeof(msg));
    printf("Validacao: %s\n", msg);
    ops->libera(arv);
}

static void sobre(const OpsArvore *ops)
{
    printf("\nTrabalho de Estruturas de Dados II - UNICID\n");
    printf("Prof. Cid Rodrigues de Andrade\n");
    printf("Integrantes: Gabriel Vieira da Silva (43756581),\n");
    printf("             Erick Luiz de Andrade Carrera (44324499),\n");
    printf("             Caio Perez Martins (42646961)\n");
    printf("Estrutura deste programa: %s | chave: AppID | dataset: Steam Games (Kaggle)\n", ops->nome);
}

static void mostraMenu(Estado *e)
{
    printf("\n=========== %s - Jogos da Steam ===========\n", e->ops->nome);
    printf("CSV: %s | jogos na arvore: %llu\n",
           e->carregado ? e->caminho : "nao carregado", (ull)e->ops->tamanho(e->arv));
    printf(" 1 - Carregar CSV e montar a arvore\n");
    printf(" 2 - Inserir um jogo\n");
    printf(" 3 - Buscar jogo pelo AppID\n");
    printf(" 4 - Remover jogo pelo AppID\n");
    printf(" 5 - Exibir percursos\n");
    printf(" 6 - Altura e informacoes de balanceamento\n");
    printf(" 7 - Validar propriedades da arvore\n");
    printf(" 8 - Executar testes\n");
    printf(" 9 - Executar benchmark\n");
    printf("10 - Demonstracao (20 jogos)\n");
    printf("11 - Sobre o projeto\n");
    printf(" 0 - Sair\n");
}

static void ajuda(const char *programa)
{
    printf("Uso: %s [opcoes]   (sem opcoes abre o menu)\n\n", programa);
    printf("  --csv <arquivo>   CSV de entrada\n");
    printf("  --bench           executa o benchmark\n");
    printf("  --reps <n>        repeticoes de cada medicao (padrao: 10)\n");
    printf("  --seed <n>        seed do caso aleatorio (padrao: 42)\n");
    printf("  --out <pasta>     pasta de saida (padrao: resultados)\n");
    printf("  --demo            mostra busca e remocao em 20 jogos\n");
    printf("  --test            roda os testes\n");
    printf("  --help            mostra esta ajuda\n");
}

static int lerNumeroSemSinal(const char *texto, ull *valor)
{
    char *fim;
    ull numero;

    if (!texto || !texto[0] || texto[0] == '-') return 0;
    errno = 0;
    numero = strtoull(texto, &fim, 10);
    if (errno == ERANGE || fim == texto || *fim) return 0;
    *valor = numero;
    return 1;
}

static int programaPrincipal(const OpsArvore *ops, int argc, char **argv)
{
    Estado e;
    int modoDemo = 0, modoTeste = 0, modoBench = 0, i, codigo = 0;
    long long opcao;

    prepararConsole();
    estadoInicia(&e, ops);

    for (i = 1; i < argc; i++) {
        ull valor;

        if (!strcmp(argv[i], "--help")) { ajuda(argv[0]); estadoLibera(&e); return 0; }
        else if (!strcmp(argv[i], "--demo")) modoDemo = 1;
        else if (!strcmp(argv[i], "--test")) modoTeste = 1;
        else if (!strcmp(argv[i], "--bench")) modoBench = 1;
        else if (i + 1 < argc && !strcmp(argv[i], "--csv")) {
            strncpy(e.caminho, argv[++i], sizeof(e.caminho) - 1);
        } else if (i + 1 < argc && !strcmp(argv[i], "--out")) {
            e.diretorio = argv[++i];
        } else if (i + 1 < argc && !strcmp(argv[i], "--reps")) {
            if (!lerNumeroSemSinal(argv[++i], &valor) || valor == 0) {
                fprintf(stderr, "Valor invalido para --reps.\n");
                estadoLibera(&e);
                return 1;
            }
            e.reps = (int)valor;
        } else if (i + 1 < argc && !strcmp(argv[i], "--seed")) {
            if (!lerNumeroSemSinal(argv[++i], &valor)) {
                fprintf(stderr, "Valor invalido para --seed.\n");
                estadoLibera(&e);
                return 1;
            }
            e.seed = valor;
        } else {
            fprintf(stderr, "Opcao desconhecida: %s\n", argv[i]);
            ajuda(argv[0]);
            estadoLibera(&e);
            return 1;
        }
    }

    if (modoTeste) {
        codigo = testaTudo(ops) ? 0 : 1;
        estadoLibera(&e);
        return codigo;
    }
    if (modoDemo || modoBench) {
        if (!e.caminho[0]) {
            const char *achado = localizaCSV();

            if (!achado) { fprintf(stderr, "CSV nao encontrado. Use --csv.\n"); estadoLibera(&e); return 1; }
            strncpy(e.caminho, achado, sizeof(e.caminho) - 1);
        }
        if (!carregaEMonta(&e)) { estadoLibera(&e); return 1; }
        if (modoDemo) demonstra(ops, e.jogos, e.total);
        if (modoBench) executaBenchmark(ops, e.jogos, e.total, e.reps, e.seed, e.diretorio);
        estadoLibera(&e);
        return 0;
    }

    /* Menu interativo. */
    for (;;) {
        mostraMenu(&e);
        if (!lerInteiro("Escolha: ", &opcao)) {
            if (feof(stdin)) break;
            printf("Opcao invalida.\n");
            continue;
        }
        if (opcao == 0) break;
        switch (opcao) {
        case 1: opCarregar(&e); break;
        case 2: opInserir(&e); break;
        case 3: opBuscar(&e); break;
        case 4: opRemover(&e); break;
        case 5: opPercursos(&e); break;
        case 6: opInfo(&e); break;
        case 7: opValidar(&e); break;
        case 8: testaTudo(ops); break;
        case 9: opBenchmark(&e); break;
        case 10:
            if (garanteCSV(&e)) demonstra(ops, e.jogos, e.total);
            break;
        case 11: sobre(ops); break;
        default: printf("Opcao invalida.\n");
        }
    }
    printf("Ate logo!\n");
    estadoLibera(&e);
    return 0;
}

#endif
