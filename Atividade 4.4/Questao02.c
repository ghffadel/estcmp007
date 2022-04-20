// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel
// COMANDO PARA COMPILAR: gcc -o Questao02 Questao02.c

// Declaração das bibliotecas necessárias
# include <signal.h>
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
// Tamanho do buffer
# define N 100

// Variáveis compartilhadas
bool *consumer_sleeping, *producer_sleeping;
int *buffer, *consumer_position, *count, *producer_position;

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
    buffer[*producer_position] = item;
    // Próxima posição do produtor
    *producer_position = (*producer_position + 1) % N;
    print_buffer();
}

// Removendo um item do buffer
int remove_item (void) {
    int item = buffer[*consumer_position];
    // A posição atual voltará a ficar vazia
    buffer[*consumer_position] = EMPTY;
    // Próxima posição do consumidor
    *consumer_position = (*consumer_position + 1) % N;
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
        printf("\nPRODUCING...\n");

        int item = produce_item();

        // Se o buffer estiver cheio, o produtor passará a "dormir"
        if (*count == N) {
            printf("\nPRODUCER WILL SLEEP...\n");
            *producer_sleeping = true;
        }

        // O loop não continuará sua execução se o produtor estiver "dormindo"
        if (*producer_sleeping) {
            continue;
        }

        insert_item(item);
        *count = (*count + 1);

        // Se o buffer passar a ter 1 elemento, o consumidor não estará mais "dormindo"
        if (*count == 1) {
            printf("\nCONSUMER WILL WAKE UP...\n");
            *consumer_sleeping = false;
        }

        // Espera de 1 segundo para a próxima execução
        sleep(1);
    }
}

// Definindo a função do consumidor
void consumer (void) {
    while (true) {
        printf("\nCONSUMING...\n");

        // Se não houver elementos no buffer, o consumidor irá "dormir"
        if (*count == 0) {
            printf("\nCONSUMER WILL SLEEP...\n");
            *consumer_sleeping = true;
        }

        // O loop não continuará sua execução se o consumidor estiver "dormindo"
        if (*consumer_sleeping) {
            continue;
        }

        int item = remove_item();
        *count = (*count - 1);

        // Se houver 1 espaço vazio no buffer, o consumidor não estará mais "dormindo"
        if (*count == N - 1) {
            printf("\nPRODUCER WILL WAKE UP...\n");
            *producer_sleeping = false;
        }

        consume_item(item);

        // Espera de 2 segundos para a próxima execução
        sleep(2);
    }
}

int main () {
    int child_status;
    pid_t pid;

    // Semente dos números aleatórios
    srand(time(NULL));

    // Definindo um segmento de memória compartilhada para o buffer
    int buffer_id = shmget((key_t) 0x100, N * sizeof(int), 0666|IPC_CREAT);
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

    // Definindo um segmento de memória compartilhada para a posição do consumidor
    int consumer_position_id = shmget((key_t) 0x500, sizeof(int), 0666|IPC_CREAT);
    consumer_position = (int *) shmat(consumer_position_id, 0, 0);

    // Definindo um segmento de memória compartilhada para a posição do produtor
    int producer_position_id = shmget((key_t) 0x600, sizeof(int), 0666|IPC_CREAT);
    producer_position = (int *) shmat(producer_position_id, 0, 0);

    // Inicialização das variáveis compartilhadas e do buffer
    *count = 0;
    *consumer_sleeping = false;
    *producer_sleeping = false;
    *consumer_position = 0;
    *producer_position = 0;

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
