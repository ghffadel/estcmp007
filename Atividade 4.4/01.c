// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel

# include <locale.h>
# include <stdbool.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

// Quantidade de processos
# define N 2

int interested[N], turn;

// Definindo a função que entra na região crítica
void enter_region (int process) {
    // O número do outro processo
    int other = 1 - process;
    
    // O processo atual está interessado em entrar na região crítica
    interested[process] = true;
    // A vez é do processo atual
    turn = process;
    
    // Aguarda o outro processo sair da região crítica, se ele estiver
    while (turn == process && interested[other]) {
        printf("Processo %d está aguardando...\n", process);
    }
}

// Definindo a função que sai da região crítica
void leave_region (int process) {
    // O processo não está mais interessado na região crítica
    interested[process] = false;
}

int main (void) {
    // Utilização de acentos nas saídas
    setlocale(LC_ALL, "Portuguese");
    
    while (true) {
        pid_t pid;
        
        // Se houver erro na criação de um processo
        if ((pid = fork()) < 0) {
            printf("Erro ao realizar o fork\n");
            return 0;
        }
        
        // Se o id do processo for 0, o número do processo também será 0
        else if (pid == 0) {
            enter_region(0);
            printf("Processo 0 entrou na região crítica\n");
            leave_region(0);
            printf("Processo 0 saiu da região crítica\n");
        }
        
        // Caso contrário, será utilizado 1
        else {
            enter_region(1);
            printf("Processo 1 entrou na região crítica\n");
            leave_region(1);
            printf("Processo 1 saiu da região crítica\n");
        }
    }
    
    return 0;
}