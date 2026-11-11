
#ifndef CPU_H
#define CPU_H

#include "instrucao.h"
#include "ram.h"
#include "cache.h"

typedef struct {
    int PC;

    Instrucao *programa;
    int tamanhoPrograma;

    int opcode;
    long long custo;   // tempo hipotetico de execucao (soma dos custos de cada acesso)

    /* Estatisticas da UCM: onde cada acesso encontrou o bloco */
    long long int hitCache1;
    long long int hitCache2;
    long long int hitCache3;
    long long int hitRAM;

    int registrador1;
    int registrador2;

    CACHE cache1;
    CACHE cache2;
    CACHE cache3;

} CPU;


/* Inicializacao */
void cpuInicializar(CPU *cpu);

/* Registradores */
void setRegistrador1(CPU *cpu, int valor);
void setRegistrador2(CPU *cpu, int valor);

int getRegistrador1(CPU *cpu);
int getRegistrador2(CPU *cpu);

/* Programa */
void setPrograma(CPU *cpu, Instrucao *programa, int tamanhoPrograma);

/* Execucao */
void iniciarCPU(
    CPU *cpu,
    RAM *ram,
    int tamanhoCache1,
    int tamanhoCache2,
    int tamanhoCache3
);

/* UCM - Unidade de Controle de Memoria */
int lerPalavraUCM(CPU *cpu, RAM *ram, Endereco endereco, int *valor);
int escreverPalavraUCM(CPU *cpu, RAM *ram, Endereco endereco, int valor);

/* Resultados */
void imprimirResultados(CPU *cpu);

/* Liberacao */
void liberarCPU(CPU *cpu);

#endif
