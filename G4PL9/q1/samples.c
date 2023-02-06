#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


long int get_file_size(char *filename) {
    FILE *fp = fopen(filename, "r");

    if (fp == NULL)
        return -1;

    if (fseek(fp, 0, SEEK_END) < 0) {
        fclose(fp);
        return -1;
    }

    long size = ftell(fp);                                                  //retorna o tamanho
    return size;
}
int main(int argc, char* argv[]){
    if (argc < 2){
        printf("usage: samples file numberfrags maxfragsize\n");
        return 0;
    }
    FILE *f;
    char *p;
    long size = strtol(argv[3],&p,10);                                      // Numero de caracteres por print
    long lines = strtol(argv[2],&p,10);                                     // Numero de prints
    f = fopen(argv[1], "r");
    long int tamanho = get_file_size(argv[1]);                      // Numero de letras no ficheiro
    fseek(f, 0, SEEK_SET);                                                  // Move o pointer para o inicio
    srandom(0);
    for (int i = 0; i < lines; i++) {
        long x = rand() % (tamanho - size);                                 // Índice aleatório do ficheiro
        fseek(f,x, SEEK_SET);                                               // Move o pointer para o índice aleatorio
        printf(">");
        for (int j = 0; j < size; j++) {                                    // Ciclo for para dar print aos caracteres seguintes (com tamanho 'size')
            char y = fgetc(f);
            if (y == '\n') printf(" ");                                     // Se chegar ao fim da linha dá um espaço
            else printf("%c", y);                                           // Print ao caractere
        }
        printf("<\n");

    }
    fclose(f);
    return 0;
}
