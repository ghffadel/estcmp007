// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel

// Importando as bibliotecas necessárias
# include <signal.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/types.h>
# include <unistd.h>

// Quantidade de vezes que o processo alterará a variável
# define M 5
// N = número de processos que terão concorrência no acesso à região crítica
# define N 2

// Variáveis compartilhadas
bool *interested;
int *shared_variable, *turn;

// Função que entra na região crítica
void enter_region (int i) {
    // Variável que verifica se o printf já foi executado para não imprimir repetidas vezes
    bool printed = false;

    interested[i] = true;
    *turn = (1 - i);

    // Verifica se o outro processo está na região crítica
    while (interested[i] && (*turn == 1 - i)) {
        if (!printed) {
            printf("Process %d is waiting\n", i);
            printed = true;
        }
    }

    // O processo entrou na região crítica
    printf("Process %d in critical region", i);
}

// Função que sai da região crítica
void leave_region (int i) {
    interested[i] = false;
}

// Função que realiza a entrada e saída na região crítica
void process (int i) {
    printf("%p\n", interested);

    for (int j = 0; j < M; j++) {
        enter_region(i);

        *shared_variable = rand();
        printf("\tShared variable value: %d\n", *shared_variable);
        sleep(2);

        leave_region(i);
    }
}

int main () {
    pid_t pid;

    // Definindo um segmento de memória compartilhada para a vez
    int turn_id = shmget((key_t) 0x100, sizeof(int), 0666|IPC_CREAT);
    turn = (int *) shmat(turn_id, 0, 0);

    // Definindo um segmento de memória compartilhada para a variável que será compartilhada
    int shared_variable_id = shmget((key_t) 0x200, sizeof(int), 0666|IPC_CREAT);
    shared_variable = (int *) shmat(shared_variable_id, 0, 0);

    // Definindo um segmento de memória compartilhada para o vetor de interessados
    int interested_id = shmget((key_t) 0x300, N * sizeof(bool), 0666|IPC_CREAT);
    interested = (bool *) shmat(interested_id, 0, 0);


    // Um processo aleatório irá começar
    *turn = rand() % N;

    // Reinicia o vetor de interessados
    for (int i = 0; i < N; i++) {
        interested[i] = false;
    }

    while (true) {
        if ((pid = fork()) >= 0) {
            process(pid > 0);
        }

        else {
            printf("Error in creating process\n");
            break;
        }

        sleep(1);
        kill(pid, SIGKILL);
    }

    return 0;
}
