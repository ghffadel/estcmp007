// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel

# include <locale.h>
# include <semaphore.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

// Tamanho máximo do buffer
# define N 100

// Definindo o monitor
typedef struct monitor {
    int count;
    sem_t full, empty;
} Monitor;

// Produzindo um item aleatório 
int produce_item (void) {
    return rand() % N;
}

// Declarando variáveis
int buffer[N], consumer_position, producer_position;
Monitor monitor;

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

// Isolando a inserção de um item no buffer
void monitor_insert (int item) {
    // Aguarda o buffer ter pelo menos uma posição vazia
    if (monitor.count == N) {
        sem_wait(&(monitor.full));
    }
    
    insert_item(item);
    monitor.count++;
    
    // Se o buffer não estiver mais vazio, envia um sinal para o consumidor
    if (monitor.count == 1) {
        sem_post(&(monitor.empty));
    }
}

int monitor_remove (void) {
    // Aguarda o buffer ter pelo menos uma posição preenchida
    if (monitor.count == 0) {
        sem_wait(&(monitor.empty));
    }
    
    int item = remove_item();
    monitor.count--;
    
    // Se o buffer não estiver mais cheio, envia um sinal para o consumidor
    if (monitor.count == N - 1) {
        sem_post(&(monitor.full));
    }
}

// Definindo o produtor
void producer (void) {
    while (true) {
        int item = produce_item();
        monitor_insert(item);
    }
}

// Definindo o consumidor
void consumer (void) {
    while (true) {
        int item = monitor_remove();
        consume_item(item);
    }
}