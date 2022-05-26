// Dupla: Carlos Alberto de Jesus Vasconcelos e Gustavo Henrique Franco Fadel
// COMANDO PARA COMPILAR: gcc -o Questao05 Questao05.c -pthread

// Declaração de bibliotecas
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
// Número de usuário
# define N 5

// Definição da struct do processo
typedef struct {
    char name;
    int burst_time, position;
} Process;

// Definição da struct do usuário
typedef struct {
    int identifier, processes_count;
    Process *processes;
} User;

// Declaração de variáveis
int quantum, turn = 0;
pthread_t threads[N];
User users[N];

// Gera uma letra baseado na tabela ASCII (0 - A, 1 - B, 2 - C, 3 - D, ...)
char generate_letter (int position) {
    return 'A' + position;
}

// Imprime todos os usuários e informações sobre seus respectivos processos (nome e temp de execução)
void print_users () {
    printf("\nUsers:\n");
    
    for (int i = 0; i < N; i++) {
        printf("\nUser %d:\n", users[i].identifier);
        
        for (int j = 0; j < users[i].processes_count; j++) {
            printf("Process %c:\tBurst time: %.1f\n", users[i].processes[j].name, users[i].processes[j].burst_time / 1000.0);
        }
    }
    
    printf("\n");
}

// Função para escolher o próximo usuário
void choose_next (int current_position) {
    int next_position = (current_position + 1) % N;
    
    while (true) {
        bool finished_all = true;
        
        // Verifica se todos os processos do usuário estão finalizados
        for (int j = 0; j < users[next_position].processes_count; j++) {
            if (users[next_position].processes[j].burst_time > 0) {
                finished_all = false;
                break;
            }
        }
        
        // Se nem todos estiverem terminados, a posição atual será a escolhida
        if (!finished_all) {
            break;
        }
        
        next_position = (next_position + 1) % N;
        
        // Para a execução caso a posição retorne para o início, indicando que todos os processos de todos os usuários já terminaram de executar
        if (next_position == current_position) {
            return;
        }
    }
    
    // Atualiza a variável da vez
    turn = next_position;
}

// Função executada por cada thread
void *run (void *tid) {
    bool finished_all = false;
    int i = (intptr_t) tid;
    
    while (true) {
        // Aguarda a sua vez
        while (turn + 1 != users[i].identifier);
        
        int remaining_processes = 0;
        
        // Verifica quantos processos ainda não terminaram de executar
        for (int j = 0; j < users[i].processes_count; j++) {
            if (users[i].processes[j].burst_time > 0) {
                remaining_processes++;
            }
        }
        
        // Caso todos tenham terminado, o laço será interrompido
        if (remaining_processes == 0) {
            break;
        }
        
        // Divide o quantum para a quantidade de processos restantes
        int quantum_division = quantum / remaining_processes;
        
        printf("User %d:\n", users[i].identifier);
        printf("Quantum division = %.1f\n", quantum_division / 1000.0);
        
        for (int j = 0; j < users[i].processes_count; j++) {
            // Se o tempo restante for maior que o quantum, o processo executará em tempo igual ao quantum
            if (users[i].processes[j].burst_time > quantum_division) {
                usleep(quantum_division);
                users[i].processes[j].burst_time -= quantum_division;
                printf("Process %c\tExecution time: %.1f\tRemaining time: %.1f\n", users[i].processes[j].name, quantum_division / 1000.0, users[i].processes[j].burst_time / 1000.0);
            }
            
            // Caso contrário, o processo executará o tempo que falta
            else if (users[i].processes[j].burst_time > 0) {
                usleep(users[i].processes[j].burst_time);
                printf("Process %c\tExecution time: %.1f\tRemaining time: 0\n", users[i].processes[j].name, users[i].processes[j].burst_time / 1000.0);
                users[i].processes[j].burst_time = 0;
            }
        }
        
        printf("\n");
        
        // Escolhe o próximo usuário
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
    // Multiplica-se por 1000 para converter para milisegundos
    quantum *= 1000;
    
    // Leitura dos usuários
    for (int i = 0; i < N; i++) {
        users[i].identifier = i + 1;
        printf("Number of processes of user %d: ", users[i].identifier); scanf("%d", &users[i].processes_count);
        // Alocação de memória para o vetor de processos
        users[i].processes = malloc(users[i].processes_count * sizeof(Process));
        
        // Inicialização dos processos
        for (int j = 0; j < users[i].processes_count; j++) {
            users[i].processes[j].burst_time = (rand() % LIMIT + 1) * 1000;
            users[i].processes[j].name = generate_letter(j);
            users[i].processes[j].position = j;
        }
    }
    
    // Imprime os usuários e seus processos
    print_users();
    
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
    
    // Libera a memória que foi utilizada pelos vetores de processos
    for (int i = 0; i < N; i++) {
        free(users[i].processes);
    }
    
    return 0;
}