// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel
// COMANDO PARA COMPILAR: gcc -o Questao04 Questao04.c -pthread

// Declaração das bibliotecas necessárias
# include <fcntl.h>
# include <semaphore.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/types.h>
# include <sys/wait.h>
# include <time.h>
# include <unistd.h>

// Constante que representa um local vazio no buffer
# define EMPTY -1
// Tamanho máximo do buffer
# define N 100

// Definindo o monitor
typedef struct monitor {
    int consumer_position, count, producer_position;
    sem_t empty, full;
} Monitor;

// Definindo variáveis compartilhadas
int *buffer;
Monitor *monitor;

// Imprime o conteúdo atual do buffer
void print_buffer (void) {
    printf("Current buffer:\n");

    for (int i = 0; i < N; i++) {
        printf("%d ", buffer[i]);
    }

    printf("\n");
}

// Produzindo um item aleatório
int produce_item (void) {
    return rand() % N;
}

// Inserindo item no buffer
void insert_item (int item) {
    buffer[monitor->producer_position] = item;
    // Próxima posição do produtor
    monitor->producer_position = (monitor->producer_position + 1) % N;
    print_buffer();
}

// Removendo um item do buffer
int remove_item (void) {
    int item = buffer[monitor->consumer_position];
    // A posição atual voltará a ficar vazia
    buffer[monitor->consumer_position] = EMPTY;
    // Próxima posição do consumidor
    monitor->consumer_position = (monitor->consumer_position + 1) % N;
    print_buffer();
    return item;
}

// Consumindo um item do buffer
void consume_item (int item) {
    printf("Consumed item: %d\n", item);
}

// Isolando a inserção de um item no buffer
void monitor_insert (int item) {
    // Aguarda o buffer ter pelo menos uma posição vazia
    if (monitor->count == N) {
        sem_wait(&monitor->full);
    }

    insert_item(item);
    monitor->count = monitor->count + 1;

    // Se o buffer não estiver mais vazio, envia um sinal para o consumidor
    if (monitor->count == 1) {
        sem_post(&monitor->empty);
    }
}

int monitor_remove (void) {
    // Aguarda o buffer ter pelo menos uma posição preenchida
    if (monitor->count == 0) {
        sem_wait(&monitor->empty);
    }

    int item = remove_item();
    monitor->count = monitor->count - 1;

    // Se o buffer não estiver mais cheio, envia um sinal para o consumidor
    if (monitor->count == N - 1) {
        sem_post(&monitor->full);
    }

    return item;
}

// Definindo o produtor
void producer (void) {
    while (true) {
        printf("\nPRODUCING...\n");
        int item = produce_item();
        monitor_insert(item);
        // Espera de 1 segundo para a próxima execução
        sleep(1);
    }
}

// Definindo o consumidor
void consumer (void) {
    while (true) {
        printf("\nCONSUMING...\n");
        int item = monitor_remove();
        consume_item(item);
        // Espera de 2 segundos para a próxima execução
        sleep(2);
    }
}

int main (void) {
    int child_status;
    pid_t pid;

    // Semente dos números aleatórios
    srand(time(NULL));

    // Definindo um segmento de memória compartilhada para o buffer
    int buffer_id = shmget((key_t) 0x100, N * sizeof(int), 0666|IPC_CREAT);
    buffer = (int *) shmat(buffer_id, 0, 0);

    // Definindo um segmento de memória compartilhada para o monitor
    int monitor_id = shmget((key_t) 0x200, sizeof(Monitor), 0666|IPC_CREAT);
    monitor = (Monitor *) shmat(monitor_id, 0, 0);

    // Inicialização dos semáforos
    sem_init(&monitor->empty, 1, 0);
    sem_init(&monitor->full, 1, 1);

    // Inicialização das variáveis compartilhadas e do buffer
    monitor->count = 0;
    monitor->consumer_position = 0;
    monitor->producer_position = 0;

    for (int i = 0; i < N; i++) {
        buffer[i] = EMPTY;
    }

    // Criação de um novo processo
    pid = fork();

    // Verfica se houve algum erro no fork
    if (pid < 0) {
        perror("fork = ");
        exit(1);
    }

    // Processo-filho é o consumidor
    else if (pid == 0) {
        consumer();
        exit(0);
    }

    // Processo-pai é o produtor
    else {
        producer();
    }

    // Espera o retorno do processo-filho
    wait(&child_status);

    return 0;
}
