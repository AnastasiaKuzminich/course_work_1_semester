#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heading.h"

const char* get_gram_value(token* tok, const char* type) {
    if (!tok || !tok->node) return NULL;
    int target_pos_found = 0;
    
    TreeNode* node = tok->node;
    for (int om = 0; om < node->omonyms_count; om++) {

        for (int g = 0; g < node->grammems_counts[om]; g++) {
            char* gram = node->grammems[om][g];
            
            if (strcmp(type, "gender") == 0) {
                if (strcmp(gram, "masc") == 0 || strcmp(gram, "femn") == 0 || strcmp(gram, "neut") == 0)
                    return gram;
            }
            else if (strcmp(type, "number") == 0) {
                if (strcmp(gram, "sing") == 0 || strcmp(gram, "plur") == 0)
                    return gram;
            }
            else if (strcmp(type, "case") == 0) {
                if (strcmp(gram, "nomn") == 0 || strcmp(gram, "gent") == 0 || 
                    strcmp(gram, "datv") == 0 || strcmp(gram, "accs") == 0 ||
                    strcmp(gram, "ablt") == 0 || strcmp(gram, "loct") == 0)
                    return gram;
            }
        }
    }
    return NULL;
}

// Добавляет грамматическую ошибку в массив (оба слова через пробел)
void add_grammar_mistake(token* word1, token* word2,
                          mistake** arr_mistakes, int* mistakes_count, int* mistakes_capacity) {
    //  Проверка, нужно ли расширить массив (динамическая память)
    if (*mistakes_count >= *mistakes_capacity) {
        int new_capacity = (*mistakes_capacity == 0) ? 10 : (*mistakes_capacity * 2);
        mistake* temp = (mistake*)realloc(*arr_mistakes, new_capacity * sizeof(mistake));
        if (temp == NULL) {
            printf("ОШИБКА!! Не удалось выделить память под грамматическую ошибку\n");
            return;
        }
        *arr_mistakes = temp;
        *mistakes_capacity = new_capacity;
    }
    int idx = *mistakes_count;
    // Формируем строку из двух слов через пробел
    char combined[512];
    snprintf(combined, sizeof(combined), "%s %s", word1->original_word, word2->original_word);
    // Сохраняем в массив ошибок
    (*arr_mistakes)[idx].original_word = strdup(combined);
    (*arr_mistakes)[idx].lowercase_word = strdup(combined);
    (*arr_mistakes)[idx].type_mistake = 1; // 1 = грамматическая ошибка
    (*arr_mistakes)[idx].count_corrections = 0;
    (*arr_mistakes)[idx].arr_corrections = NULL;
    
    (*mistakes_count)++;
}

// Проверка согласования прилагательного с существительным
void check_adj_noun(token* adj, token* noun, 
                     mistake** arr_mistakes, int* mistakes_count, int* mistakes_capacity) {

    if (!adj->node || !noun->node) return;
    const char *adj_number = get_gram_value(adj, "number");
    const char *noun_number = get_gram_value(noun, "number");
    if (!adj_number || !noun_number) return;

    if (strcmp(adj_number, noun_number) != 0) {
        add_grammar_mistake(adj, noun, arr_mistakes, mistakes_count, mistakes_capacity);
        return; // число не совпало - дальше не проверяем
    }
    // Если число совпало и это множественное число - род не проверяем
    if (strcmp(adj_number, "plur") == 0) {
        // Только падеж
        const char *adj_case = get_gram_value(adj, "case");
        const char *noun_case = get_gram_value(noun, "case");
        if (adj_case && noun_case && strcmp(adj_case, noun_case) != 0) {
            add_grammar_mistake(adj, noun, arr_mistakes, mistakes_count, mistakes_capacity);
        }
        return;
    }
     // Если число совпало и это единственное число - проверяем род и падеж
    if (strcmp(adj_number, "sing") == 0) {
        // Проверка рода
        const char *adj_gender = get_gram_value(adj, "gender");
        const char *noun_gender = get_gram_value(noun, "gender");
        
        if (adj_gender && noun_gender && strcmp(adj_gender, noun_gender) != 0) {
            add_grammar_mistake(adj, noun, arr_mistakes, mistakes_count, mistakes_capacity);
            return; // род не совпал - дальше не проверяем
        }

        // Проверка падежа
        const char *adj_case = get_gram_value(adj, "case");
        const char *noun_case = get_gram_value(noun, "case");
        
        if (adj_case && noun_case && strcmp(adj_case, noun_case) != 0) {
            add_grammar_mistake(adj, noun, arr_mistakes, mistakes_count, mistakes_capacity);
        }
    }
}

