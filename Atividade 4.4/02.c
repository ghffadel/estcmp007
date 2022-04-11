// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel

# include <signal.h>

// Definindo o tamanho do buffer
# define N 100

// Variável que representa a quantidade de itens atualmente no buffer
int count = 0;

// Definindo a função do produtor
void producer (void) {
    while (true) {
        int item = produce_item();
        
        if (count == N) {
            // Função de dormir
            signal(SIGSTOP, producer);
        }
        
        insert_item(item);
        count++;
        
        if (count == 1) {
            // Função de acordar
            signal(SIGCONT, consumer);
        }
    }
}

// Definindo a função do consumidor
void consumer (void) {
    while (true) {
        if (count == 0) {
            // Função de dormir
            signal(SIGSTOP, consumer);
        }
        
        int item = remove_item();
        count--;
        
        if (count == N - 1) {
            // Função de acordar
            signal(SIGCONT, producer);
        }
        
        consume_item(item);
    }
}