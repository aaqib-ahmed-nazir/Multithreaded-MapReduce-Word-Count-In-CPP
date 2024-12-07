#ifndef TASK1_MAIN_H
#define TASK1_MAIN_H

// Defining Constants
#define MAX_WORD_LENGTH 10000
#define MAX_INPUT_LENGTH 10000
#define MAX_WORDS 5000

// Defining Structs
struct KeyValue;
struct SharedData;
struct ThreadData;
struct ReducerData;

// Defining Functions
void shuffle_data(SharedData *data);
void *shuffle_thread_func(void *arg);
void sanitize_word(char *word);
void *map_function(void *arg);
void *reduce_function(void *arg);
void split_input_text(const char *input_text, char ***all_words, int &word_count);
void split_words_into_chunks(char **all_words, int word_count, char ***inputs, int num_mappers);
int run_main(const char *input_text);

#endif