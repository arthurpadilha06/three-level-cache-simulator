#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "cache.h"

/* Verifica se o endereco da palavra e valido */
int enderecoValido(Endereco endereco){

    if(endereco.endBloco < 0 ||
       endereco.endPalavra < 0 ||
       endereco.endPalavra >= PALAVRAS_POR_BLOCO){

        printf("Erro: endereco invalido.\n");
        return 0;
    }

    return 1;
}


/* Soma o custo e conta o hit conforme o nivel onde o bloco foi encontrado */
void contabilizarAcesso(CPU *cpu, int nivel){

    switch(nivel){
        case 1: cpu->hitCache1++; cpu->custo += CUSTO_CACHE1; break;
        case 2: cpu->hitCache2++; cpu->custo += CUSTO_CACHE2; break;
        case 3: cpu->hitCache3++; cpu->custo += CUSTO_CACHE3; break;
        case 4: cpu->hitRAM++;    cpu->custo += CUSTO_RAM;    break;
    }
}


/* Procura na cache 1 a linha que contem o bloco (depois da UCM ele esta la) */
int posicaoNaCache1(CPU *cpu, int endBloco){

    for(int i = 0; i < cpu->cache1.tamanho; i++){
        BlocoMemoria b = getDadoCACHE(&cpu->cache1, i);

        if(b.atualizado == 1 && b.endBloco == endBloco)
            return i;
    }

    return -1;
}


/* UCM - leitura: busca o bloco em L1 -> L2 -> L3 -> RAM e devolve a palavra */
int lerPalavraUCM(CPU *cpu, RAM *ram, Endereco endereco, int *valor){

    if(valor == NULL || !enderecoValido(endereco))
        return 0;

    BlocoMemoria bloco = buscarNasMemorias(endereco, ram->tamanho, ram,
                                           &cpu->cache1, &cpu->cache2, &cpu->cache3);

    if(bloco.endBloco == -1){
        printf("Erro: bloco %d nao encontrado nas memorias.\n", endereco.endBloco);
        return 0;
    }

    contabilizarAcesso(cpu, bloco.cacheHit);

    *valor = bloco.palavras[endereco.endPalavra];

    return 1;
}


/* UCM - escrita: traz o bloco para a cache 1 e altera a palavra la (write-back:
   o valor so desce para L2/L3/RAM quando o bloco for retirado da cache 1) */
int escreverPalavraUCM(CPU *cpu, RAM *ram, Endereco endereco, int valor){

    if(!enderecoValido(endereco))
        return 0;

    BlocoMemoria bloco = buscarNasMemorias(endereco, ram->tamanho, ram,
                                           &cpu->cache1, &cpu->cache2, &cpu->cache3);

    if(bloco.endBloco == -1){
        printf("Erro: bloco %d nao encontrado nas memorias.\n", endereco.endBloco);
        return 0;
    }

    contabilizarAcesso(cpu, bloco.cacheHit);

    int pos = posicaoNaCache1(cpu, endereco.endBloco);

    if(pos == -1){
        printf("Erro: bloco %d nao esta na cache 1.\n", endereco.endBloco);
        return 0;
    }

    BlocoMemoria linha = getDadoCACHE(&cpu->cache1, pos);
    linha.palavras[endereco.endPalavra] = valor;
    setDadoCACHE(&cpu->cache1, pos, linha);

    return 1;
}


/* Mostra a tabela de resultados da execucao */
void imprimirResultados(CPU *cpu){

    int total = cpu->hitCache1 + cpu->hitCache2 + cpu->hitCache3 + cpu->hitRAM;

    if(total == 0){
        printf("Nenhum acesso a memoria.\n");
        return;
    }

    /* cada nivel so e consultado quando o anterior falhou */
    int acessos2 = total - cpu->hitCache1;
    int acessos3 = acessos2 - cpu->hitCache2;

    printf("\n%-8s %10s %10s %10s\n", "Memoria", "Hit", "Miss", "Taxa hit");
    printf("%-8s %10d %10d %9.2f%%\n", "Cache 1", cpu->hitCache1, total - cpu->hitCache1,
           100.0 * cpu->hitCache1 / total);
    printf("%-8s %10d %10d %9.2f%%\n", "Cache 2", cpu->hitCache2, acessos2 - cpu->hitCache2,
           acessos2 ? 100.0 * cpu->hitCache2 / acessos2 : 0.0);
    printf("%-8s %10d %10d %9.2f%%\n", "Cache 3", cpu->hitCache3, acessos3 - cpu->hitCache3,
           acessos3 ? 100.0 * cpu->hitCache3 / acessos3 : 0.0);
    printf("%-8s %10d\n", "RAM", cpu->hitRAM);
    printf("\nTotal de acessos: %d\n", total);
    printf("Tempo hipotetico de execucao: %lld\n", cpu->custo);
}


/* Inicializa a CPU */
void cpuInicializar(CPU *cpu){

    if(cpu == NULL)
        return;

    cpu->registrador1 = 0;
    cpu->registrador2 = 0;

    cpu->PC = 0;
    cpu->opcode = 0;
    cpu->custo = 0;

    cpu->hitCache1 = 0;
    cpu->hitCache2 = 0;
    cpu->hitCache3 = 0;
    cpu->hitRAM = 0;

    cpu->programa = NULL;
    cpu->tamanhoPrograma = 0;

    cpu->cache1.memoria = NULL;
    cpu->cache1.tamanho = 0;

    cpu->cache2.memoria = NULL;
    cpu->cache2.tamanho = 0;

    cpu->cache3.memoria = NULL;
    cpu->cache3.tamanho = 0;
}


