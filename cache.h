#ifndef CACHE_H
#define CACHE_H

#include "instrucao.h"
#include "ram.h"

#define TAM_PALAVRA 4

typedef struct{
    int palavra[TAM_PALAVRA];
    int status; // 0 desatualizado 1 para atualizado
} Memoria;


typedef struct {
	BlocoMemoria *memoria;
    int tamanho;
} CACHE;

/* Custos hipoteticos de cada acesso */
#define CUSTO_CACHE1 10
#define CUSTO_CACHE2 110
#define CUSTO_CACHE3 1110
#define CUSTO_RAM    11110000

// Criacao da CACHE
void criarCache(CACHE *cacheX, int tamanho);

// Leitura e escrita
void setDadoCACHE(CACHE *cacheX, int endereco, BlocoMemoria conteudo);
BlocoMemoria getDadoCACHE(CACHE *cacheX, int endereco);

// Leitura e escrita e busca
BlocoMemoria buscarNasMemorias(Endereco e, int tamanhoRAM, RAM *ram, CACHE *cache1, CACHE *cache2, CACHE *cache3);

// Movimentacao entre caches
BlocoMemoria moveCache2para1(int endHit, CACHE *cache2, CACHE *cache1);
int moveCache3para2(int endHit, CACHE *cache3, CACHE *cache2);
int moveRamparaCache3(int endHit, RAM *ram, CACHE *cache3);

/* Utilidades */
void imprimirCache(CACHE *cacheX);

void liberarCache(CACHE *cacheX);

#endif //CACHE_H