// Проверка согласования глагола прошедшего времени с существительным
void check_verb_noun(token *verb, token *noun,
                      mistake **arr_mistakes, int *mistakes_count, int* mistakes_capacity) {
    // Существительное должно быть в именительном падеже
    const char* noun_case = get_gram_value(noun, "case");
    if (!noun_case || strcmp(noun_case, "nomn") != 0) return;
    
    const char *verb_number = get_gram_value(verb, "number");
    const char *noun_number = get_gram_value(noun, "number");
    if (!verb_number || !noun_number) return;
    
    if (strcmp(verb_number, noun_number) != 0) {
        add_grammar_mistake(verb, noun, arr_mistakes, mistakes_count, mistakes_capacity);
        return;
    }

    // Если множественное число - род не проверяем
    if (strcmp(verb_number, "plur") == 0) {
        return; // род не проверяется
    }

    // Если единственное число - проверяем род
    if (strcmp(verb_number, "sing") == 0) {
        const char *verb_gender = get_gram_value(verb, "gender");
        const char *noun_gender = get_gram_value(noun, "gender");
        
        if (verb_gender && noun_gender && strcmp(verb_gender, noun_gender) != 0) {
            add_grammar_mistake(verb, noun, arr_mistakes, mistakes_count, mistakes_capacity);
        }
    }
}


// Есть ли у токена нужная граммема
int has_gram(token* tok, const char* target) {
    if (!tok || !tok->node) return 0;
    
    TreeNode *node = tok->node;
    for (int om = 0; om < node->omonyms_count; om++) {
        for (int g = 0; g < node->grammems_counts[om]; g++) {
            if (strcmp(node->grammems[om][g], target) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

// Определяет часть речи токена и возвращает соответствующий код
int get_pos_type(token* tok) {
    if (!tok || !tok->found_in_dict || !tok->node) {
        return POS_OTHER;
    }
    // Существительное
    if (has_gram(tok, "NOUN")) {
        return POS_NOUN;
    }
    // Прилагательное (ADJF - полное, ADJS - краткое)
    if (has_gram(tok, "ADJF") || has_gram(tok, "ADJS")) {
        return POS_ADJ;
    }
    // Глагол прошедшего времени
    if (has_gram(tok, "VERB") && has_gram(tok, "past")) {
        return POS_PAST_VERB;
    }
    // Другой глагол
    if (has_gram(tok, "VERB")) {
        return POS_VERB;
    }
    return POS_OTHER;
}


int check_grammatics(token* arr_tokens, int token_count, 
                      mistake** arr_mistakes, int *mistakes_count, int* mistakes_capacity) {
    
    if (!arr_mistakes) return 1;
    
    // для каждого токена хранится код части речи
    int* pos_arr = (int*)calloc(token_count, sizeof(int));
    if (!pos_arr) return 1;

    for (int i = 0; i < token_count; i++) {
        pos_arr[i] = get_pos_type(&arr_tokens[i]);
    } // определили части речи всех слов

    int before_mistakes_count  = *mistakes_count;
    
    for (int i = 0; i < token_count; i++) {
        // если текущий токен не существительное - пропускаем
        if (pos_arr[i] != POS_NOUN) continue;
        
        // проверка ЛЕВОГО соседа
        if (i > 0 && pos_arr[i-1] == POS_ADJ) {
            check_adj_noun(&arr_tokens[i-1], &arr_tokens[i], 
                           arr_mistakes, mistakes_count, mistakes_capacity);
        }
        else if (i > 0 && pos_arr[i-1] == POS_PAST_VERB) {
            check_verb_noun(&arr_tokens[i-1], &arr_tokens[i],
                            arr_mistakes, mistakes_count, mistakes_capacity);
        }
        
        // проверка ПРАВОГО соседа
        if (i+1 < token_count && pos_arr[i+1] == POS_ADJ) {
            check_adj_noun(&arr_tokens[i+1], &arr_tokens[i],
                           arr_mistakes, mistakes_count, mistakes_capacity);
        }
        else if (i+1 < token_count && pos_arr[i+1] == POS_PAST_VERB) {
            check_verb_noun(&arr_tokens[i+1], &arr_tokens[i],
                            arr_mistakes, mistakes_count, mistakes_capacity);
        }
    }
    free(pos_arr);
    int errors_found = *mistakes_count - before_mistakes_count;
    return 0;
}
