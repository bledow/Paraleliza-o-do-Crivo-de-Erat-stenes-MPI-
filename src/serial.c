#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <time.h>

/* Mede o tempo de execucao */
double tempo_atual(void) {
    struct timespec tempo;
    clock_gettime(CLOCK_MONOTONIC, &tempo);

    return tempo.tv_sec + tempo.tv_nsec / 1e9;
}

/* Conta os primos dentro de um intervalo (suporta > 2 bilhoes) */
long long contar_primos(long long inicio, long long fim,
                        int *primos_base, int qtd_base) {

    if (inicio > fim) {
        return 0;
    }

    long long tamanho = fim - inicio + 1;

    unsigned char *primo = malloc((size_t)tamanho);

    if (primo == NULL) {
        fprintf(stderr, "Erro ao alocar memoria (%lld bytes).\n", tamanho);
        exit(1);
    }

    memset(primo, 1, (size_t)tamanho);

    /* Elimina os multiplos dos primos basicos */
    for (int i = 0; i < qtd_base; i++) {
        long long p = primos_base[i];

        long long primeiro = (inicio + p - 1) / p * p;

        if (primeiro < p * p) {
            primeiro = p * p;
        }

        for (long long j = primeiro; j <= fim; j += p) {
            primo[j - inicio] = 0;
        }
    }

    long long quantidade = 0;

    for (long long i = 0; i < tamanho; i++) {
        if (primo[i]) {
            quantidade++;
        }
    }

    free(primo);

    return quantidade;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Uso: ./serial N\n");
        return 1;
    }

    char *fim_entrada;
    long long N = strtoll(argv[1], &fim_entrada, 10);

    if (argv[1][0] == '\0' || *fim_entrada != '\0' ||
        N < 2 || N > 100000000000LL) {

        printf("Erro: informe um limite entre 2 e 100000000000.\n");
        return 1;
    }

    double inicio_tempo = tempo_atual();

    /* Encontra os primos ate a raiz de N */
    int limite_base = (int)sqrtl((long double)N);

    unsigned char *base = malloc((size_t)limite_base + 1);

    if (base == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        return 1;
    }

    memset(base, 1, (size_t)limite_base + 1);
    base[0] = 0;
    base[1] = 0;

    for (int p = 2; p <= limite_base / p; p++) {
        if (base[p]) {
            for (int j = p * p; j <= limite_base; j += p) {
                base[j] = 0;
            }
        }
    }

    int *primos_base = malloc(
        ((size_t)limite_base + 1) * sizeof(int)
    );

    if (primos_base == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        free(base);
        return 1;
    }

    int qtd_base = 0;

    for (int p = 2; p <= limite_base; p++) {
        if (base[p]) {
            primos_base[qtd_base++] = p;
        }
    }

    free(base);

    /* Processa todo o intervalo em um unico processo */
    long long quantidade = contar_primos(
        2, N, primos_base, qtd_base
    );

    double fim_tempo = tempo_atual();

    printf("===== CRIVO DE ERATOSTENES SERIAL =====\n");
    printf("Limite: %lld\n", N);
    printf("Quantidade de primos: %lld\n", quantidade);
    printf("Tempo de execucao: %.6f segundos\n",
           fim_tempo - inicio_tempo);

    free(primos_base);

    return 0;
}