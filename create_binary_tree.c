#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heading.h"

TreeNode* create_node(const char *word, char **grammems_list, int grammems_count) {   
    TreeNode *node = (TreeNode*)malloc(sizeof(TreeNode));
    if (!node) {
        printf("ОШИБКА!! Не удалось выделить память для узла\n");
        return NULL;
    }
    node->word = strdup(word);
    if (!node->word) {
        printf("ОШИБКА!! Не удалось скопировать слово\n");
        free(node);
        return NULL;
    }
    node->omonyms_count = grammems_count;
    node->grammems = malloc(grammems_count * sizeof(char**));  // sizeof(char**)--указатель на массив строк 
    node->grammems_counts = malloc(grammems_count * sizeof(int));
    if (!node->grammems || !node->grammems_counts) {
        printf("ОШИБКА!! Не удалось выделить память для граммем\n");
        free(node->word);
        free(node);
        return NULL;
    }
    for (int i = 0; i < grammems_count; i++) { // для каждого омонима
        char *temp = strdup(grammems_list[i]);// Создание копии , чтобы разбить строку граммем по запятым       
        int count = 1;
        for (char *p = temp; *p; p++) {
            if (*p == ',') count++;//после запятой будет еще одно слово
        }
        
        node->grammems[i] = malloc((count + 1) * sizeof(char*));// массив строк заканчивается значением NULL
        node->grammems_counts[i] = count;
        char *token = strtok(temp, ",");//strtok ищет первую запятую в строке temp. Находит её, заменяет на \0 и возвращает указатель на начало первого слова.
        int idx = 0;
        while (token) {
            while (*token == ' ') token++;
            node->grammems[i][idx] = strdup(token);
            idx++;
            token = strtok(NULL, ","); //NULL == "продолжи с того места, где остановился в прошлый раз"
        }
        node->grammems[i][idx] = NULL;
        free(temp);
    }
    node->left = NULL;
    node->right = NULL;
    return node;
}

TreeNode* build_balanced_tree(DictEntry *entries, int start, int end) { /* binary search tree */
    if (start > end) return NULL;
    
    int mid = (start + end) / 2;
    TreeNode *node = create_node(entries[mid].word, 
                                  entries[mid].grammems_list, 
                                  entries[mid].grammems_count);
   
    
    node->left = build_balanced_tree(entries, start, mid - 1);
    node->right = build_balanced_tree(entries, mid + 1, end);
    
    return node;
}



