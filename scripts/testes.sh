
#!/bin/bash

set -e

mkdir -p resultados

ARQUIVO="resultados/tempos.csv"

echo "limite,processos,execucao,tempo" > "$ARQUIVO"

for N in 10000000 100000000 200000000
do
    echo "Testando limite: $N"

    for REP in 1 2 3 4 5
    do
        TEMPO=$(./serial "$N" |
            awk '/Tempo de execucao:/ {print $4}')

        echo "$N,0,$REP,$TEMPO" >> "$ARQUIVO"
    done

    for P in 1 2 4
    do
        echo "  MPI com $P processos"

        for REP in 1 2 3 4 5
        do
            TEMPO=$(mpirun -np "$P" ./paralelo "$N" |
                awk '/Tempo de execucao:/ {print $4}')

            echo "$N,$P,$REP,$TEMPO" >> "$ARQUIVO"
        done
    done
done

echo "Testes concluidos!"
echo "Resultados salvos em $ARQUIVO"
