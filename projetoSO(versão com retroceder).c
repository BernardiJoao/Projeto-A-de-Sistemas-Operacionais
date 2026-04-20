#include <time.h>
#include<stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1000


#define NOVA 1
#define PRONTA 2
#define EXECUTANDO 3
#define TERMINADA 4
#define SUSPENSA 5

#define LIVRE 1
#define OCUPADA 0

#define N_CPUs 5
#define QUANTUM 2



/*================================================================================/
/ Inicialmente, definimos tres structs, uma delas respons�vel pelo armazenamento  /
/ de informacoes acerca da tarefa (TCB), outra para sabermos informacoes sobre as /
/ CPUs que est�o sendo utilizadas no sistema operacional e a ultima responsavel   /
/ pela definicao da fila e seus elementos                                         /
/================================================================================*/

typedef struct
{
    int id_tarefa;
    int estado;
    int contexto;//indica se esta ativa(1) ou nao (0)

    int ingresso; //momento que muda de nova->pronta (entra na fila de prontas)
    int duracao;//duracao da execucao da tarefa
    int termino; // momento de termino
    int espera; //tempo de espera na fila de prontas

    int restante; //tempo restante para acabar a tarefa
    int quantum_restante;
    int prioridade;
    char lista_eventos[256];
    char cor[8];//obter char para cor da tarefa


} TCB;

typedef struct
{
    int ocupado;//flag que indica se a CPU esta sendo utilizada(1) ou nao(0);
    TCB* tarefa_atual; //tarefa que usa a CPU em questao;
    int tempo_desligado; // indica o tempo que o processador ficou inativo

} CPU;

typedef struct
{
  int front;//posicao inicial da fila
  int back;//posicao final da fila
  int size;//tamanho da fila
  TCB **array;//vetor de elementos da fila, declarado como um ponteiro 

} Queue;

typedef struct
{
  int algoritmo_escalonamento;
  int quantum;
  int qtde_cpus; 

} ConfigSistema;

//precisamos ter uma estrutura que armazene o estado atual do sistema para poder retroceder ou avançar conforme o requisitado pelo usuario
typedef struct
{
  int tick;
  TCB tarefas_novas[SIZE];
  Queue* filas_prontas;
  CPU CPUs[N_CPUs];

}EstadoAtual;

/*===================================================================================/
/ para comecar propriamente a implementacao do sistema,precisamos definir o estado   /
/ inicial das tarefas, para que, ao serem devidamente "carregadas", entrarem na fila /
/ de tarefas prontas, que sera simulada por meio da estrutura de dados fila (queue)  /
/===================================================================================*/

/*===================================================================================/
/==========================--Declaracao de funcoes auxiliares--======================/
/===================================================================================*/

void inicializa_tarefas(TCB* tarefas_novas);
void define_pronta(TCB* tarefas_novas, int tick);
void enfileiraProntas(TCB* tarefas_novas, Queue* fila_prontas, int tick);
void salvarEstadoAtual(TCB* tarefas_nova, Queue* fila_prontas, int* tick, CPU* CPUs, int* tick_atual, EstadoAtual* historico);
void execucaoCompleta(TCB* tarefas_novas,int* tick, Queue* filas_prontas, CPU* CPUs,int* tick_atual,int* tick_max);
void execucaoPassoAPasso(TCB* tarefas_novas, int* tick, Queue* fila_prontas, CPU* CPUs, int* tick_atual, int* tick_max);  
void inicializaCPU (CPU* CPUs);
Queue *create (int size);
void destroy (Queue *q);
void enqueue (Queue *q, TCB* tarefa);
TCB* dequeue (Queue *q);
int frontID (Queue *q);
int backID (Queue *q);
int empty (Queue *q);
int full (Queue *q);
void print (Queue *q);
int getsize (Queue *q);
int search (Queue *q, TCB* tarefa);



//===================================Codigo central================================//