/* Registradores */
void setRegistrador1(CPU *cpu, int valor){

    if(cpu != NULL)
        cpu->registrador1 = valor;
}


void setRegistrador2(CPU *cpu, int valor){

    if(cpu != NULL)
        cpu->registrador2 = valor;
}


int getRegistrador1(CPU *cpu){

    if(cpu == NULL)
        return 0;

    return cpu->registrador1;
}


int getRegistrador2(CPU *cpu){

    if(cpu == NULL)
        return 0;

    return cpu->registrador2;
}


/* Define o programa */
void setPrograma(CPU *cpu, Instrucao *programa, int tamanhoPrograma){

    if(cpu == NULL || programa == NULL || tamanhoPrograma <= 0){
        printf("Erro ao definir programa.\n");
        return;
    }

    cpu->programa = programa;
    cpu->tamanhoPrograma = tamanhoPrograma;
}


/* Executa o programa */
void iniciarCPU(CPU *cpu, RAM *ram, int tamanhoCache1, int tamanhoCache2, int tamanhoCache3){

    if(cpu == NULL || ram == NULL){
        printf("Erro: CPU ou RAM invalida.\n");
        return;
    }

    if(ram->memoria == NULL){
        printf("Erro: RAM nao inicializada.\n");
        return;
    }

    if(cpu->programa == NULL || cpu->tamanhoPrograma <= 0){
        printf("Erro: programa nao inicializado.\n");
        return;
    }

    /* Inicializa as tres caches */
    criarCache(&cpu->cache1, tamanhoCache1);
    criarCache(&cpu->cache2, tamanhoCache2);
    criarCache(&cpu->cache3, tamanhoCache3);

    if(cpu->cache1.memoria == NULL ||
       cpu->cache2.memoria == NULL ||
       cpu->cache3.memoria == NULL){

        printf("Erro ao inicializar as caches.\n");
        return;
    }

    cpu->PC = 0;
    cpu->opcode = 999;
    cpu->custo = 0;

    cpu->hitCache1 = 0;
    cpu->hitCache2 = 0;
    cpu->hitCache3 = 0;
    cpu->hitRAM = 0;

    while(cpu->opcode != -1){

        if(cpu->PC < 0 || cpu->PC >= cpu->tamanhoPrograma){
            printf("Erro: PC saiu dos limites do programa.\n");
            return;
        }

        /* FETCH */
        Instrucao inst = cpu->programa[cpu->PC];

        /* DECODE */
        cpu->opcode = inst.opcode;

        /* EXECUTE */
        switch(cpu->opcode){

            /* FINALIZAR */
            case -1:
                imprimirCache(&cpu->cache1);
                imprimirCache(&cpu->cache2);
                imprimirCache(&cpu->cache3);
                imprimirResultados(cpu);
                break;

            /* SOMA */
            case 0: {

                int valor1, valor2;

                if(!lerPalavraUCM(cpu, ram, inst.add1, &valor1))
                    return;

                if(!lerPalavraUCM(cpu, ram, inst.add2, &valor2))
                    return;

                int resultado = valor1 + valor2;

                if(!escreverPalavraUCM(cpu, ram, inst.add3, resultado))
                    return;

                cpu->registrador1 = resultado;

                break;
            }

            /* SUBTRACAO */
            case 1: {

                int valor1, valor2;

                if(!lerPalavraUCM(cpu, ram, inst.add1, &valor1))
                    return;

                if(!lerPalavraUCM(cpu, ram, inst.add2, &valor2))
                    return;

                int resultado = valor1 - valor2;

                if(!escreverPalavraUCM(cpu, ram, inst.add3, resultado))
                    return;

                cpu->registrador1 = resultado;

                break;
            }

            /* REGISTRADOR -> RAM */
            case 2: {

                /* o gerador coloca de 0 a 999 em add1:
                   par = registrador 1, impar = registrador 2 */
                if(inst.add1.endBloco % 2 == 0){

                    if(!escreverPalavraUCM(
                        cpu, ram, inst.add2, cpu->registrador1))
                        return;

                }
                else{

                    if(!escreverPalavraUCM(
                        cpu, ram, inst.add2, cpu->registrador2))
                        return;

                }

                break;
            }

            /* RAM -> REGISTRADOR */
            case 3: {

                int valor;

                if(!lerPalavraUCM(cpu, ram, inst.add2, &valor))
                    return;

                /* par = registrador 1, impar = registrador 2 */
                if(inst.add1.endBloco % 2 == 0)
                    cpu->registrador1 = valor;

                else
                    cpu->registrador2 = valor;

                break;
            }

            default:

                printf("Erro: opcode invalido: %d\n", cpu->opcode);
                return;
        }

        if(cpu->opcode != -1)
            cpu->PC++;
    }
}


/* Libera as caches da CPU */
void liberarCPU(CPU *cpu){

    if(cpu == NULL)
        return;

    liberarCache(&cpu->cache1);
    liberarCache(&cpu->cache2);
    liberarCache(&cpu->cache3);

    cpu->programa = NULL;
    cpu->tamanhoPrograma = 0;

    cpu->PC = 0;
    cpu->opcode = 0;

    cpu->registrador1 = 0;
    cpu->registrador2 = 0;
    cpu->custo = 0;
}