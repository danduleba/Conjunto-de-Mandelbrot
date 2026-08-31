#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <limits.h>
#include  <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <pthread.h>
#define LOGIN "dad"

unsigned char *imagem;

int largura, altura, max_iteracoes, num_threads;
int proxima_linha;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    int inicio;
    int fim;
}Faixa;

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
    int iteracao = 0;

    c_real = -2.0+3.0 * coluna / largura;
    c_imag = -1.5 + 3.0 * linha / altura;
    
    while (iteracao<max_iteracoes && z_real*z_real +z_imag*z_imag<=4){
        novo_real = z_real*z_real - z_imag * z_imag + c_real;

        z_imag = 2.0 * z_real * z_imag + c_imag;
        z_real = novo_real;
        iteracao++;
    }
    return(unsigned char)(255.0 * iteracao / max_iteracoes);

}

void calcular_linhas(int inicio, int fim){
    int linha, coluna;

    for(linha= inicio; linha < fim; linha++){
        for(coluna = 0; coluna < largura; coluna++){
            imagem[(size_t)linha * largura+coluna]= calcular_pixel(linha,coluna);
        }
    }
}

void executar_serial(void){
    calcular_linhas(0, altura);
}

void executar_openmp(void){
    int linha, coluna;

    #pragma omp parallel for num_threads(num_threads) private(coluna)
    for(linha = 0; linha < altura; linha++){
        for(coluna = 0; coluna < largura; coluna++){
            imagem[(size_t)linha*largura+coluna]=calcular_pixel(linha,coluna);
        }
    }
}

void *rotina_pthreads1(void *argumento){
    Faixa *faixa =(Faixa *)argumento;

    calcular_linhas(faixa->inicio,faixa->fim);
    return NULL;
}

int executar_pthreads1(void){
    pthread_t *threads;
    Faixa *faixas;
    int criadas = 0;
    int sucesso = 1;
    int i;

    threads = calloc((size_t)num_threads, sizeof *threads);
    faixas = calloc((size_t)num_threads, sizeof *faixas);

    if(threads == NULL || faixas == NULL){
        free(threads);
        free(faixas);
        return 0;
    }
    for( i=0; i<num_threads; i++){
        faixas[i].inicio=(int)((size_t)i* altura/num_threads);
        faixas[i].fim=(int)((size_t)(i+1)* altura/num_threads);

        if(pthread_create(&threads[i],NULL, rotina_pthreads1, &faixas[i]) !=0){
            sucesso =0;
            break;
        }
        criadas++;
    }
    for(i=0;i<criadas; i++){
        if(pthread_join(threads[i], NULL) !=0){
            sucesso =0;
        }
    }
    free(threads);
    free(faixas);
    return sucesso;
}


rotina_pthreads2(void *argumento){
    int linha;
    (void)argumento;

    while(1){
        pthread_mutex_lock(&mutex);
        if(proxima_linha >= altura){
            pthread_mutex_unlock(&mutex);
            break;
        }
        linha = proxima_linha;
        proxima_linha++;
        calcular_linhas(linha, linha+1);
    }
    return NULL;
}

int salvar_imagem(const char*nome){
    FILE*arquivo;
    int linha, coluna;
    arquivo = fopen(nome, "w");
    if(arquivo == NULL){
        return 0;
    }
    for(linha =0; linha < altura; linha++){
        for(coluna =0; coluna<largura;coluna++){
            if(fprintf(arquivo, "%d%c", imagem[(size_t)linha * largura + coluna], coluna == largura - 1 ? '\n' : ' ') <0){
                fclose(arquivo);
                return 0;
            }
        }
    } 
    return fclose(arquivo) == 0;
}

double calcular_tempo(struct timespec inicio, struct timespec fim) {
    return fim.tv_sec - inicio.tv_sec +(fim.tv_nsec - inicio.tv_nsec) / 1000000000.0;
}

int salvar_tempos(double serial,double openmp,double pthreads1) {
    FILE *arquivo;
    arquivo = fopen("times.txt", "w");

    if (arquivo == NULL) {
        return 0;
    }

    if (fprintf(arquivo,"Serial: %.6fs\n""OpenMP: %.6fs\n""Pthreads1: %.6fs\n",serial, openmp, pthreads1) < 0) {
        fclose(arquivo);
        return 0;
    }

    return fclose(arquivo) == 0;
}

 int main(int argc, char *argv[]){
    struct timespec inicio,fim;
    double tempo_serial;
    double tempo_openmp;
    double tempo_pthreads1;
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
    tempo_serial = calcular_tempo(inicio, fim);
    

    if(!salvar_imagem("mandelbrot_" LOGIN "_serial.pgm")){
        fprintf(stderr, "Erro ao criar a imagem serial.\n");
        free(imagem);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    executar_openmp();
    clock_gettime(CLOCK_MONOTONIC, &fim);

    tempo_openmp = calcular_tempo(inicio, fim);

    if(!salvar_imagem("mandelbrot_" LOGIN "_openmp.pgm")){
        fprintf(stderr, "erro ao criar imagem openmp.\n");
        free(imagem);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &inicio);
    if (!executar_pthreads1()) {
        fprintf(stderr, "erro na execução pthreads1\n");
        free(imagem);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &fim);
    tempo_pthreads1 = calcular_tempo(inicio, fim);
    if (!salvar_imagem("mandelbrot_" LOGIN "_pthreads1.pgm")) {
        fprintf(stderr, "Erro ao criar a imagem Pthreads 1.\n");
        free(imagem);
        return 1;
    }
     if (!salvar_tempos(tempo_serial,tempo_openmp, tempo_pthreads1)){
        fprintf(stderr, "erro ao criar times.txt.\n");
        free(imagem);
        return 1;
     }
     free(imagem);
     return 0;
 }