// tempos.h

#ifndef TEMPOS_H
#define TEMPOS_H

// Definição dos tempos de cada semáforo - PRINCIPAL
#define TEMPO_VERDE_MIN_PRINCIPAL 10
#define TEMPO_VERDE_MAX_PRINCIPAL 18
#define TEMPO_VERMELHO_MIN_PRINCIPAL 5
#define TEMPO_VERMELHO_MAX_PRINCIPAL 10

// Definição do servidor e porta
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5000  

// Limites de velocidade
#define LIMITE_VELOCIDADE_PRINCIPAL 80 // 80 km/h
#define LIMITE_VELOCIDADE_AUXILIAR 60 // 60 km/h

// Definição dos tempos de cada semáforo - Auxiliar
#define TEMPO_VERDE_MIN_AUXILIAR 5
#define TEMPO_VERDE_MAX_AUXILIAR 8
#define TEMPO_VERMELHO_MIN_AUXILIAR 10
#define TEMPO_VERMELHO_MAX_AUXILIAR 20


// Definição do tempo amarelo - AMBOS
#define TEMPO_AMARELO 2

#endif
