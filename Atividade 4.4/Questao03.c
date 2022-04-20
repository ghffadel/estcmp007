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

# define M 5
// Tamanho do buffer
# define N 100

// Definindo variáveis
int *buffer, *consumer_position, *producer_position;
sem_t empty, full, mutex;

// Produzindo um item aleatório
int produce_item (void) {
    return rand() % N;
}

// Inserindo item no buffer
void insert_item (int item) {
    buffer[*producer_position] = item;
    *producer_position = (*producer_position + 1) % N; // Próxima posição do produtor
}

// Removendo um item do buffer
int remove_item (void) {
    int item = buffer[*consumer_position];
    *consumer_position = (*consumer_position + 1) % N; // Próxima posição do consumidor
    return item;
}

// Consumindo um item do buffer
void consume_item (int item) {
    printf("Consumed item: %d\n", item);
}

// Definindo a função do produtor
void producer (void) {
    while (true) {
        int item = produce_item();
        sem_wait(&empty); // Decrementando um espaço vazio
        sem_wait(&mutex); // Entrando na região crítica
        insert_item(item);
        sem_post(&mutex); // Saindo da região crítica
		    sem_post(&full); // Incrementando um espaço preenchido
    }
}

// Definindo a função do consumidor
void consumer (void) {
    while (true) {
        sem_wait(&full); // Decrementando um espaço preenchido
        sem_wait(&mutex); // Entrando na região crítica
        int item = remove_item();
        sem_post(&mutex); // Saindo da região crítica
		    sem_post(&empty); // Incrementando um espaço vazio
		    consume_item(item);
    }
}

int main (void) {
    pid_t pid;

    int buffer_id = shmget((key_t) 0x100, sizeof(int *), 0666|IPC_CREAT);
    buffer = (int *) shmat(buffer_id, 0, 0);

    int consumer_position_id = shmget((key_t) 0x200, sizeof(int), 0666|IPC_CREAT);
    consumer_position = (int *) shmat(consumer_position_id, 0, 0);

    int producer_position_id = shmget((key_t) 0x300, sizeof(int), 0666|IPC_CREAT);
    producer_position = (int *) shmat(producer_position_id, 0, 0);

    int empty_id = shmget((key_t) 0x400, sizeof(sem_t), 0666|IPC_CREAT);
    empty = (sem_t) shmat(empty_id, 0, 0);

    int full_id = shmget((key_t) 0x500, sizeof(sem_t), 0666|IPC_CREAT);
    full = (sem_t) shmat(full_id, 0, 0);

    int mutex_id = shmget((key_t) 0x600, sizeof(sem_t), 0666|IPC_CREAT);
    mutex = (sem_t) shmat(mutex_id, 0, 0);

    buffer = malloc(N * sizeof(int));
    *consumer_position = *producer_position = 0;

    sem_init(&empty, 1, N);
    sem_init(&full, 1, 0);
    sem_init(&mutex, 1, 0);

    for (int i = 0; i < M; i++) {
        if ((pid = fork()) >= 0) {
            if (pid == 0) {
                producer();
                sleep(2);
            }

            else {
                consumer();
                sleep(3);
            }
        }

        else {
            printf("Error in creating process\n");
            break;
        }

        sleep(1);
        kill(pid, SIGKILL);
    }
}
