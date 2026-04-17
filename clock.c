#include <time.h>
#include <unistd.h>

#define SIZE 1000
#define N_CPUs 5

#define NOVA 1
#define PRONTA 2
#define EXECUTANDO 3
#define TERMINADA 4
#define SUSPENSA 5

#define LIVRE 1
#define OCUPADA 0

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
void inicializaCPU (CPU* CPUs);
Queue *create (int size);
void destroy (Queue *q);
void enqueue (Queue *q, TCB* tarefa);
TCB* dequeue (Queue *q);
TCB* front (Queue *q);
int empty (Queue *q);
int full (Queue *q);
void print (Queue *q);
int getsize (Queue *q);
int search (Queue *q, TCB* tarefa);

//===================================Codigo central================================//

int main()
{
    int tick = 0;

    //A inicializacao das tarefas e da fila de pronto, bem como a chamada para
    //elas entrarem no estado "NOVA" deve ocorrer fora do loop geral de execucao, caso contrario � executado sempre
    TCB tarefas_novas[SIZE];
    Queue* fila_prontas = create(SIZE);
    CPU CPUs[N_CPUs]; //vetor que representa os processadores do Sistema Operacional

    for(int i = 0; i < N_CPUs; i++)
        inicializaCPU(&CPUs[i]);

    inicializa_tarefas(tarefas_novas);

    while(1)
    {

        define_pronta(tarefas_novas, tick);

        for(int i = 0; i < SIZE && !full(fila_prontas);i++)
        {
            if(tarefas_novas[i].estado == PRONTA && tarefas_novas[i].ingresso == tick)//garante com que a tarefa esteja pronta e entre apenas uma vez na fila
                enqueue(fila_prontas,&tarefas_novas[i]);//a fila de tarefas prontas armazena somente o id
        }


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

        //Essa funcao contabiliza o tempo em que a tarefa esta sendo executada e, caso se encerre o processo,
        //ela e retirada do processador e tem seu tempo de finalizacao marcado

        for(int i = 0; i < N_CPUs; i++)
        {

          if(CPUs[i].ocupado == OCUPADA && CPUs[i].tarefa_atual->estado == EXECUTANDO)
          {
            if(CPUs[i].tarefa_atual->restante == 0)//tempo de execucao acabou, libera a CPU e muda seu estado
            {
              CPUs[i].tarefa_atual->restante--;//Caso seja executado fora do if pode levar a segmentation fault se a CPU estiver livre

              CPUs[i].tarefa_atual->contexto = 0;
              CPUs[i].tarefa_atual->estado = TERMINADA;
              CPUs[i].tarefa_atual->termino = tick;
              CPUs[i].ocupado = LIVRE;
              CPUs[i].tarefa_atual = NULL;

              
            }
          }
        }



        tick++;
    }


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

//Caso o momento de ingresso da tarefa seja equivalente ao tick atual, atualizamos seu estado para "PRONTA"
void define_pronta(TCB* tarefas_novas, int tick)
{
    for(int i = 0; i < SIZE; i++ )
    {
        if(tarefas_novas[i].ingresso == tick)
            tarefas_novas[i].estado = PRONTA;
    }

}

void inicializaCPU (CPU* CPUs)
{
  CPUs->ocupado = LIVRE; 
  CPUs->tempo_desligado = 0;
  CPUs->tarefa_atual = NULL;

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
void enqueue (Queue *q, TCB* tarefa) {
  if (full(q)) {
    printf ("error: queue overflow!\n");
    exit(1);
  }
  q->array[q->back] = tarefa;
  q->back = (q->back + 1) % q->size;
}

//Fun��o para desenfileirar o primeiro elemento da fila
TCB* dequeue (Queue *q) {
  if (empty(q)) {
    printf ("error: queue underflow!\n");
    exit(1);
  }
  TCB* tarefa = q->array[q->front];
  q->front = (q->front + 1) % q->size;
  return tarefa;
}

//retorna mas n�o remove o primeiro elemento da fila
TCB* front (Queue *q) {
  if (!empty(q))
    return q->array[q->front];
  else
    return NULL;
}

// verifica se a fila est� vazia ou n�o
int empty (Queue *q) {
  return (q->front == q->back);
}

//verifica se a fila est� cheia ou n�o
int full (Queue *q) {
  return (q->front == ((q->back + 1) % q->size));
}

//imprime a fila
void print (Queue *q) {
  int i;
  printf("Queue: ");
  for (i = q->front; i != q->back; i = (i+1) % q->size) {
    printf ("%d ", q->array[i]->id_tarefa);
  }
  printf("\n");
}

//desaloca as estruturas de uma fila
void destroy (Queue *q) {
  free (q->array);
  free (q);
}

//retorna o n�mero de elementos alocados no vetor
int getsize (Queue *q) {
  return q->size;
}

// retorna 1 se o elemento existe em na fila
int search (Queue *q, TCB* tarefa) {
  int i;
  for (i = q->front; i != q->back; i = (i+1) % q->size)
    if (q->array[i] == tarefa)
      return 1;
  return 0;
}
