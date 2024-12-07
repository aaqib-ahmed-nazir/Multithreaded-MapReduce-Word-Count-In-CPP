#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <cstring>
#include <unistd.h>
#include <iomanip>
#include "functions.h"
#include <algorithm> 

using namespace std;

// Struct Definitions
struct KeyValue
{
    // - Structure to store the key and its counts. 
    char key[50];
    int counts[MAX_WORDS];
    int count_size;
};

struct SharedData
{
    // - Structure to store the shared data between threads.
    char **input_chunks;
    KeyValue *intermediate_data;
    KeyValue *final_output;
    int intermediate_size;
    int output_size;
    int num_mappers;
    pthread_mutex_t mutex;
    sem_t shuffle_done;
    sem_t map_done_sem;
};

struct ThreadData
{
    // - Structure to store the thread data.
    SharedData *data;
    int thread_id;
};

struct ReducerData
{
    // - Structure to store the reducer data.
    SharedData *data;
    int thread_id;
    char key[50];
};

// Function Declarations
void shuffle_data(SharedData *data);
void *shuffle_thread_func(void *arg);
void sanitize_word(char *word);
void *map_function(void *arg);
void *reduce_function(void *arg);
void split_input_text(const char *input_text, char ***all_words, int &word_count);
void split_words_into_chunks(char **all_words, int word_count, char ***inputs, int num_mappers);

// Function Definitions

void *shuffle_thread_func(void *arg)
{
    /*
        - Parameters:
            - arg: void pointer to the shared data.

        - Returns:
            - NULL.

        - Description:
            - This function is used to wait for all the mappers to complete their
              work and then shuffle the data. 
    */

    SharedData *data = (SharedData *)arg;

    for (int i = 0; i < data->num_mappers; ++i)
    {
        sem_wait(&data->map_done_sem);
    }

    shuffle_data(data);
    
    return NULL;
}

void shuffle_data(SharedData *data)
{
    /*
        - Parameters:
            - data: pointer to the shared data.

        - Returns:
            - NULL.

        - Description:
            - This function is used to shuffle the data. 
    */

    cout << "\nAfter Shuffle:\n";
    cout << "----------------------------\n";
    cout << left << setw(20) << "Key" << setw(20) << "Counts" << endl;

    // Cirtical section: Starts 
    pthread_mutex_lock(&data->mutex);

    // Sort the intermediate data.
    for (int i = 0; i < data->intermediate_size - 1; i++)
    {
        for (int j = 0; j < data->intermediate_size - i - 1; j++)
        {
            if (strcasecmp(data->intermediate_data[j].key,
                           data->intermediate_data[j + 1].key) > 0)
            {
                KeyValue temp = data->intermediate_data[j];
                data->intermediate_data[j] = data->intermediate_data[j + 1];
                data->intermediate_data[j + 1] = temp;
            }
        }
    }

    // Printing
    for (int i = 0; i < data->intermediate_size; i++)
    {
        cout << left << setw(20) << data->intermediate_data[i].key
             << "[";
        for (int j = 0; j < data->intermediate_data[i].count_size; j++)
        {
            cout << data->intermediate_data[i].counts[j];
            if (j < data->intermediate_data[i].count_size - 1)
                cout << ", ";
        }
        cout << "]" << endl;
    }

    // Critical section: Ends
    pthread_mutex_unlock(&data->mutex);

    sem_post(&data->shuffle_done);
}

void sanitize_word(char *word)
{
    /*
        - Parameters:
            - word: pointer to the word.

        - Returns:
            - NULL.

        - Description:
            - This function is used to sanitize the word.
            - Convert the word to lowercase.
            - Remove all non-alphanumeric characters except '-' and '_'.
    */

    if (!word)
        return;

    char *write = word;
    char *read = word;

    while (*read)
    {
        if (isalnum(*read) || *read == '-' || *read == '_')
        {
            *write = tolower(*read);
            write++;
        }
        read++;
    }
    
    *write = '\0';
}

