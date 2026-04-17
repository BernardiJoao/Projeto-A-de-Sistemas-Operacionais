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
    int tarefa_atual; //tarefa que usa a CPU em questão;
    int tempo_desligado; // indica o tempo que o processador ficou inativo
} CPU;

typedef struct
{
  int front;//posicao inicial da fila
  int back;//posicao final da fila
  int size;//tamanho da fila
  TCB *array;//vetor de elementos da fila

} Queue;

/*===================================================================================/
/ para começar propriamente a implementação do sistema,precisamos definir o estado   /
/ inicial das tarefas, para que, ao serem devidamente "carregadas", entrarem na fila /
/ de tarefas prontas, que será simulada por meio da estrutura de dados fila (queue)  /
/===================================================================================*/

/*===================================================================================/
/==========================--Declaracao de funcoes auxiliares--======================/
/===================================================================================*/

void inicializa_tarefas(TCB* tarefas);
Queue *create (int size);
void destroy (Queue *q);
void enqueue (Queue *q, int elem);
int dequeue (Queue *q);
int front (Queue *q);
int empty (Queue *q);
int full (Queue *q);
void print (Queue *q);
int getsize (Queue *q);
int search (Queue *q, int e);

//===================================Codigo central================================//

int main()
{
    Queue* tarefas = create(SIZE);
    inicializa_tarefas(tarefas);

    for(int i = 0; i < SIZE && !full(tarefas);i++)
    {
        if(tarefas->array[i].estado == PRONTA)
            enqueue(tarefas[i]);
    }


}

int tick()
{
   int tick= 0;

   while(1)
   {
       tick++;
   }

   return 0;
}

/*===================================================================================/
/=============================--Funcoes auxiliares utilizadas--======================/
/===================================================================================*/

//altera o estado da tarefa para "NOVA"
void inicializa_tarefas(Queue* tarefas)
{
    int i;
    for (i = 0; i < SIZE; i++)
        tarefas.array[i]->estado = NOVA;
}

//cria e inicializa uma fila de tamanho variavel
Queue *create (int size)
{
  Queue *q = (Queue *)malloc(sizeof(Queue));
  q->front = 0;
  q->back = 0;
  q->size = size;
  q->array = (TCB *)malloc(size * sizeof(TCB));

  return q;

}
//enfileira um elemento na fila
void enqueue (Queue *q, int elem) {
  if (full(q)) {
    printf ("error: queue overflow!\n");
    exit(1);
  }
  q->array[q->back] = elem;
  q->back = (q->back + 1) % q->size;
}

//Função para desenfileirar o primeiro elemento da fila
int dequeue (Queue *q) {
  if (empty(q)) {
    printf ("error: queue underflow!\n");
    exit(1);
  }
  int e = q->array[q->front];
  q->front = (q->front + 1) % q->size;
  return e;
}

//retorna mas não remove o primeiro elemento da fila
int front (Queue *q) {
  if (!empty(q))
    return q->array[q->front];
  else
    return ERROR;
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
    printf ("%d ", q->array[i]);
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
int search (Queue *q, int e) {
  int i;
  for (i = q->front; i != q->back; i = (i+1) % q->size)
    if (q->array[i] == e)
      return 1;
  return 0;
}
