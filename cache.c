#include <stdio.h>
#include <stdlib.h>

#include "cache.h"
#include "cpu.h"

// Criacao das caches
void criarCache(CACHE *cacheX, int tamanho){

    // Verifica se o ponteiro da cache é válido
    if(cacheX == NULL){
        printf("Erro: ponteiro da CACHE invalido.\n");
        return;
    }

    // Verifica se o tamanho é válido
    if(tamanho <= 0){
        printf("Erro: tamanho da CACHE invalido.\n");
        return;
    }

    // Aloca memória para os blocos da cache
    BlocoMemoria *novaMemoria = malloc(tamanho * sizeof(BlocoMemoria));

    if(novaMemoria == NULL){
        printf("Erro ao alocar memoria para criar CACHE.\n");
        return;
    }

    // Inicializa todos os blocos
    for(int i = 0; i < tamanho; i++){

        novaMemoria[i].atualizado = 0;
        novaMemoria[i].endBloco = -1;
        novaMemoria[i].custo = 0;
        novaMemoria[i].cacheHit = 0;

        // Inicializa as palavras do bloco
        for(int j = 0; j < PALAVRAS_POR_BLOCO; j++){
            novaMemoria[i].palavras[j] = 0;
        }
    }

    // Libera a memória anterior, caso exista
    free(cacheX->memoria);

    // Atualiza a estrutura da cache
    cacheX->memoria = novaMemoria;
    cacheX->tamanho = tamanho;
}

// Leitura e escrita
void setDadoCACHE(CACHE *cacheX, int endereco, BlocoMemoria conteudo){

    if(cacheX == NULL || cacheX->memoria == NULL){
        printf("Erro: CACHE invalida.\n");
        return;
    }

    if(endereco < 0 || endereco >= cacheX->tamanho){
        printf("Erro: endereco %d fora da CACHE.\n", endereco);
        return;
    }

    cacheX->memoria[endereco] = conteudo;
}

BlocoMemoria getDadoCACHE(CACHE *cacheX, int endereco){

    BlocoMemoria blocoInvalido = {0};
    blocoInvalido.endBloco = -1;

    if(cacheX == NULL || cacheX->memoria == NULL){
        printf("Erro: CACHE invalida.\n");
        return blocoInvalido;
    }

    if(endereco < 0 || endereco >= cacheX->tamanho){
        printf("Erro: endereco %d fora da CACHE.\n", endereco);
        return blocoInvalido;
    }

    return cacheX->memoria[endereco];
}