int simulacaoSO()
{
    int tick = 0;

    //A inicializacao das tarefas e da fila de pronto, bem como a chamada para
    //elas entrarem no estado "NOVA" deve ocorrer fora do loop geral de execucao, caso contrario � executado sempre
    TCB tarefas_novas[SIZE];
    Queue* fila_prontas = create(SIZE);
    CPU CPUs[N_CPUs]; //vetor que representa os processadores do Sistema Operacional
    char opcao_executacao; //salva a maneira escolhida pelo usuario para executar a simulacao
    EstadoAtual historico[SIZE]; //cria a variável que armazenará o historico mais recente da simulação

    for(int i = 0; i < N_CPUs; i++)
        inicializaCPU(&CPUs[i]);

    inicializa_tarefas(tarefas_novas);


    //while(1)
    {
      int tick_atual = 0;
      int tick_max = tick_atual;

       printf("Como deseja realizar a simulação? Passo-a-passo(a) ou completa(b)?\n");
        scanf(" %c", &opcao_executacao);

       if(opcao_executacao == 'a')
         execucaoPassoAPasso(tarefas_novas, &tick, fila_prontas, CPUs, historico);  
          
       else if (opcao_executacao == 'b')
         execucaoCompleta(tarefas_novas,fila_prontas, &tick,CPUs, &tick_atual, &tick_max); 
       else
       {
         printf("Opção invalida!");
        exit(1);
      }

     
    //}


}
/*===================================================================================/
/=============================--Funcoes auxiliares utilizadas--======================/
/===================================================================================*/

//altera o estado da tarefa para "NOVA" e inicializa seus valores
void inicializa_tarefas(TCB* tarefas_novas)
{
    int i;
    for (i = 0; i < SIZE; i++)
    {
        tarefas_novas[i].estado = NOVA;
        tarefas_novas[i].contexto = 0;
        tarefas_novas[i].id_tarefa = i;

        tarefas_novas[i].duracao = rand() % 10+1;
        tarefas_novas[i].restante = tarefas_novas[i].duracao;
        tarefas_novas[i].ingresso = rand() % 10+1;
        tarefas_novas[i].quantum_restante = QUANTUM;
    }

}

//necessario uma função que salve o estado atual do sistema para ocasioes futuras, se for necessário avançar ou retroceder no sistema
void salvarEstadoAtual(TCB* tarefas_novas, Queue* fila_prontas, int* tick, CPU* CPUs, int* tick_atual, EstadoAtual* historico)
{
 
  historico[tick_atual].tick = *tick;

  for(int i = 0; i < SIZE; i++)
    historico[tick_atual].tarefas_novas[i] = tarefas_novas[i];

  for(int i = 0; i < N_CPUs;i++)
    historico[tick_atual].CPUs[i] = CPUs[i];

    /*======================================================================================================================================================/ 
    /Como a fila_prontas armazena o ENDEREÇO da fila, não é possivel fazer a atribuição de filas_prontas para o histórico diretamente, pois                 /
    /isso ocasionaria apenas na passagem do endereço da fila, e não propriamente ela, o que ocasionaria em dificuldades para retroceder o estado do sistema /
    / visto que se houver alguma alteração no estado atual do sistema, é igualmente alterado na variável "historico".Por isso, usa-se deep copy,            /
    / que faz realmente a copia completa de todos os dados, e não do endereço das informações                                                               /
    /======================================================================================================================================================*/

    historico[tick_autal].fila_prontas = create(SIZE); //cria uma nova fila que será armazenada na estrutura "fila_prontas", de historico

    int front = frontID(fila_prontas);//por meio da função "frontID", armazena em uma variavel auxiliar o valor da frente da fila de tarefas prontas
    int back = backID(fila_prontas);//por meio da função "backID", armazena em uma variavel auxiliar o valor da parte de tras da fila de tarefas prontas

    while(front != back)
    {
      enqueue(historico[i.atual].fila_prontas,fila_prontas->array[front]);
      front = (front + 1) % SIZE;  //fila circular para garantir que sempre está respeitando o limite da fila, usando novamente a posicao inicial caso necessário
    }

}


//A função retrocede, por meio do historico salvo em salvarEstadoAtual, o estado da simulacao para aquele anteriormente registrado
void retrocederEstadoAtual(TCB* tarefas_novas, Queue* fila_prontas, int* tick, CPU* CPUs, EstadoAtual* historico, int tick_atual)
{
  *tick = historico[tick_atual]->tick;

  for(int i = 0; i < SIZE; i++)
    tarefas_novas[i] = historico[tick_atual]->tarefas_novas[i];

  for(int i = 0; i < N_CPUs; i++)
    CPUs[i] = historico[tick_atual]->CPUs[i]; 

  *filas_prontas = historico[tick_atual]->filas_prontas;
  
}

//Caso o momento de ingresso da tarefa seja equivalente ao tick atual, atualizamos seu estado para "PRONTA"
void define_pronta(TCB* tarefas_novas, int tick)
{
    for(int i = 0; i < SIZE; i++ )
    {
        if(tarefas_novas[i].ingresso == tick)
            tarefas_novas[i].estado = PRONTA;
    }

}

void enfileiraProntas(TCB* tarefas_novas, Queue* fila_prontas, int tick)
{
 for(int i = 0; i < SIZE && !full(fila_prontas);i++)
        {
            if(tarefas_novas[i].estado == PRONTA && tarefas_novas[i].ingresso == tick)//garante com que a tarefa esteja pronta e entre apenas uma vez na fila
                enqueue(fila_prontas,&tarefas_novas[i]);//a fila de tarefas prontas armazena somente o id
        }
}

void inicializaCPU (CPU* CPUs)
{
  CPUs->ocupado = LIVRE; 
  CPUs->tempo_desligado = 0;
  CPUs->tarefa_atual = NULL;

}

void execucaoPassoAPasso(TCB* tarefas_novas, int* tick, Queue* fila_prontas, CPU* CPUs, int* tick_atual, int* tick_max)
{
  char atalho;//armazena a opção de ação escolhida pelo usuario

  printf("Qual ação deseja realizar?\n");
  printf("A - Avançar simulação || R = Retroceder simulação || M - Modificar estado\n");
  scanf("%c", &atalho);

  if(atalho == 'A')
  {

    salvarEstadoAtual(tarefas_novas, filas_prontas, &tick, CPUs, &tick_atual);

    tick_atual++;
    tick_max = tick_atual;

    define_pronta(tarefas_novas, tick);//marca as tarefas como prontas

    enfileiraProntas(tarefas_novas, fila_prontas, tick);//adiciona as tarefas com estado "PRONTA" na fila de prontas


        //A funcao abaixo e responsavel por distribuir as tarefas prontas para os CPUs disponiveis, e caso 
        //nao haja tarefas prontas, a CPU e "desligada" e seu tempo nesse estado e contabilizado
    for(int i = 0; i < N_CPUs; i++)
       {
        if(CPUs[i].ocupado == LIVRE && !empty(fila_prontas))
        {
          CPUs[i].ocupado = OCUPADA;
          CPUs[i].tarefa_atual = dequeue(fila_prontas);
          CPUs[i].tarefa_atual->contexto = 1;
          CPUs[i].tarefa_atual->estado = EXECUTANDO;
        }
        if(CPUs[i].ocupado == LIVRE && empty(fila_prontas))
          CPUs[i].tempo_desligado++;
        }

        //Esse loop contabiliza o tempo em que a tarefa esta sendo executada e, caso se encerre o processo,
        //ela e retirada do processador e tem seu tempo de finalizacao marcado

        for(int i = 0; i < N_CPUs; i++)
        {

          if(CPUs[i].ocupado == OCUPADA && CPUs[i].tarefa_atual->estado == EXECUTANDO)
          {
            
           CPUs[i].tarefa_atual->restante--;//Caso seja executado fora do if pode levar a segmentation fault se a CPU estiver livre

            if(CPUs[i].tarefa_atual->restante == 0)//tempo de execucao acabou, libera a CPU e muda seu estado
            {

              CPUs[i].tarefa_atual->contexto = 0;
              CPUs[i].tarefa_atual->estado = TERMINADA;
              CPUs[i].tarefa_atual->termino = tick;
              CPUs[i].ocupado = LIVRE;
              CPUs[i].tarefa_atual = NULL;

              
            }
          }
        }

      (*tick)++;
  }
  else if (atalho == 'M')
  {
    int id_modificado;

    printf("Qual estado será definido para a tarefa?\n");
    printf("NOVA (1), PRONTA (2), EXECUTANDO (3), TERMINADA (4)");
    scanf("%c", &atalho);

    printf("Indique qual tarefa deseja modificar pelo ID da mesma\n");
    scanf("%d", &id_modificado);
    
   
    }
    else if (atalho == 'R')// quando retroceder, é necessário indicar que o tick_atual foi reduzido em uma unidade, ou seja, voltou para a iteração anterior,então retomamos o historico já salvo
    {
      if(tick_atual > 0)//evitar acesso indevido a memória
      {
        (*tick_atual)--;
        retrocederEstadoAtual(tarefas_novas, fila_prontas, CPUs, &tick, historico, tick_atual);
      }
      else//caso o valor seja zero, ou seja, o caso inicial da simulação, não conseguimos retroceder
      {
        printf("Opção invalida!");
        exit(1);
      }
    }
  }
}

