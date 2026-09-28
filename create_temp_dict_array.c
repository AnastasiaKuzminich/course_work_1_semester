#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "heading.h"

int init_dict_array(DictArray *arr) {  
    arr->capacity = 100000;
    arr->count = 0;
    arr->entries = malloc(arr->capacity * sizeof(DictEntry));
    if (arr->entries == NULL)
        return 1;
    return 0;
}

// добавление в массив слова как нового в конец массива, либо как омонин к уже существующему слову
void add_dict_entry(DictArray *arr, const char *word, const char *grammem) {  
    for (int i = 0; i < arr->count; i++) {
        if (strcmp(arr->entries[i].word, word) == 0) {
            // Омоним: добавляем новый вариант граммем
            arr->entries[i].grammems_count++;
            arr->entries[i].grammems_list = realloc(arr->entries[i].grammems_list, arr->entries[i].grammems_count * sizeof(char*));
            arr->entries[i].grammems_list[arr->entries[i].grammems_count - 1] = strdup(grammem);
            return;
        }
    }
    if (arr->count >= arr->capacity) { // новое слово
        arr->capacity *= 2;
        arr->entries = realloc(arr->entries, arr->capacity * sizeof(DictEntry));
    }
    arr->entries[arr->count].word = strdup(word);
    arr->entries[arr->count].grammems_list = malloc(sizeof(char*));
    arr->entries[arr->count].grammems_list[0] = strdup(grammem);
    arr->entries[arr->count].grammems_count = 1;
    arr->count++;
}

int compare_dict_entries(const void *a, const void *b) {  
    DictEntry *da = (DictEntry*)a; // преобразование к нужному типу для сравнения
    DictEntry *db = (DictEntry*)b;
    return strcmp(da->word, db->word); 
}

int sort_and_find_middle(DictEntry *arr, int left, int right){  
    int middle = left + (right-left)/2;
    DictEntry mid_elem = arr[middle];
    int L = left; 
    int R = right;
    while (1){
        while (strcmp(arr[L].word, mid_elem.word) < 0){ // поиск того элемента слева (к середине двигаясь), который больше центрального
            L++;
        }
        while (strcmp(arr[R].word, mid_elem.word) > 0){ // поиск того элемента справа (к середине двигаясь), который меньше центрального
            R--;
        }
        if (L>=R){
            return R;
        }
        DictEntry temp = arr[L];
        arr[L] = arr[R];
        arr[R] = temp;
        L++;
        R--;
    }
}

void quick_sort(DictEntry *arr, int left, int right){   
    if (left < right){
        int middle = sort_and_find_middle(arr, left, right);
        quick_sort(arr, left, middle);
        quick_sort(arr, middle+1, right);
    }
}

void sort_dict_array(DictArray *arr) {     
    if (arr->count > 1) quick_sort(arr->entries, 0, arr->count-1); 
}

void to_lowercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

int parse_xml_to_array(const char *filename, DictArray *arr) {
    FILE *file = fopen(filename, "rb");
    if (!file) return 1;

    char common_grammems[MAX_GRAMMEM] = "";
    char current_f_word[MAX_WORD] = "";
    char current_f_grammems[MAX_GRAMMEM] = "";
    int in_lemma = 0;
    int in_f_tag = 0;

    int c;
    char tag[4096];
    int tag_pos = 0;
    int in_tag = 0;

    while ((c = fgetc(file)) != EOF) {
        if (c == '<') {
            in_tag = 1;
            tag_pos = 0;
        } else if (c == '>') {
            if (in_tag) {
                tag[tag_pos] = '\0';
                in_tag = 0;

                // 1. Начало леммы — сбрасываем общие граммемы
                if (strncmp(tag, "lemma", 5) == 0) {
                    in_lemma = 1;
                    common_grammems[0] = '\0';
                }
                // 2. Тег <l> — это общие признаки для всех форм (часть речи, род леммы)
                else if (in_lemma && tag[0] == 'l' && tag[1] != '/') {
                    char *g_ptr = tag;
                    char *g_start;
                    while ((g_start = strstr(g_ptr, "g v=\"")) != NULL) {
                        g_start += 5;
                        char *g_end = strchr(g_start, '"');
                        if (g_end) {
                            if (common_grammems[0] != '\0') strcat(common_grammems, ",");
                            strncat(common_grammems, g_start, g_end - g_start);
                            g_ptr = g_end;
                        } else break;
                    }
                }
                // 3. Начало формы <f t="...">
                else if (in_lemma && strncmp(tag, "f t=\"", 5) == 0) {
                    in_f_tag = 1;
                    current_f_grammems[0] = '\0';
                    
                    char *word_start = tag + 5;
                    char *word_end = strchr(word_start, '"');
                    if (word_end) {
                        int len = word_end - word_start;
                        strncpy(current_f_word, word_start, len);
                        current_f_word[len] = '\0';
                        to_lowercase(current_f_word);
                    }
                }
                // 4. Теги <g v="..."> внутри <f> или просто в лемме
                else if (in_lemma && strncmp(tag, "g v=\"", 5) == 0) {
                    char *v_start = tag + 5;
                    char *v_end = strchr(v_start, '"');
                    if (v_end) {
                        if (in_f_tag) {
                            if (current_f_grammems[0] != '\0') strcat(current_f_grammems, ",");
                            strncat(current_f_grammems, v_start, v_end - v_start);
                        } else {
                            // Если мы еще не в <f>, значит это граммемы леммы (как в <l>)
                            if (common_grammems[0] != '\0') strcat(common_grammems, ",");
                            strncat(common_grammems, v_start, v_end - v_start);
                        }
                    }
                }
                // 5. Конец формы </f> — вот теперь сохраняем всё в массив!
                else if (strncmp(tag, "/f", 2) == 0 && in_f_tag) {
                    char full_grammems[MAX_GRAMMEM * 2] = "";
                    if (common_grammems[0]) strcpy(full_grammems, common_grammems);
                    if (current_f_grammems[0]) {
                        if (full_grammems[0]) strcat(full_grammems, ",");
                        strcat(full_grammems, current_f_grammems);
                    }
                    
                    if (full_grammems[0] == '\0') strcpy(full_grammems, "UNKNOWN");
                    
                    add_dict_entry(arr, current_f_word, full_grammems);
                    in_f_tag = 0; // Закрыли форму
                }
                else if (strncmp(tag, "/lemma", 6) == 0) {
                    in_lemma = 0;
                }
            }
        } else if (in_tag && tag_pos < 4095) {
            tag[tag_pos++] = c;
        }
    }
    fclose(file);
    return 0;
}