// Leitura e escrita e busca
BlocoMemoria buscarNasMemorias(Endereco e, int tamanhoRAM, RAM *ram, CACHE *cache1, CACHE *cache2, CACHE *cache3){
    int endCache = e.endBloco % cache1->tamanho;

    //retorna um bloco inválido caso nao ache em nenhuma memória
    BlocoMemoria blocoInvalido = {0};
    blocoInvalido.endBloco = -1;

    //procurar na cache 1 de forma direta
    if(getDadoCACHE(cache1, endCache).atualizado == 1 && getDadoCACHE(cache1, endCache).endBloco == e.endBloco){
        BlocoMemoria m = getDadoCACHE(cache1, endCache);
        m.custo += CUSTO_CACHE1;
        m.cacheHit = 1;
        setDadoCACHE(cache1, endCache, m);
        
        return m;
    }
    
    //procurar na cache 1 de forma associativa caso nao encontre
    for(int i = 0; i < cache1->tamanho; i++){
        if(getDadoCACHE(cache1, i).atualizado == 1 && getDadoCACHE(cache1, i).endBloco == e.endBloco){
            BlocoMemoria m = getDadoCACHE(cache1, i);
            m.custo += CUSTO_CACHE1;
            m.cacheHit = 1;
            setDadoCACHE(cache1, i, m);
            
            return m;
        }
    }

    endCache = e.endBloco % cache2->tamanho;

    //procurarna cache 2 de forma direta
    if(getDadoCACHE(cache2, endCache).atualizado == 1 && getDadoCACHE(cache2, endCache).endBloco == e.endBloco){
        BlocoMemoria blocoHit = getDadoCACHE(cache2, endCache);
        blocoHit.custo += CUSTO_CACHE2;
        blocoHit.cacheHit = 2;
        setDadoCACHE(cache2, endCache, blocoHit);

        BlocoMemoria m = moveCache2para1(endCache, cache2, cache1);

        if(m.endBloco != -1)
            return m;
        
        return blocoInvalido;
    }

    //procurar na cache 2 de forma associativa caso nao encontre
    for(int i = 0; i < cache2->tamanho; i++){
        if(getDadoCACHE(cache2, i).atualizado == 1 && getDadoCACHE(cache2, i).endBloco == e.endBloco){
            BlocoMemoria blocoHit = getDadoCACHE(cache2, i);
            blocoHit.custo += CUSTO_CACHE2;
            blocoHit.cacheHit = 2;
            setDadoCACHE(cache2, i, blocoHit);

            BlocoMemoria m = moveCache2para1(i, cache2, cache1);

            if(m.endBloco != -1)
                return m;
            
            return blocoInvalido;
        }
    }


    endCache = e.endBloco % cache3->tamanho;

    //procurarna cache 3 de forma direta
    if(getDadoCACHE(cache3, endCache).atualizado == 1 && getDadoCACHE(cache3, endCache).endBloco == e.endBloco){
        BlocoMemoria blocoHit = getDadoCACHE(cache3, endCache);
        blocoHit.custo += CUSTO_CACHE3;
        blocoHit.cacheHit = 3;
        setDadoCACHE(cache3, endCache, blocoHit);
        
        int endLastHis = moveCache3para2(endCache, cache3, cache2);
        
        if(endLastHis == -1)
            return blocoInvalido;

        BlocoMemoria m = moveCache2para1(endLastHis, cache2, cache1);

        if(m.endBloco != -1)
            return m;
        
        return blocoInvalido;
    }

    //procurar na cache 3 de forma associativa caso nao encontre
    for(int i = 0; i < cache3->tamanho; i++){
        if(getDadoCACHE(cache3, i).atualizado == 1 && getDadoCACHE(cache3, i).endBloco == e.endBloco){
            BlocoMemoria blocoHit = getDadoCACHE(cache3, i);
            blocoHit.custo += CUSTO_CACHE3;
            blocoHit.cacheHit = 3;
            setDadoCACHE(cache3, i, blocoHit);
            
            int endLastHis = moveCache3para2(i, cache3, cache2);
            
            if(endLastHis == -1)
                return blocoInvalido;

            BlocoMemoria m = moveCache2para1(endLastHis, cache2, cache1);

            if(m.endBloco != -1)
                return m;
            
            return blocoInvalido;
        }
    }

    for(int i = 0; i < tamanhoRAM; i++){
        if(getDadoRAM(ram, i).atualizado == 1 && getDadoRAM(ram, i).endBloco == e.endBloco){
            BlocoMemoria blocoHit = getDadoRAM(ram, i);
            blocoHit.custo += CUSTO_RAM;
            blocoHit.cacheHit = 4;
            setDadoRAM(ram, i, blocoHit);

            int endLastHis = moveRamparaCache3(i, ram, cache3);

            if(endLastHis != -1){    
                endLastHis = moveCache3para2(endLastHis, cache3, cache2);

                if(endLastHis != -1){
                    BlocoMemoria m = moveCache2para1(endLastHis, cache2, cache1);
                    
                    if(m.endBloco != -1){
                        return m;
                    }
                }
            }
            return blocoInvalido;
        }
    }

    return blocoInvalido;
}

BlocoMemoria moveCache2para1(int endHit, CACHE *cache2, CACHE *cache1){
    if(endHit < 0 || endHit >= cache2->tamanho) { // endHit inválido
        BlocoMemoria blocoInvalido = {0};
        blocoInvalido.endBloco = -1;
        return blocoInvalido;
    }
     
    BlocoMemoria blocoAUX;

    int endCache1 = getDadoCACHE(cache2, endHit).endBloco % cache1->tamanho;

    //endereço direto em que o endereço na cache1 esta desatualizado
    if(getDadoCACHE(cache1, endCache1).atualizado == 0){
        setDadoCACHE(cache1, endCache1, getDadoCACHE(cache2, endHit));

        BlocoMemoria blocoOrigem = getDadoCACHE(cache2, endHit);
        blocoOrigem.atualizado = 0;
        setDadoCACHE(cache2, endHit, blocoOrigem);
        return getDadoCACHE(cache1, endCache1);
    }

    // encontrar um endereco desatualizado
    for(int i = 0; i < cache1->tamanho; i++){
        if(getDadoCACHE(cache1, i).atualizado == 0){ 
            setDadoCACHE(cache1, i, getDadoCACHE(cache2, endHit));

            BlocoMemoria blocoOrigem = getDadoCACHE(cache2, endHit);
            blocoOrigem.atualizado = 0;
            setDadoCACHE(cache2, endHit, blocoOrigem);
            return getDadoCACHE(cache1, i);
        }
    }
    // encontrar um endereco que o custo seja maior que o de cache 2
    for(int i = 0; i < cache1->tamanho; i++){
        if(getDadoCACHE(cache1, i).custo < 44440){ 
            blocoAUX = getDadoCACHE(cache1, i);
            setDadoCACHE(cache1, i, getDadoCACHE(cache2, endHit));
            setDadoCACHE(cache2, endHit, blocoAUX);
            return getDadoCACHE(cache1, i);
        }
    }

    //se não é melhor deixar onde o endereço será procurado primeiramente
    blocoAUX = getDadoCACHE(cache1, endCache1);
    setDadoCACHE(cache1, endCache1, getDadoCACHE(cache2, endHit));
    setDadoCACHE(cache2, endHit, blocoAUX);
    return getDadoCACHE(cache1, endCache1);   
}