void execucaoCompleta(TCB* tarefas_novas,int* tick,Queue* filas_prontas, CPU* CPUs,int &tick_atual,int &tick_max)
{
   tick_atual++;
  tick_max = tick_atual;

    define_pronta(tarefas_novas, tick);//marca as tarefas como prontas

    enfileiraProntas(tarefas_novas, fila_prontas, tick);//adiciona as tarefas com estado "PRONTA" na fila de prontas


        //A funcao abaixo e responsavel por distribuir as tarefas prontas para os CPUs disponiveis, e caso 
        //nao haja tarefas prontas, a CPU e "desligada" e seu tempo nesse estado e contabilizado
    for(int i = 0; i < N_CPUs; i++)
       {
        if(CPUs[i].ocupado == LIVRE && !empty(fila_prontas))
        {
          CPUs[i].ocupado = OCUPADA;
          CPUs[i].tarefa_atual = dequeue(fila_prontas);
          CPUs[i].tarefa_atual->contexto = 1;
          CPUs[i].tarefa_atual->estado = EXECUTANDO;
        }
        if(CPUs[i].ocupado == LIVRE && empty(fila_prontas))
          CPUs[i].tempo_desligado++;
        }

        //Esse loop contabiliza o tempo em que a tarefa esta sendo executada e, caso se encerre o processo,
        //ela e retirada do processador e tem seu tempo de finalizacao marcado

        for(int i = 0; i < N_CPUs; i++)
        {

          if(CPUs[i].ocupado == OCUPADA && CPUs[i].tarefa_atual->estado == EXECUTANDO)
          {
            
           CPUs[i].tarefa_atual->restante--;//Caso seja executado fora do if pode levar a segmentation fault se a CPU estiver livre

            if(CPUs[i].tarefa_atual->restante == 0)//tempo de execucao acabou, libera a CPU e muda seu estado
            {

              CPUs[i].tarefa_atual->contexto = 0;
              CPUs[i].tarefa_atual->estado = TERMINADA;
              CPUs[i].tarefa_atual->termino = tick;
              CPUs[i].ocupado = LIVRE;
              CPUs[i].tarefa_atual = NULL;

              
            }
          }
        }

      (*tick)++;
}


