# Conjunto de Mandelbrot

Programa em C que gera uma representação do conjunto de Mandelbrot e compara quatro formas de realizar o cálculo:

- Serial;
- OpenMP;
- Pthreads 1, com divisão fixa das linhas;
- Pthreads 2, com divisão dinâmica usando mutex.

As quatro versões produzem a mesma imagem.

## Compilar

```bash
make
```

## Executar

```bash
./mandelbrot largura altura max_iteracoes num_threads
```

Exemplo:

```bash
./mandelbrot 800 600 1000 4
```

Os argumentos devem ser números inteiros positivos.

## Arquivos gerados

```text
mandelbrot_dad_serial.pgm
mandelbrot_dad_openmp.pgm
mandelbrot_dad_pthreads1.pgm
mandelbrot_dad_pthreads2.pgm
times.txt
```

Os arquivos `.pgm` contêm somente os valores dos pixels, sem cabeçalho. O `times.txt` registra o tempo de cálculo das quatro versões.

## Limpar

```bash
make clean
```

## Repositório

[GitHub](https://github.com/danduleba/Conjunto-de-Mandelbrot)
