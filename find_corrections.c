#include <stdlib.h>
#include <string.h>
#include "heading.h"

int get_string_length(const char* str) {
    if (str == NULL) {
        return 0;
    }
    unsigned char* pointer = (unsigned char*)str;
    int length = 0;
    while (*pointer != '\0') {
        unsigned char first_byte = *pointer;
        if (first_byte == 0xD0) {
            // Русские буквы (А-Я + а-п)
            if (*(pointer + 1) == '\0') {
                break; // Неполный UTF-8 символ
            }
            pointer += 2;
            length++;
        }
        else if (first_byte == 0xD1) {
            // Русские буквы (р-я)
            if (*(pointer + 1) == '\0') {
                break; // Неполный UTF-8 символ
            }
            pointer += 2;
            length++;
        }
        else if (first_byte == '-') {
            // Дефис
            pointer++;
            length++;
        }
        else {
            // Недопустимый символ (!!!!не должен встречаться)
            pointer++;
        }
    }
    return length;
}

int min_from_three(int a, int b, int c){
    if (a<b){
        if (a<c) return a; else return c;
    }else {
        if(b>c) return c; else return b;
    }
}

int* word_to_arr_codes(const char* w, int len){
    unsigned char* uk_letter_in_word = (unsigned char*) w;// указатель на байи слова 
    int* codes = (int*) malloc(len*sizeof(int));// массив из кодов букв ( по бокам нули)
    if (codes == NULL) return NULL;
    int i = 0;
    while (*uk_letter_in_word && i<len){
        int curr_code = get_code_letter(&uk_letter_in_word);
        if (curr_code != -1){
            codes[i] = curr_code; 
            i++;   
        }
    }
    return codes; 
}

int find_distance(const char* word_cor, const char* word_er){
    int len_cor = get_string_length(word_cor); //vertical, lines
    int len_er = get_string_length(word_er);// horizontal, columns
    int matrix[len_cor+1][len_er+1];

    for (int i = 0; i<=len_cor; i++) matrix[i][0] = i;
    for (int i = 0; i<=len_er; i++) matrix[0][i] = i;
    int* arr_cor = word_to_arr_codes(word_cor, len_cor);
    int* arr_er = word_to_arr_codes(word_er, len_er);

    for (int r=1; r<=len_cor;r++){
        for (int c=1; c<=len_er; c++){
            int cost = arr_cor[r-1] == arr_er[c-1] ? 0 : 1;
            
            matrix[r][c] = min_from_three(  matrix[r-1][c-1] + cost, // замена 
                                            matrix[r][c-1] + 1, // вставка 
                                            matrix[r-1][c] +1 ); // удаление
            
            if (r>1 && c>1 && arr_cor[r-1] == arr_er[c-2] && arr_cor[r-2] == arr_er[c-1]){
                int transposition_cost = matrix[r-2][c-2] + 1;
                if (transposition_cost < matrix[r][c]){
                    matrix[r][c] = transposition_cost;
                }
            }
        }
    }
    free(arr_cor);
    free(arr_er);
    return matrix[len_cor][len_er];
}

int if_tree_node_in_temp_arr(TreeNode* tree, temp_variants_corr* all_temp_variants, int count_temp_variants){
    for (int i=0; i<count_temp_variants;i++){
        if (tree == all_temp_variants[i].tree_node){
            return 1; // да, такой уже есть
        }
    }
    return 0;
}

void add_tree_nodes_to_temp_variants(int* codes, int count_codes, temp_variants_corr* all_temp_variants, int* count_temp_variants, arr_trigram** triagrams ){
    int index;
    for (int i=0; i<=count_codes-3; i++){
        if (codes[i] < 0 || codes[i+1] < 0 || codes[i+2] < 0) continue; // кривые индексы пропускаем
        index = 34*34*codes[i] + 34*codes[i+1]+codes[i+2];
        if (index < 0 || index >= 39304) continue;
        arr_trigram* triagram_node_of_index = triagrams[index];
        while (triagram_node_of_index != NULL && triagram_node_of_index->tree_node != NULL){
            if (*count_temp_variants >=COUNT_TEMP_CORRECTIONS) return;
            if (!if_tree_node_in_temp_arr(triagram_node_of_index->tree_node, all_temp_variants, *count_temp_variants)){
                all_temp_variants[*count_temp_variants].tree_node = triagram_node_of_index->tree_node;
                (*count_temp_variants) ++;
            }
            triagram_node_of_index = triagram_node_of_index->next_node;
        }   
    }
}

void fulfill_all_variants(mistake* node_mistake, char* word, arr_trigram** triagrams, temp_variants_corr* all_temp_variants){
    if (word == NULL || triagrams == NULL || all_temp_variants == NULL) return;
    node_mistake->count_corrections = 0; 
    // из слова сделали массив кодов для каждой буквы
    unsigned char* uk_letter_in_word = (unsigned char*) word;// указатель на байи слова 
    int codes[128];// массив из кодов букв ( по бокам нули)
    int count_codes = 0;
    while (*uk_letter_in_word){
        int curr_code = get_code_letter(&uk_letter_in_word);
        if (curr_code != -1){
            codes[count_codes] = curr_code;    
            count_codes++;
        }
    }
    
    int count_temp_variants = 0;
    // для каждого кода зайти в соответсвующий индекс массива триаграмм, вытянуть все связыне узлы дерева из него(если нет в это 1000, то добавить этот узел дерева)
    add_tree_nodes_to_temp_variants(codes, count_codes, all_temp_variants, &count_temp_variants, triagrams);
    
    // для каждого из count_temp_variants узлов проверить расстояние и занести это значение
    for (int i=0; i<count_temp_variants; i++){
        int distance = find_distance(all_temp_variants[i].tree_node->word, word);
        all_temp_variants[i].distance = distance;
    }
    int curr_distance = 1;
    while (node_mistake->count_corrections < MAX_COUNT_CORRECTIONS && curr_distance <= MAX_DISTANCE){
        for (int i = 0; i<count_temp_variants;i++){
            if (node_mistake->count_corrections < MAX_COUNT_CORRECTIONS && all_temp_variants[i].distance == curr_distance){
                node_mistake->arr_corrections[node_mistake->count_corrections] = strdup(all_temp_variants[i].tree_node->word);
                node_mistake->count_corrections++;
            }
        }
        curr_distance++;
    }
    return;

}

void collect_corrections(mistake* node_mistake, TreeNode* root, arr_trigram** triagrams){
    if (node_mistake==NULL || root==NULL) return;
    temp_variants_corr* all_temp_variants = (temp_variants_corr*)malloc(COUNT_TEMP_CORRECTIONS*sizeof(temp_variants_corr));
    if (all_temp_variants == NULL) return;
    fulfill_all_variants(node_mistake, node_mistake->lowercase_word, triagrams, all_temp_variants);
    
    free(all_temp_variants);
}

void find_orpho_corrections(mistake** arr_mistakes, int count_mistakes, TreeNode* root, arr_trigram** triagrams){
    if (count_mistakes == 0 || arr_mistakes == NULL || root==NULL) return;
    for (int i = 0; i < count_mistakes; i++){
        if ((*arr_mistakes)[i].type_mistake != 0) continue;//не орфографическая ошибка
        collect_corrections(&(*arr_mistakes)[i], root, triagrams);
    }
    return;
}