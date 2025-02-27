#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define WORDCHAR_MAX 100
#define BUFFER_SIZE 1024
#define MAX_TOKENS 300000
#define TABLE_SIZE 115200


typedef struct WordData {
    char *word;
    int count;
    struct WordData *next;
} WordData;


void tokenizer(const char* path, char **tokens) {
    FILE *file = fopen(path, "r");
    if (!file) {
        perror("Error opening file");
        return;
    }   
    
    char buffer[BUFFER_SIZE];
    char word[WORDCHAR_MAX];
    int token_count = 0, word_idx = 0;
    
    while (fgets(buffer, sizeof(buffer), file) && token_count < MAX_TOKENS) {
        for (int i = 0; buffer[i]; i++) {
            if (isalnum((unsigned char)buffer[i])) {
                // add the character to the word
                if (word_idx >= WORDCHAR_MAX - 1) // skip if the word exceeds the limit. Shoudln't happen.
                {
                    word_idx = 0; // if happens, move to the next buffer
                    break;
                }
                word[word_idx++] = tolower((unsigned char)buffer[i]);                    
            }
            else{ // the word should ends for all non-alphanumeric characters
                if (word_idx == 0) continue; // skip if the word is empty
                word[word_idx] = '\0';
                tokens[token_count] = malloc(word_idx + 1);
                memcpy(tokens[token_count], word, word_idx + 1);                
                token_count++;
                word_idx = 0;
            }
        }
    }    
    fclose(file);
}



void upsert(WordData **linked_list, const char *word) {
    WordData *current = *linked_list;
    
    while (current) {
        if (strcmp(current->word, word) == 0) {
            current->count++;
            return;
        }
        current = current->next;
    }

    // No match
    WordData *new_word = malloc(sizeof(WordData));
    new_word->word = strdup(word);
    new_word->count = 1;
    new_word->next = *linked_list; // attach front
    *linked_list = new_word;
}

char **find_frequent_words(const char *path, int32_t n) {

    // 1. Tokenize the file
    char **tokens = malloc((MAX_TOKENS + 1) * sizeof(char*));
    tokenizer("shakespeare.txt", tokens);

    // 2. Count unique words
    WordData **linked_list = malloc(TABLE_SIZE * sizeof(WordData*));
    for (int i=0; i<TABLE_SIZE; i++) linked_list[i] = NULL;
    for (int i = 0; tokens[i]; i++) {
        upsert(linked_list, tokens[i]);
    }

    // print linked list
    for (int i = 0; i < TABLE_SIZE; i++) {
        WordData *current = linked_list[i];
        while (current) {
            printf("%s: %d\n", current->word, current->count);
            current = current->next;
        }
    }

    // 3. get counts by the index
    int count_list[TABLE_SIZE] = {0};
    int idx = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        WordData *current = linked_list[i];
        while (current && idx < TABLE_SIZE) {
            count_list[idx] = current->count;
            idx++;
            current = current->next;
        }
    }

    // print count list
    // for (int i = 0; i < TABLE_SIZE; i++) {
    //     printf("%d: %d\n", i, count_list[i]);
    // }
    

    // 4. sort the index by count
    printf("Sorting...\n");
    for (int i = 0; i < TABLE_SIZE - 1; i++) {
        for (int j = 0; j < TABLE_SIZE - i - 1; j++) {
            if (count_list[j] < count_list[j + 1]) {
                // Swap counts
                int temp_count = count_list[j];
                count_list[j] = count_list[j + 1];
                count_list[j + 1] = temp_count;               
            }
        }
    }
    printf("Sorted\n");


    // 5. get the top n words
    char **result = malloc((n + 1) * sizeof(char*));
    int words_found = 0;

    // Iterate through the linked list to find words matching top counts
    for (int i = 0; i < n && words_found < n; i++) {
        int target_count = count_list[i];
        for (int j = 0; j < TABLE_SIZE && words_found < n; j++) {
            WordData *current = linked_list[j];
            while (current && words_found < n) {
                if (current->count == target_count) {
                    result[words_found] = strdup(current->word);
                    words_found++;
                }
                current = current->next;
            }
        }
    }
    result[words_found] = NULL;

   
    
    return result;
    



}

int main() {
    int n = 10; // Number of top words to print
    char **words = find_frequent_words("shakespeare.txt", n);

    if (!words) {
        printf("Error finding top words\n");
        return;
    }

    // Print the top n words
    printf("Top %d most frequent words:\n", n);
    for (int i = 0; words[i] && i < n; i++) {
        printf("%d. %s\n", i + 1, words[i]);
    }

    // Clean up the result array
    for (int i = 0; words[i]; i++) {
        free(words[i]);
    }
    free(words);

    return 0;
}