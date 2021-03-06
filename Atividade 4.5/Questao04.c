// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel
// COMANDO PARA COMPILAR: gcc -o Questao04 Questao04.c -pthread

// Declaração de bibliotecas
# include <limits.h>
# include <pthread.h>
# include <stdbool.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <unistd.h>

// O tempo limite de execução é de 15 segundos
# define LIMIT 15
// Função que compara x e y e retorna o maior
# define max(x, y) (x) > (y) ? (x) : (y)
// Número de processos
# define N 5

// Definição da struct do processo
typedef struct {
    char name;
    int burst_time, position;
} Process;

// Declaração de variáveis
int quantum, turn;
Process processes[N];
pthread_t threads[N];

// Gera uma letra baseado na tabela ASCII (0 - A, 1 - B, 2 - C, 3 - D, ...)
char generate_letter (int position) {
    return 'A' + position;
}

// Gera um tempo de execução aleatório no intervalo [quantum, LIMIT)
int generate_burst_time (int quantum) {
    int number;
    
    while ((number = max(quantum, rand() % LIMIT)) == 0);
    
    return number;
}

// Imprime informações relevantes sobre os processos (nome e tempo de execução restante)
void print_processes () {
    printf("\nProcesses:\n");
    
    for (int i = 0; i < N; i++) {
        printf("Process %c:\tBurst time: %d\n", processes[i].name, processes[i].burst_time);
    }
    
    printf("\n");
}

// Função para escolher a próxima posição
void choose_next () {
    bool finished_all = true;
    int next_position, shortest_time = INT_MAX;
    
    for (int i = 0; i < N; i++) {
        // Se o processo ainda não terminou de ser executado, então nem todos terminaram
        if (processes[i].burst_time > 0) {
            finished_all = false;
        }
        
        // Verifica se o processo ainda não terminou de ser executado e se tem o menor tempo de execução
        if (processes[i].burst_time > 0 && processes[i].burst_time < shortest_time) {
            shortest_time = processes[i].burst_time;
            next_position = processes[i].position;
        }
    }
    
    // Se nem todos os processos terminaram, a vez será do processo com menor tempo de execução
	if (!finished_all) {
		turn = next_position;
	}
	
	// Imprime o estado atual de todos os processos
	print_processes();
}

// Função executada por cada thread
void *run (void *tid) {
    // Índice do thread
    int i = (intptr_t) tid;
    
    // Executa enquanto o tempo de processamento ainda não chegar a 0
    while (processes[i].burst_time > 0) {
        // Aguarda a sua vez
        while (turn != processes[i].position);
        
        // Se o tempo restante for maior que o quantum, o processo executará em tempo igual ao quantum
        if (processes[i].burst_time > quantum) {
            sleep(quantum);
            processes[i].burst_time -= quantum;
            printf("Process %c\tExecution time: %d\tRemaining time: %d\n", processes[i].name, quantum, processes[i].burst_time);
            
        }
        
        // Caso contrário, o processo executará o tempo que falta
        else {
            sleep(processes[i].burst_time);
            printf("Process %c\tExecution time: %d\tRemaining time: 0\n", processes[i].name, processes[i].burst_time);
            processes[i].burst_time = 0;
        }
        
        // Escolhe a próxima posição
        choose_next();
    }
    
    // Saída do thread
    pthread_exit(0);
}

int main () {
    // Semente para os números aleatórios
    srand(time(NULL));
    
    // Lê o quantum que o usuário digitar
    printf("Quantum: "); scanf("%d", &quantum);
    
    // Inicializa os processos
    for (int i = 0; i < N; i++) {
        processes[i].burst_time = generate_burst_time(quantum);
        processes[i].name = generate_letter(i);
        processes[i].position = i;
    }
    
    // Escolhe a primeira posição a ser executada
    choose_next();
    
    // Criação dos threads
    for (int i = 0; i < N; i++) {
        int status = pthread_create(&threads[i], NULL, run, (void *) (intptr_t) i);
        
        // Tratamento para erro na criação do thread
        if (status != 0) {
            printf("Error code %d in thread %d\n", status, i);
            exit(-1);
        }
    }
    
    // Espera todos os threads finalizarem sua execução
    for (int i = 0; i < N; i++) {
        pthread_join(threads[i], 0);
    }
    
    return 0;
}