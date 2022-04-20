// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel
// COMANDO PARA COMPILAR: gcc -o Questao03 Questao03.c -pthread

// Declaração das bibliotecas necessárias
# include <fcntl.h>
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

// Definindo variáveis compartilhadas
int *buffer, *consumer_position, *producer_position;
sem_t *empty, *full, *mutex;

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
        // Decrementando um espaço vazio
        sem_wait(empty);
        // Entrando na região crítica
        sem_wait(mutex);
        insert_item(item);
        // Saindo da região crítica
        sem_post(mutex);
        // Incrementando um espaço preenchido
		    sem_post(full);
        // Espera de 1 segundo para a próxima execução
        sleep(1);
    }
}

// Definindo a função do consumidor
void consumer (void) {
    while (true) {
        printf("\nCONSUMING...\n");
        // Decrementando um espaço preenchido
        sem_wait(full);
        // Entrando na região crítica
        sem_wait(mutex);
        int item = remove_item();
        // Saindo da região crítica
        sem_post(mutex);
        // Incrementando um espaço vazio
		    sem_post(empty);
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

    // Definindo um segmento de memória compartilhada para a posição do consumidor
    int consumer_position_id = shmget((key_t) 0x200, sizeof(int), 0666|IPC_CREAT);
    consumer_position = (int *) shmat(consumer_position_id, 0, 0);

    // Definindo um segmento de memória compartilhada para a posição do produtor
    int producer_position_id = shmget((key_t) 0x300, sizeof(int), 0666|IPC_CREAT);
    producer_position = (int *) shmat(producer_position_id, 0, 0);

    // Desliga dos outros semáforos criados com o mesmo nome em uma execução anterior
    sem_unlink("empty");
    sem_unlink("full");
    sem_unlink("mutex");

    // Inicialização dos semáforos
    empty = sem_open("empty", O_CREAT | O_EXCL, S_IWUSR, N);
    full = sem_open("full", O_CREAT | O_EXCL, S_IWUSR, 0);
    mutex = sem_open("mutex", O_CREAT | O_EXCL, S_IWUSR, 1);

    // Inicialização das variáveis compartilhadas e do buffer
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
