#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){
    pid_t pid;                                                                  // Id do processo
    for(int i = 1; i < argc; i++) {
        if ((pid = fork()) == -1) {                                             // Se a criação do processo filho não foi feita com sucesso
            perror("fork");                                                     // Print do erro
            return EXIT_FAILURE;
        }
        else if (pid == 0) {                                                    // Novo processo filho
            printf("[%i] converting %s...\n", getpid(), argv[i]);               // Print do ficheiro a ser convertido e do pid (process id)
            char *refactor= strcat(strtok(strdup(argv[i]),"."),".epub");        // Retira o que está para frente do "." e acrescenta '.epub' ao nome do ficheiro
            char *arguments[]={"pandoc",argv[i],"-o",refactor,NULL};
            execvp("pandoc",arguments);
        }
        else {                                                                  // Processo pai
            if (waitpid(pid, NULL, 0) == -1)                                    // Esperar pelo processo filho
            {
                perror("waitpid");                                              // Print do erro
                return EXIT_FAILURE;
            }
        }
    }
    char *to_zip[]={"zip","-R","ebooks","*.epub",NULL};
    execvp("zip",to_zip);
    printf("\n");
    return EXIT_SUCCESS;
}