#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (!new)
        return NULL;
    INIT_LIST_HEAD(new);
    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list)
        q_release_element(entry);
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }

    list_add(&new->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;

    element_t *new = malloc(sizeof(element_t));
    if (!new)
        return false;
    new->value = strdup(s);
    if (!new->value) {
        free(new);
        return false;
    }

    list_add_tail(&new->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *tmp = list_first_entry(head, element_t, list);
    list_del(head->next);
    if (sp) {
        strncpy(sp, tmp->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    return tmp;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;

    element_t *tmp = list_last_entry(head, element_t, list);
    list_del(&tmp->list);
    if (sp) {
        strncpy(sp, tmp->value, bufsize);
        sp[bufsize - 1] = '\0';
    }

    return tmp;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head *fast = head->next;
    struct list_head *slow = head->next;
    for (; fast != head && fast != head->prev;
         fast = fast->next->next, slow = slow->next) {
    }
    list_del(slow);
    q_release_element(list_entry(slow, element_t, list));

    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;

    bool dup_last = false;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list)
        if (&safe->list != head && !strcmp(entry->value, safe->value)) {
            list_del(&entry->list);
            q_release_element(entry);
            dup_last = true;
        } else if (dup_last) {  // del dup last one
            list_del(&entry->list);
            q_release_element(entry);
            dup_last = false;
        }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        if (safe != head) {
            list_move(node, safe);
            safe = node->next;
        }
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head))
        return;

    struct list_head *node, *safe, *start = head;
    int count_k = 0, count_turn = 0;
    int turn = q_size(head) / k;

    list_for_each_safe (node, safe, head) {
        list_move(node, start);
        if (count_turn == turn) /*no complete k-group*/
            return;
        if (++count_k == k) { /*change list head per kth node*/
            start = safe->prev;
            count_turn++;
            count_k = 0;
            continue;
        }
    }
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

int compare(struct list_head *a, struct list_head *b)
{
    if (a == b)
        return 0;
    int res = list_entry(a, element_t, list)->value -
              list_entry(b, element_t, list)->value;
    return res;
}

static void build_prev_link(struct list_head *head)
{
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head) {
        safe->prev = node;
    }

    /* The final links to make a circular doubly-linked list */
    node->next = head;
    head->prev = node;
}

struct list_head *merge(struct list_head *a, struct list_head *b)
{
    struct list_head *head = NULL;
    struct list_head **tail = &head;

    for (;;) {
        if (compare(a, b) <= 0) {
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

#define SORT_BUFSIZE 32

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || head->next == head->prev)
        return;

    struct list_head *pending[SORT_BUFSIZE] = {};
    struct list_head *node, *safe;
    int i;

    head->prev->next = NULL;

    list_for_each_safe (node, safe, head) {
        node->next = NULL;

        for (i = 0; i < SORT_BUFSIZE && pending[i]; i++) {
            node = merge(pending[i], node);
            pending[i] = NULL;
        }

        if (i == SORT_BUFSIZE)
            i--;
        pending[i] = node;
    }

    struct list_head *result = NULL;
    for (i = 0; i < SORT_BUFSIZE; i++) {
        result = merge(pending[i], result);
    }
    build_prev_link(result);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    return 0;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}
