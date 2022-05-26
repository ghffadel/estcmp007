// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel
// COMANDO PARA COMPILAR: gcc -o Questao05 Questao05.c -pthread

// Declaração de bibliotecas
# include <pthread.h>
# include <stdint.h>
# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <unistd.h>

// O tempo limite de execução é de 15 segundos
# define LIMIT 15
// Função que compara x e y e retorna o maior
# define max(x, y) (x) > (y) ? (x) : (y)
// Função que compara x e y e retorna o menor
# define min(x, y) (x) < (y) ? (x) : (y)
// Número máximo de processos
# define N 5

// Definição da struct do processo
typedef struct {
    char name;
    int burst_time, position;
} Process;

// Declaração de variáveis
int processes_count, quantum, turn = 0;
Process processes[N];
pthread_t threads[N];

// Gera uma letra baseado na tabela ASCII (0 - A, 1 - B, 2 - C, 3 - D, ...)
char generate_letter (int position) {
    return 'A' + position;
}

// Gera um tempo de execução aleatório no intervalo [quantum, LIMIT)
int generate_burst_time (int quantum) {
    int number;
    
    while ((number = (rand() % LIMIT + 1) * 1000000) == 0);
    
    return number;
}

// Imprime informações relevantes sobre os processos (nome e tempo de execução restante)
void print_processes () {
    printf("\nProcesses:\n");
    
    for (int i = 0; i < N; i++) {
        printf("Process %c:\tBurst time: %.1f\n", processes[i].name, processes[i].burst_time / 1000000.0);
    }
    
    printf("\n");
}

// Função para escolher a próxima posição
void choose_next (int current_position) {
    // Começa a partir da posição seguinte
    int next_position = (current_position + 1) % N;
    
    // Loop para encontrar a posição do próximo processo que ainda não terminou de executar
    while (processes[next_position].burst_time == 0) {
        next_position = (next_position + 1) % N;
        
        // Para a execução caso a posição retorne para o início, indicando que todos os processos já terminaram de executar
        if (next_position == current_position) {
            return;
        }
    }
    
    // Atualiza a variável da vez
    turn = next_position;
    
    // Imprime o estado atual dos processos
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
        
        int remaining_processes = 0;
        
        // Verifica quantos processos ainda não terminaram de executar
        for (int j = 0; j < processes_count; j++) {
            if (processes[j].burst_time > 0) {
                remaining_processes++;
            }
        }
        
        // Caso todos tenham terminado, o laço será interrompido
        if (remaining_processes == 0) {
            break;
        }
        
        // Divide o quantum para a quantidade de processos restantes
        int quantum_division = quantum / remaining_processes;
        
        printf("Quantum division = %.1f\n", quantum_division / 1000000.0);
        
        // Se o tempo restante for maior que o quantum, o processo executará em tempo igual ao quantum
        if (processes[i].burst_time > quantum_division) {
            usleep(quantum_division);
            processes[i].burst_time -= quantum_division;
            printf("Process %c\tExecution time: %.1f\tRemaining time: %.1f\n", processes[i].name, quantum_division / 1000000.0, processes[i].burst_time / 1000000.0);
        }
        
        // Caso contrário, o processo executará o tempo que falta
        else {
            usleep(processes[i].burst_time);
            printf("Process %c\tExecution time: %.1f\tRemaining time: 0\n", processes[i].name, processes[i].burst_time / 1000000.0);
            processes[i].burst_time = 0;
        }
        
        // Escolhe a próxima posição
        choose_next(i);
    }
    
    // Saída do thread
    pthread_exit(0);
}

int main () {
    // Semente para os números aleatórios
    srand(time(NULL));
    
    // Lê o quantum que o usuário digitar
    printf("Quantum: "); scanf("%d", &quantum);
    // Multiplica-se por 1000 para converter para microsegundos
    quantum *= 1000000;
    
    // Lê a quantidade de processos
    printf("Number of processes: "); scanf("%d", &processes_count);
    // Não deixa o número de processos passar de N
    processes_count = min(processes_count, N);
    
    // Inicializa os processos
    for (int i = 0; i < processes_count; i++) {
        processes[i].burst_time = generate_burst_time(quantum);
        processes[i].name = generate_letter(i);
        processes[i].position = i;
    }
    
    // Imprime os processos
    print_processes();
    
    // Criação dos threads
    for (int i = 0; i < processes_count; i++) {
        int status = pthread_create(&threads[i], NULL, run, (void *) (intptr_t) i);
        
        // Tratamento para erro na criação do thread
        if (status != 0) {
            printf("Error code %d in thread %d\n", status, i);
            exit(-1);
        }
    }
    
    // Espera todos os threads finalizarem sua execução
    for (int i = 0; i < processes_count; i++) {
        pthread_join(threads[i], 0);
    }
    
    return 0;
}