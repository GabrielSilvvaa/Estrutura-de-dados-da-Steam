# 🎮 Todos os Jogos da Steam

**Entrega 1 — Estruturas de Dados II**
UNICID · Prof. Cid Rodrigues de Andrade

## 👥 Integrantes do Grupo

| Nome completo | RA |
|---|---|
| Gabriel Vieira da Silva | 43756581 |
| Erick Luiz de Andrade Carrera | 44324499 |
| Caio Perez Martins | 42646961 |

---

## 1. Dataset

### 1.1 Descrição

Neste trabalho usamos a base do Kaggle com os jogos da Steam para PC, independentemente de gênero, criador etc. O formato é CSV, com mais de 100 mil linhas de dados (cerca de 125 mil jogos).

### 1.2 Fonte

[Steam Games Dataset (Kaggle)](https://www.kaggle.com/datasets/fronkongames/steam-games-dataset)

### 1.3 Estrutura dos dados

| Campo | Tipo | Descrição |
|---|---|---|
| `IDjogo` (AppID) | int | Identificador único do jogo (chave das árvores) |
| `Nome` | String | Nome do jogo |
| `Data_lancamento` | date | Data de lançamento |
| `Propriet_estimados` | int | Número estimado de proprietários |
| `pico_CCU` | float | Pico de jogadores simultâneos |
| `Idade_Exigida` | int | Idade mínima exigida |
| `Preco` | float | Preço |
| `Contagem_DLC` | int | Quantidade de DLCs |
| `Sobre_Jogo` | String | Descrição do jogo |
| `Idiomas` | String | Idiomas suportados |

### 1.4 Justificativa da escolha

Escolhemos esta base por gostarmos de jogos, o que torna o tema mais fácil de entender e de explicar. Além disso, a base pode conter **duplicatas de AppID**, e optamos por ela justamente para enfrentar esse desafio: a inserção recusa chaves repetidas.

---

## 2. Estruturas de Árvore Escolhidas

### 2.1 Estruturas

**Árvore AVL** e **Árvore Rubro-Negra**.

### 2.2 Justificativa técnica

As duas estruturas garantem busca, inserção e remoção em tempo logarítmico, mesmo no pior caso, o que uma BST comum não garante (dados em ordem crescente ou decrescente a degeneram em lista). A AVL mantém um balanceamento mais rigoroso, resultando em árvores mais baixas e buscas um pouco mais rápidas. A Rubro-Negra é mais flexível e costuma exigir menos rotações em inserções e remoções. Assim, podemos comparar o desempenho das duas ao organizar e consultar os mais de 125 mil jogos do dataset.

### 2.3 Operações implementadas

- Inserção (recusa AppID duplicado)
- Remoção (por AppID)
- Busca (por AppID)
- Percursos: pré-ordem, em ordem e pós-ordem
- Balanceamento:
  - **AVL:** rotações simples e duplas, fator de balanceamento
  - **Rubro-Negra:** recoloração e rotações
- Outras: validação das propriedades da árvore, cálculo de altura (e altura negra na rubro-negra), contagem de rotações e de comparações, carga de CSV, testes automáticos e benchmark

### 2.4 Complexidade

| Operação | Melhor caso | Caso médio | Pior caso |
|---|---|---|---|
| Inserção | O(log n) | O(log n) | O(log n) |
| Busca | O(1) (chave na raiz) | O(log n) | O(log n) |
| Remoção | O(log n) | O(log n) | O(log n) |
| Percursos | O(n) | O(n) | O(n) |

---

## 3. Plano de Testes

### 3.1 Objetivo dos testes

Validar a corretude das operações (inserção, busca, remoção e percursos), a manutenção das propriedades de cada árvore após cada operação, o comportamento em casos extremos e o desempenho em diferentes tamanhos e ordens de entrada.

### 3.2 Cenários de teste

Todos rodam com `--test` e são checados automaticamente.

| # | Cenário | Entrada | Resultado esperado | Status |
|---|---|---|---|---|
| 1 | Leitor de CSV | vírgula entre aspas, aspas escapadas, delimitador | campos lidos corretamente | ✓ |
| 2 | Árvore vazia | busca e remoção sem elementos | busca/remoção retornam nada; tamanho e altura 0 | ✓ |
| 3 | Um elemento e duplicado | inserir uma chave duas vezes | duplicado recusado, tamanho 1; remoção deixa a árvore vazia | ✓ |
| 4 | Inserção crescente | 1.000 chaves em ordem crescente | árvore válida a cada passo, altura logarítmica | ✓ |
| 5 | Inserção decrescente | 1.000 chaves em ordem decrescente | idem | ✓ |
| 6 | Inserção aleatória | 1.000 chaves embaralhadas | idem, e todas as chaves são encontradas | ✓ |
| 7 | Remoção crescente | remover todas em ordem crescente | árvore válida a cada passo, termina vazia | ✓ |
| 8 | Remoção decrescente | remover todas em ordem decrescente | idem | ✓ |
| 9 | Remoção aleatória | remover todas em ordem aleatória | idem | ✓ |
| 10 | Remoção repetida da raiz | remover a raiz até esvaziar | propriedades mantidas | ✓ |
| 11 | Operações mistas | 6.000 inserções, buscas e remoções aleatórias | resultado coerente com um controle simples | ✓ |
| 12 | Percursos | pré-ordem, em ordem e pós-ordem | visitam todos os nós; em ordem sai crescente por AppID | ✓ |

### 3.3 Casos extremos (edge cases)

- Árvore vazia (busca, remoção e raiz)
- Um único elemento
- AppID duplicado (inserção recusada, tamanho inalterado)
- Remover chave inexistente e remover a mesma chave duas vezes
- Dados em ordem crescente e decrescente (pior caso de uma BST comum; a árvore balanceada deve manter altura logarítmica)
- Remoção sucessiva da raiz
- Volume máximo do dataset (último tamanho do benchmark = todos os AppIDs únicos)
- Registros com vírgula dentro de aspas no CSV

### 3.4 Testes de desempenho

O benchmark (`--bench`) usa os mesmos registros e as mesmas ordens para as duas árvores. Cada medição é repetida (padrão: 10 vezes, ajustável com `--reps`) e a ordem aleatória usa seed fixa (padrão: 42, ajustável com `--seed`).

- **Tamanhos de entrada:** 1.000, 5.000, 10.000 e o total de AppIDs únicos do dataset
- **Cenários de inserção:** mediana, crescente, decrescente e aleatório
- **O que mede:** tempo em ms (inserção, busca, busca de chave inexistente e remoção), número de comparações, número de rotações e altura final
- **Como mede o tempo:** `QueryPerformanceCounter` no Windows; nada é impresso dentro da região cronometrada
- **Memória:** o projeto não mede uso de memória

### 3.5 Resultados obtidos

Os resultados são gravados em `resultados/resultados_avl.csv` e `resultados/resultados_rubro_negra.csv`.

 Tabela resumo por árvore:

| n | Árvore | Tempo de inserção (ms) | Altura | Rotações |
|---|---|---|---|---|
| 1.000 | AVL |0,088|12,0|700,4|
| 1.000 | Rubro-Negra |0,079 |12,0 |572,1 |
| 5.000 | AVL |0,602 |15,0 |3.457,7 |
| 5.000 | Rubro-Negra |0,457 |15,0 |2.895,5 |
| 10.000 | AVL |1,387 |16,0 |6.906,7 |
| 10.000 | Rubro-Negra |1,098 |16,0 |5.795,2 |
| total | AVL |40,205 |20,2 |87.896,6 |
| total | Rubro-Negra |28,507 |21,0 |73.364,2 |

**Comparação com O(log n):** ao multiplicar *n* por 10, a altura e o número de comparações devem crescer pouco (aproximadamente +3 níveis, já que log₂10 ≈ 3,3). Isso confirma o comportamento logarítmico esperado.

---

## 4. Como Executar

### 4.1 Pré-requisitos

- Linguagem C (padrão C11)
- Compilador GCC (MinGW) ou Code::Blocks no Windows
- Nenhuma biblioteca externa
- Arquivo `steam_games_clean.csv` na pasta `/dataset`

### 4.2 Instruções

```bash
git clone <link-do-repositorio>
cd <pasta>/src

gcc -std=c11 -O2 -o avl avl_main.c
gcc -std=c11 -O2 -o rubro_negra rubro_negra_main.c
```

Execução:

```bash
avl.exe                  # menu interativo
avl.exe --test           # testes
avl.exe --bench          # benchmark, grava em ../resultados
rubro_negra.exe          # menu interativo
rubro_negra.exe --test
rubro_negra.exe --bench
```

### 4.3 Estrutura do repositório

```
/src         → avl.h, avl_main.c, rubro_negra.h, rubro_negra_main.c,
               jogo.h, arvore_ops.h, menu.h, csv.h, tempo.h,
               testes.h, benchmark.h
/dataset     → steam_games_clean.csv
README.md
```

### 4.4 Vídeos dos testes

https://github.com/user-attachments/assets/1ddcce29-c8a9-4852-8b35-c1ba2f1213fb

https://github.com/user-attachments/assets/26231ac8-03cc-41a6-961b-14bd65294f8e

---

## 5. Referências

- CORMEN, T. H. et al. *Algoritmos: Teoria e Prática*. (capítulo de árvores rubro-negras)
- Documentação do dataset Steam Games, na fonte do Kaggle
- Slides das aulas de Estruturas de Dados
- Vídeos de apoio:
  - https://youtu.be/nvTd4uw2wU0?si=NYEB5O1ewWQoULt5
  - https://youtu.be/Y1jSx3lT8r8?si=TTBULtqLyioc9kNg
  - https://youtu.be/aTM2buXQbq8?si=NmNegJ83rZlwOGbr
  - https://youtu.be/yUf1AGSXYzg?si=CmNeyV9NFLK3QqVs
