//
// Created by Admin on 08/12/2022.
//

#include <string.h>
#include "markov_chain.h"

#define MAX_ARG 5
#define MIN_ARG 4
#define MAX_LINE 1001
#define ERROR "Error: "
#define USAGE "Usage: "
#define ERROR_MSG "Invalid number of arguments.\n"
#define ALLOCATION_FAILURE "Allocation failure: \n"
#define FILE_PATH_NOT_FOUND "File was not found. \n"

static void *copy_func(void *data) {
    char *str = (char *) data;
    char *new_str = malloc(strlen(str) + 1);
    if (new_str == NULL) {
        return NULL;
    }
    strcpy(new_str, str);
    return new_str;
}

static int comp_func(void *data1, void *data2) {
    char *str1 = (char *) data1;
    char *str2 = (char *) data2;
    return (strcmp(str1, str2));

}

static bool is_last(void *data) {
    char *str = (char *) data;
    if (str[strlen(str) - 1] == '.') {
        return true;
    }
    return false;

}


static void print_func(void *data) {
    char *str = (char *) data;
    if (is_last(data)) {
        printf("%s", str);
    } else {
        printf("%s ", str);
    }

}


/**
 * function that fills the markov chain with data from the giving file
 * @param fp - pointer to the file that contains the data
 * @param words_to_read - number of words to read
 * @param markov_chain - the chain to fill
 * @return 0 if succeeded else 1
 */
static int
fill_database(FILE *fp, int words_to_read, MarkovChain *markov_chain) {
    if (!fp || !markov_chain) {
        return EXIT_FAILURE;
    }
    bool continue_to_read = true;
    if (words_to_read == 0) {
        continue_to_read = false;
    }
    char line[MAX_LINE], *tofill = NULL, *input = NULL;
    Node *new_node = NULL, *new_next_node = NULL;
    while (fgets(line, MAX_LINE, fp) && continue_to_read) {
        input = strtok(line, "\n");
        tofill = strtok(input, " ");
        words_to_read = (words_to_read == -1) ? words_to_read : words_to_read -
                                                                1;
        if (tofill == NULL) {
            continue;
        }
        new_node = add_to_database(markov_chain, tofill);
        if (new_node == NULL) {
            return EXIT_FAILURE;
        }
        tofill = strtok(NULL, " ");
        words_to_read =
                (words_to_read == -1) ? words_to_read : words_to_read - 1;
        while (tofill != NULL && continue_to_read) {
            new_next_node = add_to_database(markov_chain, tofill);
            if (new_next_node == NULL ||
                add_node_to_counter_list(new_node->data,
                                         new_next_node->data, markov_chain) ==
                false) {
                return EXIT_FAILURE;
            }
            tofill = strtok(NULL, " ");
            new_node = new_next_node;
            if (words_to_read != -1) {
                words_to_read -= 1;
                continue_to_read = (words_to_read > 0) ? true : false;
                if (continue_to_read == false) {
                    new_next_node = add_to_database(markov_chain, tofill);
                    if (add_node_to_counter_list(new_node->data,
                                                 new_next_node->data,
                                                 markov_chain) ==
                        false) {
                        return EXIT_FAILURE;
                    }
                }
            }
        }
    }
    return EXIT_SUCCESS;
}


/**
 *function that creates a new markov chain and fills it with data
 * from the given file
 * @param fp - pointer to the file
 * @param words_to_read - number of words to read
 * @return the updated markov chain if succeeded else null
 */
static MarkovChain *fill_database_data(FILE *fp, char *words_to_read) {
    if (!fp) {
        return NULL;
    }
    MarkovChain *markov_chain = create_markovchain(print_func, comp_func, free,
                                                   copy_func,
                                                   is_last);
    if (markov_chain == NULL) {
        return NULL;
    }
    int words_to_read1 = (int) strtol(words_to_read, NULL, 10);
    if (fill_database(fp, words_to_read1, markov_chain) == EXIT_FAILURE) {
        fprintf(stdout, ALLOCATION_FAILURE);
        free_markov_chain(&markov_chain);
        return NULL;
    }
    return markov_chain;
}


int main(int argc, char *argv[]) {
    if (argc != MAX_ARG && argc != MIN_ARG) {
        fprintf(stdout, USAGE ERROR_MSG);
        return EXIT_FAILURE;
    }
    int seed = (int) strtol(argv[1], NULL, 10);
    srand(seed);
    int tweets_num = (int) strtol(argv[2], NULL, 10);
    FILE *fp = fopen(argv[3], "r");
    if (fp == NULL) {
        fprintf(stdout, ERROR FILE_PATH_NOT_FOUND);
        return EXIT_FAILURE;
    }
    MarkovChain *markov_chain;
    if (argc == MAX_ARG) {
        markov_chain = fill_database_data(fp, argv[4]);
    } else {
        markov_chain = fill_database_data(fp, "-1");
    }
    if (markov_chain == NULL) {
        fclose(fp);
        fprintf(stdout, ALLOCATION_FAILURE);
        return EXIT_FAILURE;
    }
    for (int i = 1; i <= tweets_num; i++) {
        printf("Tweet %d: ", i);
        generate_random_sequence(markov_chain, NULL, 20);
        printf("\n");
    }
    free_markov_chain(&markov_chain);
    fclose(fp);
    return EXIT_SUCCESS;
}


