#!/bin/bash

# Compila os programas
echo "Compilando serial.c e paralelo.c..."
gcc -O2 serial.c -o serial -lm || { echo "ERRO ao compilar serial.c"; exit 1; }
mpicc -O2 paralelo.c -o paralelo -lm || { echo "ERRO ao compilar paralelo.c"; exit 1; }


mkdir -p resultados
ARQUIVO="tempos.csv"

echo "limite,processos,execucao,tempo" > "$ARQUIVO"

for N in 500000000 1000000000 2000000000
do
    echo "=========================================="
    echo "Testando limite: $N"

    for REP in 1 2 3
    do
        echo -n "  [Serial] Execucao $REP/3... "
        SAIDA=$(./serial "$N" 2>&1)
        TEMPO=$(echo "$SAIDA" | awk '/Tempo de execucao:/ {print $4}')

        if [ -z "$TEMPO" ]; then
            echo "FALHOU!"
            echo "  Motivo: $SAIDA"
            exit 1
        fi

        echo "${TEMPO}s"
        echo "$N,0,$REP,$TEMPO" >> "$ARQUIVO"
    done

    for P in 1 2 4
    do
        echo "  [MPI - $P processo(s)]"

        for REP in 1 2 3
        do
            echo -n "    Execucao $REP/3... "
            SAIDA=$(mpirun --oversubscribe -np "$P" ./paralelo "$N" 2>&1)
            TEMPO=$(echo "$SAIDA" | awk '/Tempo de execucao:/ {print $4}')

            if [ -z "$TEMPO" ]; then
                echo "FALHOU!"
                echo "    Motivo: $SAIDA"
                exit 1
            fi

            echo "${TEMPO}s"
            echo "$N,$P,$REP,$TEMPO" >> "$ARQUIVO"
        done
    done
done

cp "$ARQUIVO" "resultados/tempos.csv"

echo "=========================================="
echo "Concluido com sucesso! Arquivo gerado: $(pwd)/$ARQUIVO"