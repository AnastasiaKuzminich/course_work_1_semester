#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "heading.h"

// Проверка, является ли символ буквой (русской)
int is_letter(const char *str, int *move) {
    unsigned char *ptr = (unsigned char*)str;
    unsigned char first = *ptr;
    
    if (first == 0xD0) {
        unsigned char second = *(ptr + 1);
        if ((second >= 0x90 && second <= 0xBF) || second == 0x81){ // А-Я + а-п + Ё
            *move = 2;
            return 1;
        }
    }
    else if (first == 0xD1) {
        unsigned char second = *(ptr + 1);
        if ((second >= 0x80 && second <= 0x8F) || second == 0x91) { // р-я + ё
            *move = 2;
            return 1;
        }
    }
    // Дефис (считаем частью слова)
    else if (first == '-') {
        *move = 1;
        return 2; // специальный код для дефиса
    }
    
    *move = 1;
    return 0;
}

// Преобразование слова в нижний регистр (для русских букв)
char* to_lowercase_utf8(const char *word) {
    if (!word) return NULL;
    
    int len = strlen(word);
    char *result = malloc(len + 1);
    if (!result) return NULL;
    int i = 0;
    int j = 0;
    while (i < len) {
        unsigned char c = word[i];
        if (c == 0xD0) {
            unsigned char next = word[i + 1];
            if (next >= 0x90 && next <= 0x9F) { // А-П (верхний регистр)
                result[j++] = 0xD0;
                result[j++] = next + 0x20; // преобразуем в нижний регистр
                i += 2;
                continue;
            }
            else if (next >= 0xA0 && next <= 0xAF) { // Р-Я (верхний регистр)
                result[j++] = 0xD1;
                result[j++] = next - 0x20; // преобразуем в нижний регистр (в другой диапазон)
                i += 2;
                continue;
            }
            else if (next == 0x81) {
                result[j++] = 0xD1;
                result[j++] = 0x91;
                i += 2;
                continue;
            }
            else {
                result[j++] = c;
                result[j++] = next;
                i += 2;
                continue;
            }
        }
        else {
            result[j++] = c;
            i++;
        }
    }
    result[j] = '\0';
    return result;
}

// Получение длины строки в символах (не байтах)
int utf8_strlen(const char *str) {
    if (!str) return 0; 
    int len = 0;
    int i = 0;
    while (str[i]) {
        if ((str[i] & 0x80) == 0) {
            i += 1; // ASCII символ (1 байт)
        }
        else if ((str[i] & 0xE0) == 0xC0) {
            i += 2; // 2-байтовый UTF-8
        }
        else if ((str[i] & 0xF0) == 0xE0) {
            i += 3; // 3-байтовый UTF-8
        }
        else if ((str[i] & 0xF8) == 0xF0) {
            i += 4; // 4-байтовый UTF-8
        }
        else {
            i++; // ошибка, но продолжаем
        }
        len++;
    }
    return len;
}

token* tokenize_text(const char *text, int *token_count) {
    // Временное накопление слова
    char word_buffer[1024];
    int buffer_pos = 0;
    // память под массив токенов (макс. 10 000) 
    token *tokens = malloc(10000 * sizeof(token));
    if (!tokens) {
        printf("ОШИБКА!! Не удалось выделить память для токенов\n");
        return NULL;
    }
    *token_count = 0;
    int i = 0;
    int text_len = strlen(text);
    while (i < text_len) {
        int move = 0;
        int is_letter_result = is_letter(text + i, &move);
        if (is_letter_result == 1 || is_letter_result == 2) { // буква или дефис
            // Копируем символ в буферы
            for (int j = 0; j < move; j++) {
                if (buffer_pos < 1023) {
                    word_buffer[buffer_pos] = text[i + j];
                    buffer_pos++;
                }
            }
            i += move;
        }
        else { // разделитель
            if (buffer_pos > 0) {
                // Завершение текущего слова
                word_buffer[buffer_pos] = '\0';
                // Создание токена
                token *t = &tokens[*token_count];
                t->original_word = strdup(word_buffer);
                t->lowercase_word = to_lowercase_utf8(word_buffer);
                t->length = utf8_strlen(word_buffer);
                t->found_in_dict = 0;
                t->node = NULL;
                
                (*token_count)++;
                // Сброс буферов
                buffer_pos = 0;
            }
        i += move; // пропускаем разделитель
        }
    }
    // Обрабатываем последнее слово, если текст не заканчивается разделителем
    if (buffer_pos > 0) {
        word_buffer[buffer_pos] = '\0';
        token *t = &tokens[*token_count];
        t->original_word = strdup(word_buffer);
        t->lowercase_word = to_lowercase_utf8(word_buffer);
        t->length = utf8_strlen(word_buffer);
        t->found_in_dict = 0;
        t->node = NULL;
        (*token_count)++;
    }
    return tokens;
}