void *map_function(void *arg)
{
    /*
        - Parameters:
            - arg: void pointer to the thread data.

        - Returns:
            - NULL.

        - Description:
            - This function is used to map the words to their counts. 
    */

    ThreadData *tdata = (ThreadData *)arg;
    SharedData *data = tdata->data;
    char *input = data->input_chunks[tdata->thread_id];

    // Tokenize the input.
    char *input_copy = strdup(input);
    char *saveptr;
    char *token = strtok_r(input_copy, " \t\n\r\f\v", &saveptr);

    while (token != NULL)
    {
        char word[MAX_WORD_LENGTH];
        strncpy(word, token, MAX_WORD_LENGTH - 1);
        word[MAX_WORD_LENGTH - 1] = '\0';
        sanitize_word(word);

        if (strlen(word) > 0)
        {
            pthread_mutex_lock(&data->mutex);
            bool found = false;

            // If the word is found, increment the count.
            for (int i = 0; i < data->intermediate_size; i++)
            {
                if (strcasecmp(data->intermediate_data[i].key, word) == 0)
                {
                    data->intermediate_data[i].counts[data->intermediate_data[i].count_size++] = 1;
                    found = true;
                    
                    break;
                }
            }

            // If the word is not found, add it to the intermediate data.
            if (!found && data->intermediate_size < MAX_WORDS)
            {
                strncpy(data->intermediate_data[data->intermediate_size].key, word, 49);
                data->intermediate_data[data->intermediate_size].key[49] = '\0';
                data->intermediate_data[data->intermediate_size].counts[0] = 1;
                data->intermediate_data[data->intermediate_size].count_size = 1;
                data->intermediate_size++;
                cout << "Mapper " << tdata->thread_id << " added key: " << word << endl;
            }
            pthread_mutex_unlock(&data->mutex);
        }

        token = strtok_r(NULL, " \t\n\r\f\v", &saveptr);
    }

    free(input_copy);
    sem_post(&data->map_done_sem);

    cout << "Mapper " << tdata->thread_id << " completed." << endl;
    
    return NULL;
}

void *reduce_function(void *arg)
{
    /*
        - Parameters:
            - arg: void pointer to the reducer data.

        - Returns:
            - NULL.

        - Description:
            - This function is used to reduce the data.
    */

    ReducerData *rdata = (ReducerData *)arg;
    SharedData *data = rdata->data;
    int total_count = 0;

    // Counting the total count of the key.
    for (int i = 0; i < data->intermediate_size; i++)
    {
        if (strcasecmp(data->intermediate_data[i].key, rdata->key) == 0)
        {
            total_count += data->intermediate_data[i].count_size;
        }
    }

    strcpy(data->final_output[rdata->thread_id].key, rdata->key);
    data->final_output[rdata->thread_id].count_size = total_count;

    return NULL;
}

void split_input_text(const char *input_text, char ***all_words, int &word_count)
{
    /*
        - Parameters:
            - input_text: pointer to the input text.
            - all_words: pointer to the array of words.
            - word_count: reference to the word count.

        - Returns:
            - NULL.

        - Description:
            - This function is used to split the input text into words.
    */

    *all_words = new char *[MAX_WORDS];
    word_count = 0;

    char *temp = strdup(input_text);
    char *word = strtok(temp, " \t\n\r\f\v");

    while (word != NULL && word_count < MAX_WORDS)
    {
        (*all_words)[word_count] = strdup(word);
        word_count++;
        word = strtok(NULL, " \t\n\r\f\v");
    }

    free(temp);
}

void split_words_into_chunks(char **all_words, int word_count, char ***inputs, int num_mappers)
{
    /*
        - Parameters:
            - all_words: array of words.
            - word_count: total number of words.
            - inputs: pointer to the array of input chunks.
            - num_mappers: number of mappers.

        - Returns:
            - NULL.

        - Description:
            - This function is used to split the words into chunks for each mapper. 
    */

    *inputs = new char *[num_mappers];
    int words_per_mapper = word_count / num_mappers;
    int extra_words = word_count % num_mappers;

    int current_word = 0;
    for (int i = 0; i < num_mappers; i++)
    {
        char chunk[MAX_INPUT_LENGTH] = "";
        int words_for_this_mapper = words_per_mapper + (i < extra_words ? 1 : 0);

        for (int j = 0; j < words_for_this_mapper && current_word < word_count; j++)
        {
            strcat(chunk, all_words[current_word]);
            if (j < words_for_this_mapper - 1 && current_word < word_count - 1)
            {
                strcat(chunk, " ");
            }
            current_word++;
        }
        (*inputs)[i] = strdup(chunk);
    }
}

