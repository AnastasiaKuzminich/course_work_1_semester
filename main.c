#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>
#include "heading.h"

int main(void) {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    
    char* input_file = "dictionary.xml";
    char* output_file = "RESULT.txt"; //name_output_file
    
    DictArray dict_array;
    if (init_dict_array(&dict_array) == 1){
        printf("ОШИБКА!! Не удалось выделить память для построения временного массива для дерева...");
        return 1;
    }

    if (parse_xml_to_array(input_file, &dict_array) == 1){
        printf("ОШИБКА!!Не удалось открыть xml файл для построения массива и дерева.");
        free_dict_array(&dict_array);
        return 1;
    }
    
    if (dict_array.count == 0) {
        printf("ОШИБКА!!Нет словоформ для построения дерева!\n");
        printf("кодировка XML файла!!!!!\n");
        free_dict_array(&dict_array);
        return 1;
    }
    printf("\nБинарное дерево будет содержать %d словоформ.\n", dict_array.count);
    
    sort_dict_array(&dict_array);
    
    TreeNode* root = build_balanced_tree(dict_array.entries, 0, dict_array.count - 1);
    free_dict_array(&dict_array);
   
    arr_trigram** array_trigrams = create_arr_trigram(root);
    if (array_trigrams == NULL) {
        printf("ОШИБКА!! Массив триграмм не создан\n");
        if (root) free_tree(root);
        return 1;
    } 
    
    FILE* output_txt = fopen(output_file, "w");
    if (output_txt == NULL){
        printf("ОШИБКА!! Выходной файл %s не создан.", output_file);
    }
    else
        fclose(output_txt);

    char *input_text;
    token* tokens;
    mistake* arr_mistakes;
    int token_count;
    int mistakes_count;
    int mistakes_capacity;
    
    while (1){
        input_text = NULL;
        tokens = NULL;
        arr_mistakes = NULL;
        token_count = 0;
        mistakes_count = 0;
        mistakes_capacity = 0;

        printf("================================\n");
        printf("ВЫБОР РЕЖИМА ВВОДА ТЕКСТА\n");
        printf("1 - Ввод текста из консоли...\n");
        printf("2 - Ввод текста из файла...\n");
        printf("0 - Завершить...\n");

        int choice = 0;
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); 
            continue;
        }
        getchar(); 


        if (choice == 0){
            printf("Программа завершена!\n");
            break; 
        }
        if (choice == 1) {
            printf("\nВведите текст (и нажмите Enter):\n");
            char buffer[100000];
            if (fgets(buffer, sizeof(buffer), stdin)) {
                int len = strlen(buffer);
                if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0';
                input_text = strdup(buffer);
            }

        }
        else if (choice == 2) {
            char filename[1024];
            FILE *file = NULL;
            
            while (file == NULL) {
                printf("\nВведите относительный путь к файлу: ");
                fgets(filename, sizeof(filename), stdin);
                int len = strlen(filename);
                if (len > 0 && filename[len-1] == '\n') {
                    filename[len-1] = '\0';
                }
                file = fopen(filename, "rb");
                if (!file) {
                    printf("ОШИБКА!! Файл '%s' не найден! Попробуйте снова.\n", filename);
                }
            }
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file); /// текущая позиция указателя в файле (в байтах от начала)
            fseek(file, 0, SEEK_SET);
            
            input_text = malloc(file_size + 1);
            fread(input_text, 1, file_size, file);  // размер элемента	(чтение по 1 байту)
            input_text[file_size] = '\0';
            
            fclose(file);
            printf("Файл успешно прочитан!\n");
        }
        else {
            printf("Неверный выбор!\n");
        }

        if (input_text != NULL) {
            FILE* output_txt = fopen(output_file, "a");
            
            if (output_txt != NULL){
                fprintf(output_txt, "\n=======================================\n");
                fprintf(output_txt, "%s", input_text);
                fclose(output_txt);
            }
        }
        
        tokens = tokenize_text(input_text, &token_count);
        
        if (tokens != NULL) {
                if (check_each_token(root, tokens, token_count, &arr_mistakes, &mistakes_count, &mistakes_capacity) == 0) {
                    if (check_grammatics(tokens, token_count, &arr_mistakes, &mistakes_count, &mistakes_capacity) != 0){
                        printf("ОШИБКА!! Произошла ошибка в выделении памяти в процессе проверки грамматики");
                    }
                    
                    if (mistakes_count > 0) {
                        find_orpho_corrections(&arr_mistakes, mistakes_count, root, array_trigrams);
                        print_report_errors(&arr_mistakes, mistakes_count, output_file);
                    } else {
                        printf("\nОшибок не обнаружено.\n");
                        printf("\n=======================================\n");
                        
                        FILE* output_txt = fopen(output_file, "a");
                        if (output_txt != NULL){
                            fprintf(output_txt, "\nОшибок не обнаружено.\n");
                            fprintf(output_txt, "\n=======================================\n");
                            fclose(output_txt);
                        }

                    }
                }
                
                if (arr_mistakes) free_mistakes_array(&arr_mistakes, mistakes_count);
                free_tokens(tokens, token_count);
            }
            free(input_text);
    }
    if (array_trigrams) free_trigrams_array(array_trigrams, TRIGRAM_SIZE);
    if (root) free_tree(root);

    return 0;
}

/* ./program.exe -f dict.xml -o result.xml
c
argc = 5
argv[0] = "./program.exe"
argv[1] = "-f"
argv[2] = "dict.xml"
argv[3] = "-o"
argv[4] = "result.xml"
argv[5] = NULL */


/* strlen	Возвращает длину строки (без \0)	strlen("NOUN") → 4
strcpy	Копирует строку	strcpy(dest, src)
strncpy	Копирует не более n символов	strncpy(dest, src, 5)
strcat	Добавляет строку в конец	strcat(dest, src)
strncat	Добавляет не более n символов	strncat(dest, src, 3)
strcmp	Сравнивает строки (регистр важен)	strcmp("NOUN", "NOUN") → 0
strstr	Ищет подстроку в строке	strstr(line, "<lemma")
strchr	Ищет символ в строке	strchr(ptr, '"')
strdup	Создаёт копию строки (в куче)	strdup("коллега")
strtok	Разбивает строку на токены	strtok(str, ",") */



/* int snprintf(char *buffer, size_t size, const char *format, ...);
buffer — куда записываем
size — максимальное количество байт для записи (включая \0)
format — строка форматирования (как в printf)
... — аргументы для форматирования */