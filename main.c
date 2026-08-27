#include <errno.h>
#include <limits.h>
#include  <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define LOGIN "dad"


int largura, altura, max_iteracoes, num_threads;

int ler_inteiro(const char *texto, int *valor){
    char *fim;
    long numero;

    errno = 0;
    numero = strtol(texto, &fim, 10);
    if(errno != 0 || *texto == '\0' || *fim != '\0' || numero < 1 || numero > INT_MAX){
        return 0;
    }
    *valor = (int)numero;
    return 1;
}

unsigned char calcular_pixel(int linnha, int coluna){
    double c_real;
    double c_imag;
    double z_real = 0.0;
    double z_imag = 0.0;
    double novo_real;
    double interacao = 0;

}