//
// Created by Admin on 08/12/2022.
//
#include "markov_chain.h"
#include <stdio.h>
#include <string.h>

#define MAX_LENGTH 20

int get_random_number(int max_number) {
    return rand() % max_number;
}

MarkovChain *create_markovchain(PrintFunc print_func, CompFunc comp_func,
                                FreeData free_data, CopyFunc copy_func,
                                IsLast is_last) {
    MarkovChain *new_chain = malloc(sizeof(MarkovChain));
    if (new_chain == NULL) {
        return NULL;
    }
    LinkedList *new_list = malloc(sizeof(LinkedList));
    if (new_list == NULL) {
        return NULL;
    }
    new_chain->database = new_list;
    new_chain->is_last = is_last;
    new_chain->copy_func = copy_func;
    new_chain->comp_func = comp_func;
    new_chain->free_data = free_data;
    new_chain->print_func = print_func;
    return new_chain;
}

/**
 * return the node in the index place.
 * @param markov_chain - pointer to the chain.
 * @param index - the place's number.
 * @return - the i-th node.
 */
Node *find_index_node(MarkovChain *markov_chain, int index) {
    if (markov_chain == NULL) {
        return NULL;
    }
    Node *to_return = markov_chain->database->first;
    for (int i = 0; i < index; i++) {
        to_return = to_return->next;
    }
    return to_return;
}

MarkovNode *get_first_random_node(MarkovChain *markov_chain) {
    if (markov_chain == NULL) {
        return NULL;
    }
    int i = get_random_number(markov_chain->database->size);
    Node *temp = find_index_node(markov_chain, i);
    while (markov_chain->is_last(temp->data->data) == true) {
        i = get_random_number(markov_chain->database->size);
        temp = find_index_node(markov_chain, i);
    }
    return temp->data;
}


MarkovNode *get_next_random_node(MarkovNode *state_struct_ptr) {
    if (state_struct_ptr == NULL) {
        return NULL;
    }
    NextNodeCounter *cur = state_struct_ptr->counter_list;
    int i = get_random_number(state_struct_ptr->freq_sum);
    int index = 0;
    while (i > 0) {
        i -= cur[index].frequency;
        if (i >= 0) {
            index += 1;
        }
    }
    return cur[index].markov_node;
}

void generate_random_sequence(MarkovChain *markov_chain, MarkovNode *
first_node, int max_length) {
    if (markov_chain == NULL) {
        return;
    }
    if (first_node == NULL) {
        first_node = get_first_random_node(markov_chain);
    }
    markov_chain->print_func(first_node->data);
    MarkovNode *ros = get_next_random_node(first_node);
    markov_chain->print_func(ros->data);
    max_length = max_length - 2;
    while (markov_chain->is_last(ros->data) == false && max_length > 0) {
        ros = get_next_random_node(ros);
        markov_chain->print_func(ros->data);
        max_length -= 1;
    }
}

void free_markov_chain(MarkovChain **markov_chain) {
    if (markov_chain == NULL || *markov_chain == NULL) {
        return;
    }
    Node *cur = (*markov_chain)->database->first, *save;
    for (int i = 0; i < (*markov_chain)->database->size; i++) {
        (*markov_chain)->free_data(cur->data->data);
        free(cur->data->counter_list);
        free(cur->data);
        cur->data->data =NULL;
        cur->data->counter_list = NULL;
        cur->data = NULL;
        save = cur->next;
        cur->next = NULL;
        free(cur);
        cur = save;
    }
    free((*markov_chain)->database);
    free((*markov_chain));
    *markov_chain = NULL;
}


/**
 *
 * @param lst - pointer to list to resize.
 * @return 1 if the resize is safe, 0 else.
 */
int check_realloc(NextNodeCounter **lst, size_t new_length) {
    NextNodeCounter *new_lst = realloc(*lst, new_length);
    if (new_lst == NULL) {
        return 0;
    }
    *lst = new_lst;
    return 1;
}

bool add_node_to_counter_list(MarkovNode *first_node, MarkovNode
*second_node, MarkovChain *markov_chain) {
    if (first_node == NULL || second_node == NULL) {
        return false;
    }
    if (markov_chain->is_last(first_node->data) == true) {
        return true;
    }
    NextNodeCounter *lst = first_node->counter_list;
    int counter = 0;
    int index = 0;
    while (counter < first_node->freq_sum) {
        if (markov_chain->comp_func(lst[index].markov_node->data,
                                    second_node->data) == 0) {
            first_node->freq_sum += 1;
            lst[index].frequency += 1;
            return true;
        }
        counter += lst[index].frequency;
        index += 1;
    }
    if ((check_realloc(&first_node->counter_list,
                       (index + 1) * sizeof(NextNodeCounter))) == 0) {
        return false;
    }
    first_node->freq_sum += 1;
    NextNodeCounter new = {second_node, 1};
    first_node->counter_list[index] = new;
    return true;
}

Node *get_node_from_database(MarkovChain *markov_chain, void *data_ptr) {
    if (markov_chain == NULL || data_ptr == NULL) {
        return NULL;
    }
    Node *cur = NULL;
    cur = markov_chain->database->first;
    for (int i = 0; i < markov_chain->database->size; i++) {
        if (markov_chain->comp_func(cur->data->data, data_ptr) == 0) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

Node *add_to_database(MarkovChain *markov_chain, void *data_ptr) {
    if (data_ptr == NULL) {
        return NULL;
    }
    Node *check = get_node_from_database(markov_chain, data_ptr);
    if (check != NULL) {
        return check;
    }
    MarkovNode *new_mnode = malloc(sizeof(MarkovNode));
    int check_in_list = add(markov_chain->database, new_mnode);
    if (check_in_list || new_mnode == NULL) {
        return NULL;
    }
    void *mnode_data = markov_chain->copy_func(data_ptr);
    if (mnode_data == NULL) {
        return NULL;
    }
    new_mnode->data = mnode_data;
    new_mnode->freq_sum = 0;
    new_mnode->counter_list = NULL;
    return markov_chain->database->last;
}

