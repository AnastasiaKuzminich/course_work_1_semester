#include <stdio.h>
#include <string.h>
#include "heading.h"



void print_report_errors(mistake** arr_mistakes, int count_mistakes,const char* output_txt){
    FILE* file = fopen(output_txt, "a");
    if (file == NULL){
    }
    else{
        fprintf(file, "\n========================================\n");
        fprintf(file, "РЕЗУЛЬТАТЫ ПРОВЕРКИ ПРАВОПИСАНИЯ ТЕКСТА!\n");
    }
    int count_orpho = 0;
    int count_gram = 0;
    for (int i=0; i<count_mistakes; i++){
        if ((*arr_mistakes)[i].type_mistake == 0){
            count_orpho++;
            printf("Некорректное слово %d:      %s\n", count_orpho, (*arr_mistakes)[i].original_word);
            printf("Варианты исправлений:       \n");
            
            if (file != NULL){
                fprintf(file, "Некорректное слово %d:      %s\n", count_orpho, (*arr_mistakes)[i].original_word);
                fprintf(file, "Варианты исправлений:       \n");
            }


            if ((*arr_mistakes)[i].count_corrections == 0){
                printf("не найдено\n");
                if (file != NULL){
                    fprintf(file, "не найдено\n");
                }
                
            } else{
                for (int j = 0; j<(*arr_mistakes)[i].count_corrections; j++){
                    printf("%d.   %s\n", j+1, (*arr_mistakes)[i].arr_corrections[j]);
                    if (file != NULL){
                        fprintf(file, "%d.   %s\n", j+1, (*arr_mistakes)[i].arr_corrections[j]);
                    }
                }
                printf("\n");
                if (file != NULL){
                    fprintf(file, "\n");
                }

            }

        } else{
            count_gram ++;
            printf("Грамматическая ошибка %d: %s\n", count_gram, (*arr_mistakes)[i].original_word);
            if (file != NULL){
                fprintf(file, "Грамматическая ошибка %d: %s\n", count_gram, (*arr_mistakes)[i].original_word);

            }
        }

        printf("\n");
        if (file != NULL){
            fprintf(file, "\n");
        }
         
    }
    
    printf("ИТОГО:\nОрфографические ошибки: %d\nГрамматические ошибки: %d\n", count_orpho, count_gram);
    printf("==================================================\n\n\n");
    if (file != NULL){
            fprintf(file, "ИТОГО:\nОрфографические ошибки: %d\nГрамматические ошибки: %d\n", count_orpho, count_gram);
            fprintf(file, "==================================================\n\n\n");
            fclose(file);
        }
}
