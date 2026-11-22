
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <mpi.h>

/* Conta os primos dentro de um intervalo */
long long contar_primos(int inicio, int fim,
                        int *primos_base, int qtd_base) {

    if (inicio > fim) {
        return 0;
    }

    int tamanho = fim - inicio + 1;

    unsigned char *primo = malloc((size_t)tamanho);

    if (primo == NULL) {
        fprintf(stderr, "Erro ao alocar memoria.\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    memset(primo, 1, (size_t)tamanho);

    /* Elimina os multiplos dos primos basicos */
    for (int i = 0; i < qtd_base; i++) {
        long long p = primos_base[i];

        long long primeiro = ((long long)inicio + p - 1) / p * p;

        if (primeiro < p * p) {
            primeiro = p * p;
        }

        for (long long j = primeiro; j <= fim; j += p) {
            primo[j - inicio] = 0;
        }
    }

    long long quantidade = 0;

    for (int i = 0; i < tamanho; i++) {
        if (primo[i]) {
            quantidade++;
        }
    }

    free(primo);

    return quantidade;
}

int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);

    int rank, num_processos;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processos);

    if (argc != 2) {
        if (rank == 0) {
            printf("Uso: mpirun -np P ./paralelo N\n");
        }

        MPI_Finalize();
        return 1;
    }

    char *fim_entrada;
    long limite = strtol(argv[1], &fim_entrada, 10);

    if (argv[1][0] == '\0' || *fim_entrada != '\0' ||
        limite < 2 || limite > INT_MAX) {

        if (rank == 0) {
            printf("Erro: informe um limite entre 2 e %d.\n",
                   INT_MAX);
        }

        MPI_Finalize();
        return 1;
    }

    int N = (int)limite;

    MPI_Barrier(MPI_COMM_WORLD);
    double inicio_tempo = MPI_Wtime();

    int *primos_base = NULL;
    int qtd_base = 0;

    int intervalo_inicio, intervalo_fim;

    int limite_base = (int)sqrt((double)N);

    if (rank == 0) {

        /* Encontra os primos ate a raiz de N */
        unsigned char *base = malloc((size_t)limite_base + 1);

        if (base == NULL) {
            MPI_Abort(MPI_COMM_WORLD, 1);
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

        primos_base = malloc(
            ((size_t)limite_base + 1) * sizeof(int)
        );

        if (primos_base == NULL) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        for (int p = 2; p <= limite_base; p++) {
            if (base[p]) {
                primos_base[qtd_base++] = p;
            }
        }

        free(base);

        /* Divide os numeros entre os processos */
        long long total = (long long)N - 1;

        for (int destino = 1; destino < num_processos; destino++) {

            int dados[3];

            dados[0] = (int)(
                2 + total * destino / num_processos
            );

            dados[1] = (int)(
                2 + total * (destino + 1) / num_processos - 1
            );

            dados[2] = qtd_base;

            MPI_Send(dados, 3, MPI_INT, destino,
                     0, MPI_COMM_WORLD);

            MPI_Send(primos_base, qtd_base, MPI_INT,
                     destino, 1, MPI_COMM_WORLD);
        }

        intervalo_inicio = 2;

        intervalo_fim = (int)(
            2 + total / num_processos - 1
        );

    } else {

        /* Recebe seu intervalo e os primos basicos */
        int dados[3];

        MPI_Recv(dados, 3, MPI_INT, 0, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        intervalo_inicio = dados[0];
        intervalo_fim = dados[1];
        qtd_base = dados[2];

        primos_base = malloc(
            (size_t)qtd_base * sizeof(int)
        );

        if (primos_base == NULL) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        MPI_Recv(primos_base, qtd_base, MPI_INT, 0, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    /* Cada processo calcula sua parte */
    long long quantidade_local = contar_primos(
        intervalo_inicio,
        intervalo_fim,
        primos_base,
        qtd_base
    );

    if (rank == 0) {

        long long quantidade_total = quantidade_local;

        /* Recebe os resultados dos trabalhadores */
        for (int origem = 1; origem < num_processos; origem++) {

            long long resultado;

            MPI_Recv(&resultado, 1, MPI_LONG_LONG,
                     origem, 2, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);

            quantidade_total += resultado;
        }

        double fim_tempo = MPI_Wtime();

        printf("===== CRIVO DE ERATOSTENES MPI =====\n");
        printf("Limite: %d\n", N);
        printf("Processos: %d\n", num_processos);
        printf("Quantidade de primos: %lld\n", quantidade_total);
        printf("Tempo de execucao: %.6f segundos\n",
               fim_tempo - inicio_tempo);

    } else {

        /* Envia a contagem ao mestre */
        MPI_Send(&quantidade_local, 1, MPI_LONG_LONG,
                 0, 2, MPI_COMM_WORLD);
    }

    free(primos_base);

    MPI_Finalize();

    return 0;
}
