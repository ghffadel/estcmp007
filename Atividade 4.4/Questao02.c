// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel

# include <signal.h>
# include <semaphore.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/types.h>
# include <unistd.h>

# define EMPTY -1
# define N 100

// Variáveis compartilhadas
bool *consumer_sleeping, *producer_sleeping;
int *buffer, *count;

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
    buffer[*count] = item;
    print_buffer();
}

// Removendo um item do buffer
int remove_item (void) {
    int item = buffer[*count];
    buffer[*count] = EMPTY;
    print_buffer();
    return item;
}

// Consumindo um item do buffer
void consume_item (int item) {
    printf("Consumed item: %d\n", item);
}

// Definindo a função do produtor
void producer (void) {
    while (true) {
        printf("FUNCTION PRODUCER\n");
        int item = produce_item();

        // Se o buffer estiver cheio, o produtor passará a "dormir"
        if (*count == N) {
            *producer_sleeping = true;
        }

        // O loop não continuará sua execução se o produtor estiver "dormindo"
        if (*producer_sleeping) {
            continue;
        }

        insert_item(item);
        *count++;

        // Se o buffer passar a ter 1 elemento, o consumidor não estará mais "dormindo"
        if (*count == 1) {
            *consumer_sleeping = false;
        }

        sleep(2);
    }
}

// Definindo a função do consumidor
void consumer (void) {
    while (true) {
        printf("FUNCTION CONSUMER\n");
        // Se não houver elementos no buffer, o consumidor irá "dormir"
        if (*count == 0) {
            *consumer_sleeping = true;
        }

        // O loop não continuará sua execução se o consumidor estiver "dormindo"
        if (*consumer_sleeping) {
            continue;
        }

        int item = remove_item();
        *count--;

        // Se houver 1 espaço vazio no buffer, o consumidor não estará mais "dormindo"
        if (*count == N - 1) {
            *producer_sleeping = false;
        }

        consume_item(item);
        sleep(3);
    }
}

int main () {
    int consumer_pid, producer_pid;

    // Definindo um segmento de memória compartilhada para o buffer
    int buffer_id = shmget((key_t) 0x100, sizeof(int *), 0666|IPC_CREAT);
    buffer = (int *) shmat(buffer_id, 0, 0);

    // Definindo um segmento de memória compartilhada para a quantidade
    int count_id = shmget((key_t) 0x200, sizeof(int), 0666|IPC_CREAT);
    count = (int *) shmat(count_id, 0, 0);

    // Definindo um segmento de memória compartilhada para verificação do estado atual do consumidor
    int consumer_sleeping_id = shmget((key_t) 0x300, sizeof(bool), 0666|IPC_CREAT);
    consumer_sleeping = (bool *) shmat(consumer_sleeping_id, 0, 0);

    // Definindo um segmento de memória compartilhada para verificação do estado atual do produtor
    int producer_sleeping_id = shmget((key_t) 0x400, sizeof(bool), 0666|IPC_CREAT);
    producer_sleeping = (bool *) shmat(producer_sleeping_id, 0, 0);

    // Inicializando as variáveis e a lista
    *count = 0;
    *consumer_sleeping = false;
    *producer_sleeping = false;

    return 0;
}
