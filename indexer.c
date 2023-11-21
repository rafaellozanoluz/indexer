//importando as bibliotecas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>

//Cria as definições das constantes e seus valores máximos
#define BUFFER 2000
#define MAX_WORD_LEN 50
#define MAX_NUM_WORDS 300000


//Estrutura do tipo lista ligada que armazena os dados da palavra a contagem e um ponteiro para a próxima palavra
typedef struct word_count {
    char word[MAX_WORD_LEN];
    int count;
    struct word_count *next;
} word_count;

//Essa estrutura representa uma tabela hash
//Nela tem um ponteiro de tamanho máximo definido no define que é do tipo word_count
typedef struct hash_table {
    word_count *table[MAX_NUM_WORDS];
    int num_words;
} hash_table;


//Tentando implementar
typedef struct file_score {
    char nome[BUFFER];
    double score;
    double tfidf;
} file_score;

//declara as funções
void cria_tabela(char *filename, hash_table *ht);
void ordena_tabela(hash_table *ht);
void freq_function(hash_table *ht, int n);
void freq_word_function(char *filename, char *word, hash_table *ht);
int compare_file_scores(const void *a, const void *b);
int hash(char *str, int num_buckets);
bool ht_contains(hash_table *ht, char *word, int file_index);
void processWord(char* word);
void add_word(hash_table *ht, char *word);
int separar_argumentos_arquivo(char *str, char ***arquivos);
void quick_sort(word_count **vector, int start, int end);

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Modo de uso: ./program <funcao> <complemento> <arquivo1> [<arquivo2> ...]\n");
        return 1;
    }

    char *function = argv[1];
    char *complement = argv[2];
    char *filename = argv[3];
    int freq_n = 0;
    char *freq_word = NULL;
    char **search_term = NULL;
    int num_search_terms = 0;

    if (strcmp(function, "--freq") == 0) {
        freq_n = atoi(complement);
    } else if (strcmp(function, "--freq-word") == 0) {
        freq_word = complement;
        processWord(freq_word);
    } else {
        printf("Erro: funcao invalida '%s'\n", function);
        return 1;
    }

    hash_table ht;
    ht.num_words = 0;

    if (strcmp(function, "--freq") == 0) {
        printf("Frequencia das %d palavras mais comuns em %s:\n", freq_n, filename);
        cria_tabela(filename, &ht);
        ordena_tabela(&ht);
        freq_function(&ht, freq_n);
    } else if (strcmp(function, "--freq-word") == 0) {
        printf("Frequencia da palavra \"%s\" em %s:\n", freq_word, filename);
        freq_word_function(filename, freq_word, &ht);
    } else if (strcmp(function, "--search") == 0) {
        char **arquivos = malloc(sizeof(char*) * argc);
        int num_arquivos = 0;
        for (int i = 3; i < argc; i++) {
            if (access(argv[i], F_OK) != -1) {
                arquivos[num_arquivos] = argv[i];
                num_arquivos++;
            }
    }
    free(arquivos);
    }
    free(search_term);
    for (int i = 0; i < ht.num_words; i++) {
        free(ht.table[i]);
    }

    return 0;
}
//FUNCOES

void cria_tabela(char *filename, hash_table *ht){

    char *arqword;
    char buffer[BUFFER];

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erro: nao foi possivel abrir o arquivo: '%s'\n", filename);
        return;
    }

    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        arqword = strtok(buffer, " ");
        while (arqword != NULL) {
            processWord(arqword);
            add_word(ht, arqword);
            arqword = strtok(NULL, " ");
        }
    }
    fclose(file);
}

void quick_sort(word_count **vector, int start, int end) {
    // Verifica se o vetor tem pelo menos dois elementos
    if (start < end) {
        
        word_count *pivot = vector[end];
        
        int pivot_index = start;
        for (int i = start; i < end; i++) {
            if (vector[i]->count > pivot->count) {
                word_count *temp = vector[i];
                vector[i] = vector[pivot_index];
                vector[pivot_index] = temp;
                pivot_index++;
            }
        }
        vector[end] = vector[pivot_index];
        vector[pivot_index] = pivot;
        
        quick_sort(vector, start, pivot_index - 1);
        quick_sort(vector, pivot_index + 1, end);
    }
}

