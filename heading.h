#ifndef HEADING_H
#define HEADING_H

#define MAX_WORD 256
#define MAX_GRAMMEM 1024
#define MAX_LINE 65536
#define TRIGRAM_SIZE 39304

#define POS_UNKNOWN 0 // part of speech
#define POS_NOUN    1
#define POS_ADJ     2
#define POS_VERB    3
#define POS_PAST_VERB 4
#define POS_OTHER   5

#define MAX_COUNT_CORRECTIONS 7
#define MAX_DISTANCE 5

#define COUNT_TEMP_CORRECTIONS 1000

typedef struct TreeNode { // узел дерева
    char *word;             // словоформа
    char ***grammems;    // массив массивов граммем (для омонимов)  
    int *grammems_counts;      // к-во граммем в каждом варианте
    int omonyms_count;    // к-во омонимов 
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

typedef struct arr_trigram{
    TreeNode* tree_node;
    struct arr_trigram* next_node;
}arr_trigram;

typedef struct {
    char *word;
    char **grammems_list;          // массив строк граммем
    int grammems_count;       // количество вариантов граммем
} DictEntry;

typedef struct { 
    DictEntry *entries;
    int count;
    int capacity;
} DictArray;

// Структура токена
typedef struct token {
    char *original_word;     // слово в исходном регистре
    char *lowercase_word;    // слово в нижнем регистре
    int length;              // длина слова в символах
    int found_in_dict;       // 1 - есть в словаре, 0 - нет
    TreeNode *node;          // указатель на узел дерева (если найден)
} token;

typedef struct mistake{
    char *original_word;     
    char *lowercase_word; 
    int type_mistake; //0-орфография, 1-грамматика
    char** arr_corrections;
    int count_corrections;
}mistake;

typedef struct temp_variants_corr{
    TreeNode* tree_node;
    int distance;
}temp_variants_corr;

int init_dict_array(DictArray *arr);
void add_dict_entry(DictArray *arr, const char *word, const char *grammem);
int compare_dict_entries(const void *a, const void *b);
int sort_and_find_middle(DictEntry *arr, int left, int right);
void quick_sort(DictEntry *arr, int left, int right);
void sort_dict_array(DictArray *arr);
void free_dict_array(DictArray *arr);
TreeNode* create_node(const char *word, char **grammems_list, int grammems_count);
TreeNode* build_balanced_tree(DictEntry *entries, int start, int end);
void free_tree(TreeNode *root);
void to_lowercase(char *str);
int parse_xml_to_array(const char *filename, DictArray *arr);

int get_code_letter(unsigned char** pointer);
void add_tree_node_to_trigrams(TreeNode* root, int* codes, int count_codes, arr_trigram** array_of_trigrams);
void recursion_around_tree(TreeNode* root, arr_trigram** array_of_trigrams);
arr_trigram** create_arr_trigram(TreeNode* root);
void free_trigrams_array(arr_trigram** array_trigrams, int size);

int is_letter(const char *str, int *move);
int utf8_strlen(const char *str);
token* tokenize_text(const char *text, int *token_count);
void free_tokens(token *tokens, int token_count);

TreeNode* search_token_in_tree(TreeNode* root, const char* word);
int check_each_token(TreeNode* root, token* arr_tokens, int tokens_count, mistake** arr_mistakes, int* mistakes_count, int* mistakes_capacity);
void free_mistakes_array(mistake** arr, int mistakes_count);

const char* get_gram_value(token* tok, const char* type);
void add_grammar_mistake(token* word1, token* word2, mistake** arr_mistakes, int* mistakes_count, int* mistakes_capacity);
void check_adj_noun(token* adj, token* noun, mistake** arr_mistakes, int* mistakes_count, int* mistakes_capacity);
void check_verb_noun(token *verb, token *noun, mistake **arr_mistakes, int *mistakes_count, int* mistakes_capacity);
int has_gram(token* tok, const char* target);
int get_pos_type(token* tok);
int check_grammatics(token* arr_tokens, int token_count, mistake** arr_mistakes, int *mistakes_count, int* mistakes_capacity);

int get_string_length(const char* str);
int min_from_three(int a, int b, int c);
int* word_to_arr_codes(const char* w, int len);
int find_distance(const char* word_cor, const char* word_er);
int if_tree_node_in_temp_arr(TreeNode* tree, temp_variants_corr* all_temp_variants, int count_temp_variants);
void add_tree_nodes_to_temp_variants(int* codes, int count_codes, temp_variants_corr* all_temp_variants, int* count_temp_variants, arr_trigram** triagrams );
void fulfill_all_variants(mistake* node_mistake, char* word, arr_trigram** triagrams, temp_variants_corr* all_temp_variants);
void collect_corrections(mistake* node_mistake, TreeNode* root, arr_trigram** triagrams);
void find_orpho_corrections(mistake** arr_mistakes, int count_mistakes, TreeNode* root, arr_trigram** triagrams);

void print_report_errors(mistake** arr_mistakes, int count_mistakes, const char* output_txt);

#endif
