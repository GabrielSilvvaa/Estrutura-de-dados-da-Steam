Todos os Jogos da Steam.

Entrega 1 de trabalho da disciplina Estruturas de Dados II — UNICID Prof. Cid Rodrigues de Andrade

👥 Integrantes do Grupo

Nome completo	RA

Gabriel Vieira da Silva - 43756581

Erick Luiz de Andrade Carrera - 44324499

Caio Perez Martins - 42646961
	
	
________________________________________
1. Dataset
1.1 Descrição
Neste trabalho usaremos a base do Kaggle para ver todos os jogos da Steam para pc, independente de genero, criador etc. Usaremos a base CSV, e o numero entre 100k de linhas de dados.

1.2 Fonte

[https://www.fifa.com/pt](https://www.kaggle.com/datasets/fronkongames/steam-games-dataset)

1.3 Estrutura dos dados

IDjogo (int)

Nome (String);

Data_lancamento (date)
 
Propriet_estimados (int);

pico_CCU (float);

Idade_Exigida (int);

Preco (float)

Contagem_DLC (int)

Sobre_Jogo (String)

Idiomas (String)

Exemplo:

- id (int)
- 
- nome (string)
- 
- valor (float)
- 
1.4 Justificativa da escolha
  
Utilizamos esta base de dados por gostarmos de jogos e seria algo mais compreensivel de explicar a fazer. 
Claramente a base de dados pode haver duplicatas e por isso optamos por escolher este ponto, por enfrentar um desafio.

________________________________________
2. Estrutura(s) de Árvore Escolhida(s)

2.1 Estrutura(s)
   
AVL e Rubro-Negra.

2.2 Justificativa técnica

Por que essas estruturas foram escolhidas para este dataset e problema? Considere complexidade, balanceamento, caso de uso.

As árvores AVL e Rubro-Negras foram escolhidas por permitirem buscas, inserções e remoções eficientes, com complexidade. A AVL mantém um balanceamento mais rigoroso, enquanto a Rubro-Negra costuma exigir menos ajustes. Ai podemos comparar o desempenho das duas estruturas ao organizar e consultar os mais de 125 mil jogos do dataset da Steam.

2.3 Operações implementadas (Para Entrega 2)

	Inserção
	
	Remoção
	
	Busca
	
	Percursos (pré-ordem, em ordem, pós-ordem)
	
	Balanceamento (se aplicável)
	
	Outra: ______
2.4 Complexidade

Tabela com a complexidade assintótica (Big-O) teórica de cada operação implementada, no melhor, médio e pior caso.

Operação	Melhor caso	Caso médio	Pior caso

Inserção	

Busca		

Remoção		

________________________________________

3. Plano de Testes

3.1 Objetivo dos testes

O que o grupo pretende validar (corretude, desempenho, comportamento em casos extremos, etc.).

3.2 Cenários de teste

#	Cenário	Entrada	Resultado esperado	Status

1				☐

2				☐

3				☐

3.3 Casos extremos (edge cases)

Liste casos como: árvore vazia, único elemento, dados duplicados, dados em ordem crescente/decrescente (pior caso para BST), volume máximo do 
dataset, etc.

3.4 Testes de desempenho (Para Entrega 2)

Descreva como o grupo mediu tempo de execução e/ou uso de memória, e com quais tamanhos de entrada (ex: 100, 1.000, 10.000 registros).

3.5 Resultados obtidos (Para Entrega 2)

Resuma os resultados (tabelas, gráficos ou links para arquivos de saída na pasta /resultados) e compare-os com a complexidade assintótica (Big-O) teórica.

________________________________________

4. Como Executar
   
4.1 Pré-requisitos (Para Entrega 2)

Linguagem, versão e dependências necessárias.

4.2 Instruções (Para Entrega 2)

# Exemplo

git clone <link-do-repositorio>

cd <pasta>

# comandos de compilação/execução

4.3 Estrutura do repositório (já com pastas para a Entrega 2)

/src         → código-fonte

/dataset     → dataset utilizado

/testes      → scripts e casos de teste

/resultados  → saídas e relatórios de desempenho

README.md

________________________________________

5. Referências

Bibliografia, artigos ou materiais consultados.
