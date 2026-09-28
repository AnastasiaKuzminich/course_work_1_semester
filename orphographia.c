#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heading.h"

TreeNode* search_token_in_tree(TreeNode* root, const char* word){
    if (root == NULL || word == NULL){
        return NULL;
    }
    if (strcmp(root->word, word) == 0){
        return root;
    }
    if (strcmp(root->word, word)>0){
        return search_token_in_tree(root->left, word);
    } else{
        return search_token_in_tree(root->right, word);
    }
}

int check_each_token(TreeNode* root, token* arr_tokens, int tokens_count, mistake** arr_mistakes, int* mistakes_count, int* mistakes_capacity){
    for (int i = 0; i<tokens_count; i++){
        if (arr_tokens[i].length == 0) continue;
        char* word = arr_tokens[i].lowercase_word;

        TreeNode* curr_node = search_token_in_tree(root, word);
        if (curr_node == NULL){ //слово не найдено в словаре
            // Расширение памяти при необходимости
            if (*mistakes_count >= *mistakes_capacity){
                int new_capacity = (*mistakes_capacity == 0) ? 4 : *mistakes_capacity * 2;
                mistake* temp = realloc(*arr_mistakes, new_capacity * sizeof(mistake));
                if (temp == NULL){
                    return 1; // Ошибка памяти
                } 
                *arr_mistakes = temp;
                *mistakes_capacity = new_capacity;
            }
            int mistake_idx = *mistakes_count;
             
            (*arr_mistakes)[mistake_idx].lowercase_word = strdup(word);
            (*arr_mistakes)[mistake_idx].original_word = strdup (arr_tokens[i].original_word);
            if ((*arr_mistakes)[mistake_idx].lowercase_word == NULL ||
                (*arr_mistakes)[mistake_idx].original_word == NULL) {
                printf("ОШИБКА!! Ошибка выделения памяти для слов в массиве ошибок...");
                return 1;
            }
            (*arr_mistakes)[mistake_idx].type_mistake = 0;
            (*arr_mistakes)[mistake_idx].count_corrections = 0;
            (*arr_mistakes)[mistake_idx].arr_corrections = malloc(MAX_COUNT_CORRECTIONS * sizeof(char*));
            
            if ((*arr_mistakes)[mistake_idx].arr_corrections == NULL) {
                printf("ОШИБКА!! Ошибка выделения памяти для исправлений в массиве ошибок...");
                return 1;
            }
            (*mistakes_count)++;
        } else { // слово найдено
            arr_tokens[i].found_in_dict = 1;
            arr_tokens[i].node = curr_node;
        }
    }
    
    return 0;
}
