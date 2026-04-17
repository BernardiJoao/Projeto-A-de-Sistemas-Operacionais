#include <time.h>
#include <unistd.h>

#define SIZE 1000

#define NOVA 1
#define PRONTA 2
#define EXECUTANDO 3
#define TERMINADA 4
#define SUSPENSA 5

#define LIVRE 1
#define OCUPADA 0

#define QUANTUM 2

/*================================================================================/
/ Inicialmente, definimos tres structs, uma delas responsável pelo armazenamento  /
/ de informacoes acerca da tarefa (TCB), outra para sabermos informacoes sobre as /
/ CPUs que estão sendo utilizadas no sistema operacional e a ultima responsavel   /
/ pela definicao da fila e seus elementos                                         /
/================================================================================*/

typedef struct
{
    int id_tarefa;
    int estado;
    int contexto;//indica se está ativa(1) ou não (0)

    int ingresso; //momento que muda de nova->pronta (entra na fila de prontas)
    int duracao;//duracao da execucao da tarefa
    int termino; // momento de termino
    int espera; //tempo de espera na fila de prontas

    int restante; //tempo restante para acabar a tarefa
    int quantum_restante;

} TCB;

typedef struct
{
    int ocupado;//flag que indica se a CPU está sendo utilizada(1) ou não(0);
    TCB* tarefa_atual; //tarefa que usa a CPU em questão;
    int tempo_desligado; // indica o tempo que o processador ficou inativo

} CPU;

typedef struct
{
  int front;//posicao inicial da fila
  int back;//posicao final da fila
  int size;//tamanho da fila
  TCB **array;//vetor de elementos da fila

} Queue;

/*===================================================================================/
/ para começar propriamente a implementação do sistema,precisamos definir o estado   /
/ inicial das tarefas, para que, ao serem devidamente "carregadas", entrarem na fila /
/ de tarefas prontas, que será simulada por meio da estrutura de dados fila (queue)  /
/===================================================================================*/

/*===================================================================================/
/==========================--Declaracao de funcoes auxiliares--======================/
/===================================================================================*/

void inicializa_tarefas(TCB* tarefas_novas);
void define_pronta(TCB* tarefas_novas, int tick);
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
    //elas entrarem no estado "NOVA" deve ocorrer fora do loop geral de execucao, caso contrario é executado sempre
    TCB tarefas_novas[SIZE];
    Queue* fila_prontas = create(SIZE);

    inicializa_tarefas(tarefas_novas);

    while(1)
    {

        define_pronta(tarefas_novas, tick);

        for(int i = 0; i < SIZE && !full(fila_prontas);i++)
        {
            if(tarefas_novas[i].estado == PRONTA && tarefas_novas[i].ingresso == tick)//garante com que a tarefa esteja pronta e entre apenas uma vez na fila
                enqueue(fila_prontas,&tarefas_novas[i]);//a fila de tarefas prontas armazena somente o id
        }

        tick++;
    }


}
/*===================================================================================/
/=============================--Funcoes auxiliares utilizadas--======================/
/===================================================================================*/

//altera o estado da tarefa para "NOVA"
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

void define_pronta(TCB* tarefas_novas, int tick)
{
    for(int i = 0; i < SIZE; i++ )
    {
        if(tarefas_novas[i].ingresso == tick)
            tarefas_novas[i].estado = PRONTA;
    }

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

//Função para desenfileirar o primeiro elemento da fila
TCB* dequeue (Queue *q) {
  if (empty(q)) {
    printf ("error: queue underflow!\n");
    exit(1);
  }
  TCB* tarefa = q->array[q->front];
  q->front = (q->front + 1) % q->size;
  return tarefa;
}

//retorna mas não remove o primeiro elemento da fila
TCB* front (Queue *q) {
  if (!empty(q))
    return q->array[q->front];
  else
    return NULL;
}

// verifica se a fila está vazia ou não
int empty (Queue *q) {
  return (q->front == q->back);
}

//verifica se a fila está cheia ou não
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

//retorna o número de elementos alocados no vetor
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