int run_main(const char *input_text) 
{   
    /*
        - Parameters:
            - input_text: the input text to be used for the test

        - Returns:
            - 0.

        - Description:
            - This function is the main function that runs the MapReduce algorithm.
            - It splits the input text into words and then splits the words into chunks for each mapper.
            - It creates threads for each mapper and reducer and waits for them to complete their work.
            - It then prints the final output.
    */
    SharedData data;

    data.intermediate_data = new KeyValue[MAX_WORDS];
    data.final_output = new KeyValue[MAX_WORDS];
    data.intermediate_size = 0;
    data.output_size = 0;

    pthread_mutex_init(&data.mutex, NULL);
    sem_init(&data.shuffle_done, 0, 0);
    sem_init(&data.map_done_sem, 0, 0);

    if (strlen(input_text) == 0)
    {
        cout << "\nFinal Output:\n";
        cout << "----------------------------\n";
        cout << left << setw(20) << "Key" << setw(10) << "Count" << endl;
        
        return 0;
    }

    // Split the input text into words.
    char **all_words;
    int word_count;
    split_input_text(input_text, &all_words, word_count);

    /* 
        - Dynamic calculation of NUM_MAPPERS based on word_count
        - // 3 mappers for <= 1000 words, 5 mappers for 1001 words, 6 mappers for 2001 words, etc
    */
    int NUM_MAPPERS = max(3, word_count / 1000 + 1); 
    data.num_mappers = NUM_MAPPERS;

    // Split the words into chunks for each mapper.
    char **inputs;
    split_words_into_chunks(all_words, word_count, &inputs, NUM_MAPPERS);

    // Freeing the memory.
    for (int i = 0; i < word_count; i++)
    {
        free(all_words[i]);
    }
    delete[] all_words;

    data.input_chunks = inputs;

    pthread_t *mappers = new pthread_t[data.num_mappers];
    ThreadData *thread_data = new ThreadData[data.num_mappers];

    // Starting the Map phase.
    cout << "\nStarting Map phase with " << NUM_MAPPERS << " mappers..." << endl;
    for (int i = 0; i < data.num_mappers; i++)
    {
        thread_data[i].data = &data;
        thread_data[i].thread_id = i;
        if (pthread_create(&mappers[i], NULL, map_function, &thread_data[i]) != 0)
        {
            cout << "Failed to create Mapper thread " << i << endl;
            exit(1);
        }
    }

    for (int i = 0; i < data.num_mappers; i++)
    {
        pthread_join(mappers[i], NULL);
    }

    pthread_t shuffle_thread;
    if (pthread_create(&shuffle_thread, NULL, shuffle_thread_func, &data) != 0)
    {
        cout << "Failed to create Shuffle thread." << endl;
        exit(1);
    }

    pthread_t *reducers = new pthread_t[data.intermediate_size];
    ReducerData *reducer_data = new ReducerData[data.intermediate_size];

    // Starting the Reduce phase.
    cout << "\nStarting Reduce phase..." << endl;
    int reducer_thread_idx = 0;
    for (int i = 0; i < data.intermediate_size; i++)
    {
        strcpy(reducer_data[reducer_thread_idx].key, data.intermediate_data[i].key);
        reducer_data[reducer_thread_idx].data = &data;
        reducer_data[reducer_thread_idx].thread_id = reducer_thread_idx;

        if (pthread_create(&reducers[reducer_thread_idx], NULL, reduce_function, &reducer_data[reducer_thread_idx]) != 0)
        {
            cout << "Failed to create Reducer thread " << reducer_thread_idx << endl;
            exit(1);
        }
        reducer_thread_idx++;
    }

    // Wait for all the reducers to complete their work.
    for (int i = 0; i < data.intermediate_size; i++)
    {
        pthread_join(reducers[i], NULL);
    }

    cout << "\nFinal Output:\n";
    cout << "----------------------------\n";
    cout << left << setw(20) << "Key" << setw(10) << "Count" << endl;
    for (int i = 0; i < data.intermediate_size; i++)
    {
        cout << left << setw(20) << data.final_output[i].key
             << setw(10) << data.final_output[i].count_size << endl;
    }

    // Freeing the memory.
    pthread_mutex_destroy(&data.mutex);
    sem_destroy(&data.shuffle_done);
    sem_destroy(&data.map_done_sem);
    delete[] data.intermediate_data;
    delete[] data.final_output;
    delete[] inputs;
    delete[] mappers;
    delete[] reducers;
    delete[] thread_data;
    delete[] reducer_data;

    return 0;
}