void ordena_tabela(hash_table *ht){
    // Cria um vetor de ponteiros para elementos da tabela hash
    word_count **vector = malloc(ht->num_words * sizeof(word_count *));
    int index = 0;
    for (int i = 0; i < ht->num_words; i++) {
        if (ht->table[i] != NULL) {
            vector[index] = ht->table[i];
            index++;
        }
    }
    // Chama a função quick_sort para ordenar o vetor
    quick_sort(vector, 0, ht->num_words - 1);
    // Copia os elementos do vetor de volta para a tabela hash
    for (int i = 0; i < ht->num_words; i++) {
        ht->table[i] = vector[i];
    }
    free(vector);
    
}

void freq_function(hash_table *ht, int n) {

    for (int i = 0; i < n && i < ht->num_words; i++) {
        printf("%d - %s: %d\n", i+1, ht->table[i]->word, ht->table[i]->count);
    }
}
//------------------------------------------------------------------------------------------------------------------//

void freq_word_function(char *filename, char *word, hash_table *ht) {

    char *arqword;
    char buffer[BUFFER];

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erro: nao foi possivel abrir o arquivo: '%s'\n", filename);
        return;
    }
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        arqword = strtok(buffer, " ");
        while (arqword != NULL) {
            processWord(arqword);
            if (strcmp(arqword, word) == 0) {
                add_word(ht, arqword);
            }
            arqword = strtok(NULL, " ");
        }
    }
    // Imprime a frequencia da palavra
    if (ht->num_words > 0) {
        printf("A palavra \"%s\" aparece %d vezes no arquivo.\n", word, ht->table[0]->count);
    } else {
        printf("A palavra \"%s\" nao aparece no arquivo.\n", word);
    }
    fclose(file);
}
//------------------------------------------------------------------------------------------------------------------//

int compare_file_scores(const void *a, const void *b) {
    file_score *fa = (file_score*) a;
    file_score *fb = (file_score*) b;
    return (fa->tfidf < fb->tfidf) - (fa->tfidf > fb->tfidf);
}
//------------------------------------------------------------------------------------------------------------------//

int hash(char *str, int num_buckets) {
    int hash_val = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        hash_val = hash_val ^ str[i];
    }
    return hash_val % num_buckets;
}
//------------------------------------------------------------------------------------------------------------------//

bool ht_contains(hash_table *ht, char *word, int file_index) {
    int hash_val = hash(word, ht->num_words);
    word_count *current = ht->table[hash_val];
    while (current != NULL) {
        if (strcmp(current->word, word) == 0 && current->count & (1 << file_index)) {
            return true;
        }
        current = current->next;
    }
    return false;
}
//------------------------------------------------------------------------------------------------------------------//


//------------------------------------------------------------------------------------------------------------------//

void processWord(char* word) {
    int i, j;
    for (i = 0, j = 0; word[i]; i++) {
        if (isalpha(word[i])) {
            word[j++] = tolower(word[i]);
        }
    }
    word[j] = '\0';
}
//------------------------------------------------------------------------------------------------------------------//

void add_word(hash_table *ht, char *word) {

    // processa a palavra
    processWord(word);

    // verifica se a palavra foi processada para uma string vazia
    if (strlen(word) <= 2) {
        return;
    }

    // procura pela palavra na tabela hash
    int index = -1;
    for (int i = 0; i < ht->num_words; i++) {
        if (strcmp(ht->table[i]->word, word) == 0) {
            index = i;
            break;
        }
    }

    // se a palavra já existe na tabela, incrementa a contagem
    if (index >= 0) {
        ht->table[index]->count++;
    }
    // caso contrário, adiciona a palavra à tabela
    else {
        word_count *wc = malloc(sizeof(word_count));
        strncpy(wc->word, word, MAX_WORD_LEN);
        wc->count = 1;
        ht->table[ht->num_words] = wc;
        ht->num_words++;
    }
}
