#include <stdlib.h>
#include "heading.h"

void free_dict_array(DictArray *arr) {
    if (!arr || !arr->entries) return;
    for (int i = 0; i < arr->count; i++) {
        free(arr->entries[i].word);
        if (arr->entries[i].grammems_list) {
            for (int j = 0; j < arr->entries[i].grammems_count; j++) {
                free(arr->entries[i].grammems_list[j]);
            }
            free(arr->entries[i].grammems_list);
        }
    }
    free(arr->entries);
    arr->entries = NULL;
    arr->count = 0;
    return;
}

void free_tree(TreeNode *root) {
    if (!root) return;
    
    free_tree(root->left);
    free_tree(root->right);
    
    free(root->word);
    
    if (root->grammems) {
        for (int i = 0; i < root->omonyms_count; i++) {
            if (root->grammems[i]) {
                for (int j = 0; root->grammems[i][j] != NULL; j++) {
                    free(root->grammems[i][j]);
                }
                free(root->grammems[i]);
            }
        }
        free(root->grammems);
    }
    free(root->grammems_counts);
    free(root);
    return;
}


void free_trigrams_array(arr_trigram** triagrams, int size) {
    if (!triagrams) return;
    for (int i = 0; i < size; i++) {
        arr_trigram* current = triagrams[i];
        while (current) {
            arr_trigram* temp = current;
            current = current->next_node;
            free(temp); 
        }
    }
    free(triagrams);
    return;
}

void free_mistakes_array(mistake** arr, int mistakes_count) {
    if (arr == NULL || *arr == NULL) return;
    
    for (int i = 0; i < mistakes_count; i++) {
        free((*arr)[i].original_word);
        free((*arr)[i].lowercase_word);
        
        if ((*arr)[i].arr_corrections) {
            for (int j = 0; j < (*arr)[i].count_corrections; j++) {
                free((*arr)[i].arr_corrections[j]);
            }
            free((*arr)[i].arr_corrections);
        }
    }
    free(*arr);
    *arr = NULL;
    return;
}

void free_tokens(token *tokens, int token_count) {
    if (!tokens) return;
    
    for (int i = 0; i < token_count; i++) {
        free(tokens[i].original_word);
        free(tokens[i].lowercase_word);
    }
    free(tokens);
}