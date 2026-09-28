#include <stdio.h>
#include <stdlib.h>
#include "heading.h"

int get_code_letter(unsigned char** pointer){ // указатель на указатель на байт строки (буква или префикс)
    unsigned char first_byte = **pointer;
    if (first_byte == 0xD0){ // буквы а-п
        (*pointer) ++;//указывает на код буквы
        int code = (**pointer - 0xB0) +1;
        (*pointer) ++;//указывает на следующий префикс
        return code;
    }
    if (first_byte == 0xD1){
        (*pointer) ++;//указывает на код буквы
        int code;
        if (**pointer == 0x91){// letter ё
            code = 33;
        } else{
            code = (**pointer - 0x80) +17;
        }
        (*pointer) ++;
        return code;
    }
    (*pointer)++;
    return -1; // не буква русского алфавита
}

void add_tree_node_to_trigrams(TreeNode* root, int* codes, int count_codes, arr_trigram** array_of_trigrams){
    int index;
    for (int i=0; i<=count_codes-3; i++){
        if (codes[i] < 0 || codes[i+1] < 0 || codes[i+2] < 0) continue; // некорректные индексы пропускаем
        index = 34*34*codes[i] + 34*codes[i+1]+codes[i+2];

        arr_trigram* new_tree_node_to_index = malloc(sizeof(arr_trigram));
        if (new_tree_node_to_index == NULL) {
            printf("ОШИБКА!! Не удалось выделить память для триграммы...\n");
            return;
        }
        // вставка в начало
        new_tree_node_to_index->tree_node = root;
        new_tree_node_to_index->next_node = array_of_trigrams[index];
        array_of_trigrams[index] = new_tree_node_to_index;
    }
}

void recursion_around_tree(TreeNode* root, arr_trigram** array_of_trigrams){
    if (root == NULL || root->word == NULL) return;

    unsigned char* uk_letter_in_word = (unsigned char*) root->word;// указатель на байт слова 
    int codes[128];// массив из кодов букв (по бокам нули)
    codes[0] = 0; // для пробела
    int count_codes = 0; // два пробела + количество букв в слове
    while (*uk_letter_in_word){
        int curr_code = get_code_letter(&uk_letter_in_word);
        if (curr_code != -1){
            count_codes++;
            codes[count_codes] = curr_code;
        }
    }
    count_codes++;
    codes[count_codes] = 0; // для пробела
    count_codes++;
    add_tree_node_to_trigrams(root, codes, count_codes, array_of_trigrams);

    recursion_around_tree(root->left, array_of_trigrams);
    recursion_around_tree(root->right, array_of_trigrams);
}

arr_trigram** create_arr_trigram(TreeNode* root){
    arr_trigram** array_of_trigrams = (arr_trigram**)calloc(39304, sizeof(arr_trigram*));

    if (array_of_trigrams == NULL) {
        printf("ОШИБКА!! Не удалось выделить память для массива триаграмм...\n");
        return NULL;
    }

    if (root != NULL) {
        recursion_around_tree(root, array_of_trigrams);
    } else {
        printf("ОШИБКА!! Корень дерева NULL!\n");
        return NULL;
    }
    return array_of_trigrams;
}