//cria e inicializa uma fila de tamanho variavel
Queue *create (int size)
{
  Queue *q = (Queue *)malloc(sizeof(Queue));
  q->front = 0;
  q->back = 0;
  q->size = size;
  q->array = (TCB**)malloc(size * sizeof(TCB*));

  return q;

}
//enfileira um elemento na fila
void enqueue (Queue *q, TCB* tarefa)
 {
  if (full(q)) {
    printf ("error: queue overflow!\n");
    exit(1);
  }
  q->array[q->back] = tarefa;
  q->back = (q->back + 1) % q->size;
}

//Fun��o para desenfileirar o primeiro elemento da fila
TCB* dequeue (Queue *q) 
{
  if (empty(q)) {
    printf ("error: queue underflow!\n");
    exit(1);
  }
  TCB* tarefa = q->array[q->front];
  q->front = (q->front + 1) % q->size;
  return tarefa;
}

//retorna mas n�o remove o primeiro elemento da fila
int frontID (Queue *q) 
{
  if (!empty(q))
    return q->array[q->front]->id_tarefa;
  else
    return NULL;
}

//retorna o valor na parte de tras da fila
int backID (Queue *q)
{
  if (!empty(q))
    return q->array[q->back - 1]->id_tarefa;
  else
    return NULL;
}

