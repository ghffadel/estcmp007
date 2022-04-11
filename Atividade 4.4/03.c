// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel

# include <locale.h>
# include <semaphore.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

// Tamanho do buffer
# define N 100

// Definindo variáveis
int buffer[N], consumer_position, producer_position;
sem_t empty, full, mutex;

// Produzindo um item aleatório 
int produce_item (void) {
    return rand() % N;
}

// Inserindo item no buffer
void insert_item (int item) {
    buffer[producer_position] = item;
    producer_position = (producer_position + 1) % N; // Próxima posição do produtor
}

// Removendo um item do buffer
int remove_item (void) {
    int item = buffer[consumer_position];
    consumer_position = (consumer_position + 1) % N; // Próxima posição do consumidor
    return item;
}

// Consumindo um item do buffer
void consume_item (int item) {
    printf("Item consumido: %d\n", item);
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