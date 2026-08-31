#ifndef SIGNAL_PROCESSING_H
#define SIGNAL_PROCESSING_H

typedef struct {
    double valor;
    double tiempo;
} Pico;

typedef struct {
    int num_picos;
    Pico *picos;
} ResultadoPicos;

void changeSign(double *vector);

ResultadoPicos encontrar_picos(
    double *datos,
    double *tiempos,
    int tamano
);

Pico *detectarP25(
    double *signal,
    double *time,
    int tamano
);

Pico *detectarN20(
    double *signal,
    double *time,
    int tamano,
    Pico *picosP25
);

#endif