// verifica se a fila est� vazia ou n�o
int empty (Queue *q) 
{
  return (q->front == q->back);
}

//verifica se a fila est� cheia ou n�o
int full (Queue *q)
 {
  return (q->front == ((q->back + 1) % q->size));
}

//imprime a fila
void print (Queue *q)
 {
  int i;
  printf("Queue: ");
  for (i = q->front; i != q->back; i = (i+1) % q->size) {
    printf ("%d ", q->array[i]->id_tarefa);
  }
  printf("\n");
}

//desaloca as estruturas de uma fila
void destroy (Queue *q) 
{
  free (q->array);
  free (q);
}

//retorna o n�mero de elementos alocados no vetor
int getsize (Queue *q) 
{
  return q->size;
}

// retorna 1 se o elemento existe em na fila
int search (Queue *q, TCB* tarefa) 
{
  int i;
  for (i = q->front; i != q->back; i = (i+1) % q->size)
    if (q->array[i] == tarefa)
      return 1;
  return 0;
}

ConfigSistema lerConfiguracao(TCB tarefas[], int *qtde_tarefas)
{
  FILE *ler_arquivo;
  char linha[256];
  ConfigSistema config;
  ConfigSistema erro = {0};
  TCB tarefas_novas;
  int i = 0;

  printf("Digite o nome do arquivo de configuracao: ");
  char arquivo[256];
  scanf("%s", arquivo);

  ler_arquivo = fopen(arquivo, "r");
  if (ler_arquivo == NULL) {
    printf("Erro ao abrir o arquivo de configuração.\n");
    return erro;
  }
 
  

  //le configuracao do sistema
  if(fgets(linha, sizeof(linha), ler_arquivo)) 
  {
    char *separar = strtok(linha, ";");
    separar = strtok(NULL, ";");
    if(separar != NULL)// Caso o arquivo venha mal formatado, pode causar segmentation fault, por isso a condicional
      sscanf(separar, "%d", &config.quantum);    // quantum

    separar = strtok(NULL, ";");
    sscanf(separar, "%d", &config.qtde_cpus); // qtde_cpus
  }

//  le as tarefas
while (fgets(linha, sizeof(linha), ler_arquivo)) 
{
    char *separar = strtok(linha, ";");
    separar = strtok(linha, ";");
    tarefas[i].id_tarefa = atoi(separar);//id
    //strcpy(tarefas[i].id_tarefa, separar);  // id

    separar = strtok(NULL, ";");
    strcpy(tarefas[i].cor, separar);         // cor

    separar = strtok(NULL, ";");
    sscanf(separar, "%d", &tarefas[i].ingresso);  // ingresso

    separar = strtok(NULL, ";");
    sscanf(separar, "%d", &tarefas[i].duracao);   // duracao

    separar = strtok(NULL, ";");
    sscanf(separar, "%d", &tarefas[i].prioridade); // prioridade

    separar = strtok(NULL, "\n");
    if (separar != NULL)
        strcpy(tarefas[i].lista_eventos, separar);  // eventos
    i++;
}

*qtde_tarefas = i;
    
  fclose(ler_arquivo);
  return config;
}