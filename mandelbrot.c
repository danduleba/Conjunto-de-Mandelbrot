#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include  <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define LOGIN "dad"

unsigned char *imagem;

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

unsigned char calcular_pixel(int linha, int coluna){
    double c_real;
    double c_imag;
    double z_real = 0.0;
    double z_imag = 0.0;
    double novo_real;
    double iteracao = 0;

    if(largura == 1){
        c_real = -2.0;
    }
    else{
        c_real = -2.0+3.0 * coluna / (largura-1);
    }
    if(altura == 1){
        c_imag = 1.5;
    }
    else{
        c_imag = -1.5 + 3.0 * linha / (altura -1);
    }
    while (iteracao<max_iteracoes && z_real*z_real +z_imag*z_imag<=4){
        novo_real = z_real*z_real - z_imag * z_imag + c_real;

        z_imag = 2.0 * z_real * z_imag + c_imag;
        z_real = novo_real;
        iteracao++;
    }
    return(unsigned char)(255.0 * iteracao / max_iteracoes);

}
void executar_serial(void){
    int linha,coluna;

    for( linha = 0; linha<altura; linha++){
        for(coluna = 0; coluna < largura; coluna++){
            imagem[linha*largura+coluna] = calcular_pixel(linha, coluna);
        }
    }
}

int salvar_imagem(void){
    FILE*arquivo;
    int linha, coluna;
    arquivo = fopen("mandelbrot_" LOGIN "_serial.pgm", "w");
    if(arquivo == NULL){
        return 0;
    }
    for(linha =0; linha < altura; linha++){
        for(coluna =0; coluna<largura;coluna++){
            if(fprintf(arquivo, "%d%c", imagem[linha * largura + coluna], coluna == largura - 1 ? '\n' : ' ') <0){
                fclose(arquivo);
                return 0;
            }
        }
    } 
    return fclose(arquivo) == 0;
}

double calcular_tempo(struct  timespec inicio, struct timespec fim){
    return fim.tv_sec - inicio.tv_sec+(fim.tv_nsec-inicio.tv_nsec) / 100000000.0;

}
 int salvar_tempo(double tempo){
    FILE *arquivo;
    arquivo = fopen("times.txt", "w");
    if (arquivo == NULL){
        return 0;
    }
    if(fprintf(arquivo, "Serial: %.6fs\n", tempo) < 0){
        fclose(arquivo);
        return 0;
    }
    return fclose(arquivo)==0;
 }

 int main(int argc, char *argv[]){
    struct timespec inicio,fim;
    double tempo;
    size_t quantidade_pixels;
    if(argc != 5){
        fprintf(stderr, "Uso: %s largura altura max_iteracoes num_threads\n", argv[0]);
        return 1;
    }
    if(!ler_inteiro(argv[1], &largura) || !ler_inteiro(argv[2], &altura) || !ler_inteiro(argv[3], &max_iteracoes) || !ler_inteiro(argv[4], &num_threads)){
        fprintf(stderr, "Erro: use apenas inteiros posivtivos.\n");
        return 1;
    }
    if ((size_t)largura> SIZE_MAX / (size_t)altura){
        fprintf(stderr,"Erro: imagem muito grande");
        return 1;
    }
    quantidade_pixels = (size_t)largura * altura;
    imagem = malloc(quantidade_pixels * sizeof *imagem);

    if(imagem == NULL){
        fprintf(stderr, "Erro na alocação.\n");
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    executar_serial();
    clock_gettime(CLOCK_MONOTONIC, &fim);
    tempo = calcular_tempo(inicio, fim);

    if(!salvar_imagem()){
        fprintf(stderr, "Erro ao criar a imagem serial.\n");
        free(imagem);
        return 1;
    }
    if(!salvar_tempo(tempo)){
        fprintf(stderr, "erro ao criar o times.txt.\n");
        free(imagem);
        return 1;
    }
    free(imagem);
    return 0;
 }