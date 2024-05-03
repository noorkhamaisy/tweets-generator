#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY -1
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60

#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20

#define ARG_NUM 3
#define ERROR "Error: "
#define USAGE "Usage: "
#define ERROR_MSG "Invalid number of arguments.\n"
#define ALLOCATION_FAILURE "Allocation failure: \n"
#define BOARD_ERROR "Couldn't create board. \n"

/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */

typedef struct Cell {
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents the jump of the ladder in case
    // there is one from this square
    int snake_to;  // snake_to represents the jump of the snake in
    // case there is one from this square
    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;


static void *copy_func(void *data) {
    Cell *dest = malloc(sizeof(Cell));
    if (dest == NULL) {
        return NULL;
    }
    Cell *src = (Cell *) data;
    dest->ladder_to = src->ladder_to;
    dest->snake_to = src->snake_to;
    dest->number = src->number;
    return dest;
}

static int comp_func(void *data1, void *data2) {
    Cell *cell_of_data1 = (Cell *) data1;
    Cell *cell_of_data2 = (Cell *) data2;
    if (cell_of_data1->number < cell_of_data2->number) {
        return -1;
    }
    if (cell_of_data1->number > cell_of_data2->number) {
        return 1;
    }
    return 0;
}

static bool is_last(void *data) {
    Cell *cell_of_data = (Cell *) data;
    if (cell_of_data->number == BOARD_SIZE) {
        return true;
    }
    return false;
}

static void print_func(void *data) {
    Cell *cell_of_data = (Cell *) data;
    if (is_last(data) == true) {
        printf("[%d]", cell_of_data->number);
    } else if (cell_of_data->snake_to != EMPTY) {
        printf("[%d]-snake to %d -> ", cell_of_data->number,
               cell_of_data->snake_to);
    } else if (cell_of_data->ladder_to != EMPTY) {
        printf("[%d]-ladder to %d -> ", cell_of_data->number,
               cell_of_data->ladder_to);
    } else {
        printf("[%d] -> ", cell_of_data->number);
    }
}


/** Error handler **/
static int handle_error(char *error_msg, MarkovChain **database) {
    printf("%s", error_msg);
    if (database != NULL) {
        free_markov_chain(database);
    }
    return EXIT_FAILURE;
}


static int create_board(Cell *cells[BOARD_SIZE]) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        cells[i] = malloc(sizeof(Cell));
        if (cells[i] == NULL) {
            for (int j = 0; j < i; j++) {
                free(cells[j]);
            }
            handle_error(ALLOCATION_ERROR_MASSAGE, NULL);
            return EXIT_FAILURE;
        }
        *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
    }

    for (int i = 0; i < NUM_OF_TRANSITIONS; i++) {
        int from = transitions[i][0];
        int to = transitions[i][1];
        if (from < to) {
            cells[from - 1]->ladder_to = to;
        } else {
            cells[from - 1]->snake_to = to;
        }
    }
    return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int fill_database(MarkovChain *markov_chain) {
    Cell *cells[BOARD_SIZE];
    if (create_board(cells) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    MarkovNode *from_node = NULL, *to_node = NULL;
    size_t index_to;
    for (size_t i = 0; i < BOARD_SIZE; i++) {
        add_to_database(markov_chain, cells[i]);
    }

    for (size_t i = 0; i < BOARD_SIZE; i++) {
        from_node = get_node_from_database(markov_chain, cells[i])->data;

        if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY) {
            index_to = MAX(cells[i]->snake_to, cells[i]->ladder_to) - 1;
            to_node = get_node_from_database(markov_chain, cells[index_to])
                    ->data;
            add_node_to_counter_list(from_node, to_node, markov_chain);
        } else {
            for (int j = 1; j <= DICE_MAX; j++) {
                index_to = ((Cell *) (from_node->data))->number + j - 1;
                if (index_to >= BOARD_SIZE) {
                    break;
                }
                to_node = get_node_from_database(markov_chain, cells[index_to])
                        ->data;
                add_node_to_counter_list(from_node, to_node, markov_chain);
            }
        }
    }
    // free temp arr
    for (size_t i = 0; i < BOARD_SIZE; i++) {
        free(cells[i]);
    }
    return EXIT_SUCCESS;
}


static MarkovChain *fill_board_with_data() {
    MarkovChain *markov_chain = create_markovchain(print_func,comp_func, free,
                                                    copy_func,
                                                   is_last);
    if (markov_chain == NULL) {
        return NULL;
    }
    if (fill_database(markov_chain) == EXIT_FAILURE) {
        return NULL;
    }
    return markov_chain;
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
    if (argc != ARG_NUM) {
        fprintf(stdout, USAGE ERROR_MSG);
        return EXIT_FAILURE;
    }
     int seed = ( int) strtol(argv[1], NULL, 10);
    srand(seed);
    int paths_num = (int) strtol(argv[2], NULL, 10);
    MarkovChain *markov_chain = fill_board_with_data();
    if (markov_chain == NULL) {
        fprintf(stdout, USAGE ALLOCATION_FAILURE);
        return EXIT_FAILURE;
    }
    for (int i = 1; i <= paths_num; i++) {
        printf("Random Walk %d: ", i);
        generate_random_sequence(markov_chain,
                                 markov_chain->database->first->data,
                                 MAX_GENERATION_LENGTH);
        printf("\n");
    }
    free_markov_chain(&markov_chain);
    return EXIT_SUCCESS;
}
