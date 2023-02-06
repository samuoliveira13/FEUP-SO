#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>

// Variaveis globais
int num_processes;
double probability;
int timeout;

int validar_args(int argc, char *argv[])
{
    if (argc != 4)
    {
        perror("Invalid number of arguments.\n");
        return -1;
    }

    // Validar o numero de processos
    num_processes = atoi(argv[1]);
    if (num_processes < 2)
    {
        perror("Invalid argument n, it must be higher than 1.\n");
        return -1;
    }

    // Validar a probabilidade
    probability = atof(argv[2]);
    if (probability < 0 || probability > 1)
    {
        perror("Invalid argument p, it must be a number between 0 and 1.\n");
        return -1;
    }

    // Validar o tempo de lock
    timeout = atoi(argv[3]);
    if (timeout < 0)
    {
        perror("Invalid argument n, it must be higher or equal to 0.\n");
        return -1;
    }

    return 1;
}

int create_pipes()
{
    // Variavel auxiliar para guardar o nome dos pipes a criar
    char tmp[100];
    char error_msg[150];

    // Criar os pipes
    for (int i = 1; i < num_processes; i++)
    {
        sprintf(tmp, "/tmp/pipe%dto%d", i, i + 1);                  // Escreve os pipes na variavel tmp
        if (mkfifo(tmp, 0777) == -1)                                // Erro a criar o pipe
        {
            sprintf(error_msg, "Error creating pipe %s\n", tmp);
            perror(error_msg);
            return -1;
        }
    }

    sprintf(tmp, "/tmp/pipe%dto%d", num_processes, 1);              // Cria o ultimo pipe, para ligar ao primeiro
    if (mkfifo(tmp, 0777) == -1)
    {
        sprintf(error_msg, "Error creating pipe %s\n", tmp);
        perror(error_msg);
        return -1;
    }

    printf("[p1] Pipes created\n");
    return 0;
}

void delete_pipes()
{
    // Variavel auxiliar para armazenar o nome dos pipes a eliminar
    char tmp[100];

    // Criar os pipes
    for (int i = 1; i < num_processes; i++)
    {
        sprintf(tmp, "/tmp/pipe%dto%d", i, i + 1);
        unlink(tmp);                                            // Apaga o pipe
    }

    // Create last pipe, from the last to the first
    sprintf(tmp, "/tmp/pipe%dto%d", num_processes, 1);
    unlink(tmp);
}

int open_pipes(int *fd_read, int *fd_write, int j, int k, int l)
{
    char name_pipe_read[50];
    char name_pipe_write[50];
    char error_msg[150];

    printf("[p%d] Opening pipes\n", k);

    sprintf(name_pipe_read, "/tmp/pipe%dto%d", j, k);               // Escreve o pipe nas variaveis
    sprintf(name_pipe_write, "/tmp/pipe%dto%d", k, l);

    // Abrir pipe para leitura
    *fd_read = open(name_pipe_read, O_RDONLY);
    if (*fd_read == -1)
    {
        sprintf(error_msg, "Error opening pipe %s to READ\n", name_pipe_read);
        perror(error_msg);
        return -1;
    }

    // Abrir pipe para escrita
    *fd_write = open(name_pipe_write, O_WRONLY);
    if (*fd_write == -1)
    {
        sprintf(error_msg, "Error opening pipe %s to WRITE\n", name_pipe_write);
        perror(error_msg);
        return -1;
    }
    printf("[p%d] Pipes opened\n", k);
    return 0;
}

int open_pipes_reverse(int *fd_read, int *fd_write, int j, int k, int l)
{
    char name_pipe_read[50];
    char name_pipe_write[50];
    char error_msg[150];

    printf("[p%d] Opening pipes\n", k);

    // Preparar o nome dos pipes
    sprintf(name_pipe_read, "/tmp/pipe%dto%d", j, k);
    sprintf(name_pipe_write, "/tmp/pipe%dto%d", k, l);

    // Abrir pipe para escrita
    *fd_write = open(name_pipe_write, O_WRONLY);
    if (*fd_write == -1)
    {
        sprintf(error_msg, "Error opening pipe %s to WRITE\n", name_pipe_write);
        perror(error_msg);
        return -1;
    }

    // Abrir pipe para leitura
    *fd_read = open(name_pipe_read, O_RDONLY);
    if (*fd_read == -1)
    {
        sprintf(error_msg, "Error opening pipe %s to READ\n", name_pipe_read);
        perror(error_msg);
        return -1;
    }
    printf("[p%d] Pipes opened\n", k);
    return 0;
}

void wait_for_token(int fd_read, int fd_write, int process_number)
{
    int token;

    while (read(fd_read, &token, sizeof(token)) > 0)
    {
        // Verificar se acertou na probabilidade
        double lucky = rand() * 1.0 / RAND_MAX;
        if (lucky < probability)
        {
            printf("[p%d] lock on token (val = %d)\n", process_number, token);      // O processo deu lock
            sleep(timeout);                                                         // Tempo de lock
            printf("[p%d] unlock\n", process_number);                               // Unlock
        }

        // Aumenta o valor do token e envia-o para o proximo processo
        token++;
        write(fd_write, &token, sizeof(token));
    }
}

void open_and_wait_for_token(int j, int k, int l)
{
    int fd_read, fd_write;

    if (open_pipes(&fd_read, &fd_write, j, k, l) == -1)
        return;

    wait_for_token(fd_read, fd_write, k);
    close(fd_read);
    close(fd_write);
}

void open_send_and_wait_for_token(int j, int k, int l)
{
    int fd_read, fd_write;
    int token = 0; // Initial value of the token to be sent to the ring

    if (open_pipes_reverse(&fd_read, &fd_write, j, k, l) == -1)
        return;

    write(fd_write, &token, sizeof(token));                         // Envia o primeiro token

    wait_for_token(fd_read, fd_write, k);
    close(fd_read);
    close(fd_write);
}

void close_program()
{
    delete_pipes();                                                 // Apagar os pipes
    exit(0);
}

int main(int argc, char *argv[])
{

    if (validar_args(argc, argv) == -1)                             // Verificar se os argumentos sao validos
        return 0;

    if (create_pipes() == -1)                                       // Se não conseguir criar os pipes
        return 0;

    signal(SIGINT, close_program);                                  //Interrompe o processo, apaga os pipes e da exit

    for (int i = 1; i < num_processes; i++)
    {
        pid_t pid = fork();                                         // Cria novo processo filho
        if (pid == 0) {

            srand(getpid());                                        // Gera uma nova sequencia aleatoria para cada filho, com base no id

            if (i + 1 == num_processes) {                           // Se o processo for o ultimo
                open_and_wait_for_token(i, i + 1, 1);       // Ligar o ultimo processo com o primeiro
            }
            else {
                open_and_wait_for_token(i, i + 1, i + 2);   //Ligar com o processo seguinte
            }

            return 0;
        }
    }

    open_send_and_wait_for_token(num_processes, 1, 2);      // Processo pai, enviar o primeiro token

    return 0;
}