//retorna onde foi alocado o bloco de memoria, a posicao na cache 2
int moveCache3para2(int endHit, CACHE *cache3, CACHE *cache2){
    if(endHit < 0 || endHit >= cache3->tamanho) // endHit inválido
        return -1;
    
    BlocoMemoria blocoAUX;

    int endcache2 = getDadoCACHE(cache3, endHit).endBloco % cache2->tamanho;

    //endereço direto em que o endereço na cache2 esta desatualizado
    if(getDadoCACHE(cache2, endcache2).atualizado == 0){
        setDadoCACHE(cache2, endcache2, getDadoCACHE(cache3, endHit));

        BlocoMemoria blocoOrigem = getDadoCACHE(cache3, endHit);
        blocoOrigem.atualizado = 0;
        setDadoCACHE(cache3, endHit, blocoOrigem);
        return endcache2;
    }

    // encontrar um endereco desatualizado
    for(int i = 0; i < cache2->tamanho; i++){
        if(getDadoCACHE(cache2, i).atualizado == 0){ 
            setDadoCACHE(cache2, i, getDadoCACHE(cache3, endHit));

            BlocoMemoria blocoOrigem = getDadoCACHE(cache3, endHit);
            blocoOrigem.atualizado = 0;
            setDadoCACHE(cache3, endHit, blocoOrigem);
            return i;
        }
    }
    // encontrar um endereco que o custo seja maior que o de cache 2
    for(int i = 0; i < cache2->tamanho; i++){
        if(getDadoCACHE(cache2, i).custo < 44440){ 
            blocoAUX = getDadoCACHE(cache2, i);
            setDadoCACHE(cache2, i, getDadoCACHE(cache3, endHit));
            setDadoCACHE(cache3, endHit, blocoAUX);

            return i;
        }
    }

    //se não é melhor deixar onde o endereço será procurado primeiramente
    blocoAUX = getDadoCACHE(cache2, endcache2);
    setDadoCACHE(cache2, endcache2, getDadoCACHE(cache3, endHit));
    setDadoCACHE(cache3, endHit, blocoAUX);
    return endcache2;   
}

//retorna onde foi alocado o bloco de memoria, a posicao na cache 3
int moveRamparaCache3(int endHit, RAM *ram, CACHE *cache3){
    if(endHit < 0 || endHit >= ram->tamanho) // endHit inválido
        return -1;
    
    BlocoMemoria blocoAUX;

    int endcache3 = getDadoRAM(ram, endHit).endBloco % cache3->tamanho;

    //endereço direto em que o endereço na cache3 esta desatualizado
    if(getDadoCACHE(cache3, endcache3).atualizado == 0){
        setDadoCACHE(cache3, endcache3, getDadoRAM(ram, endHit));

        BlocoMemoria blocoOrigem = getDadoRAM(ram, endHit);
        blocoOrigem.atualizado = 0;
        setDadoRAM(ram, endHit, blocoOrigem);
        return endcache3;
    }

    // encontrar um endereco desatualizado
    for(int i = 0; i < cache3->tamanho; i++){
        if(getDadoCACHE(cache3, i).atualizado == 0){ 
            setDadoCACHE(cache3, i, getDadoRAM(ram, endHit));

            BlocoMemoria blocoOrigem = getDadoRAM(ram, endHit);
            blocoOrigem.atualizado = 0;
            setDadoRAM(ram, endHit, blocoOrigem);
            return i;
        }
    }
    // encontrar um endereco que o custo seja maior que o de cache 2
    for(int i = 0; i < cache3->tamanho; i++){
        if(getDadoCACHE(cache3, i).custo < 44440){ 
            blocoAUX = getDadoCACHE(cache3, i);
            setDadoCACHE(cache3, i, getDadoRAM(ram, endHit));
            setDadoRAM(ram, endHit, blocoAUX);

            return i;
        }
    }

    //se não é melhor deixar onde o endereço será procurado primeiramente
    blocoAUX = getDadoCACHE(cache3, endcache3);
    setDadoCACHE(cache3, endcache3, getDadoRAM(ram, endHit));
    setDadoRAM(ram, endHit, blocoAUX);
    return endcache3;   
}

/* Utilidades */
void imprimirCache(CACHE *cacheX){
    if(cacheX == NULL)
        return;

    printf("\n\nTamanho da memória cache: %d", cacheX->tamanho);

    for(int i = 0; i < cacheX->tamanho; i++) {
        printf("\nEnd Bloco: %d", getDadoCACHE(cacheX, i).endBloco);
        printf(" Hit: %d", getDadoCACHE(cacheX, i).cacheHit);
        printf(" Custo: %d", getDadoCACHE(cacheX, i).custo);
        printf(" Atualizado: %d", getDadoCACHE(cacheX, i).atualizado);
    }
}

void liberarCache(CACHE *cacheX){
    if(cacheX == NULL)
        return;

    free(cacheX->memoria);

    cacheX->memoria = NULL;
    cacheX->tamanho = 0;
}