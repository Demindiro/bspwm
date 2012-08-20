#include <stdio.h>
#include <math.h>
#include <xcb/xcb.h>
#include <xcb/xcb_event.h>
#include "utils.h"
#include "types.h"
#include "tree.h"

bool is_leaf(Node *n)
{
    return (n != NULL && n->first_child == NULL && n->second_child == NULL);
}
    
void change_split_ratio(Node *n, value_change_t chg) {
    n->split_ratio = pow(n->split_ratio, (chg == CHANGE_INCREASE ? INC_EXP : DEC_EXP));
}

Node *first_extrema(Node *n)
{
    if (n == NULL)
        return NULL;
    else if (n->first_child == NULL)
        return n;
    else
        return first_extrema(n->first_child);
}

Node *second_extrema(Node *n)
{
    if (n == NULL)
        return NULL;
    else if (n->second_child == NULL)
        return n;
    else
        return second_extrema(n->second_child);
}

Node *find_fence(Node *n, direction_t dir)
{
    Node *p;

    if (n == NULL)
        return NULL;

    p = n->parent;

    while (p != NULL) {
        if ((dir == DIR_UP && p->split_type == TYPE_HORIZONTAL && p->rectangle.y < n->rectangle.y)
                || (dir == DIR_LEFT && p->split_type == TYPE_VERTICAL && p->rectangle.x < n->rectangle.x)
                || (dir == DIR_DOWN && p->split_type == TYPE_HORIZONTAL && (p->rectangle.y + p->rectangle.height) > (n->rectangle.y + n->rectangle.height))
                || (dir == DIR_RIGHT && p->split_type == TYPE_VERTICAL && (p->rectangle.x + p->rectangle.width) > (n->rectangle.x + n->rectangle.width)))
            return p;
        p = p->parent;
    }

    return NULL;
}

Node *find_neighbor(Node *n, direction_t dir)
{
    Node *fence = find_fence(n, dir);

    if (fence == NULL)
        return NULL;

    if (dir == DIR_UP || dir == DIR_LEFT)
        return second_extrema(fence->first_child);
    else if (dir == DIR_DOWN || dir == DIR_RIGHT)
        return first_extrema(fence->second_child);

    return NULL;
}

void move_fence(Node *n, direction_t dir, fence_move_t mov)
{
    Node *fence = find_fence(n, dir);
    
    if (fence == NULL)
        return;

    if ((mov == MOVE_PUSH && (dir == DIR_RIGHT || dir == DIR_DOWN)) 
            || (mov == MOVE_PULL && (dir == DIR_LEFT || dir == DIR_UP)))
        change_split_ratio(fence, CHANGE_INCREASE);
    else
        change_split_ratio(fence, CHANGE_DECREASE);
}


void rotate_tree(Node *n, rotate_t rot)
{
    Node *tmp;
    if (n == NULL)
        return;
    rotate_tree(n->first_child, rot);
    rotate_tree(n->second_child, rot);
    if ((rot == ROTATE_CLOCK_WISE && n->split_type == TYPE_HORIZONTAL)
            || (rot == ROTATE_COUNTER_CW && n->split_type == TYPE_VERTICAL)
            || rot == ROTATE_FULL_CYCLE) {
        tmp = n->first_child;
        n->first_child = n->second_child;
        n->second_child = tmp;
        n->split_ratio = 1.0 - n->split_ratio;
    }
    if (rot != ROTATE_FULL_CYCLE) {
        if (n->split_type == TYPE_HORIZONTAL)
            n->split_type = TYPE_VERTICAL;
        else if (n->split_type == TYPE_VERTICAL)
            n->split_type = TYPE_HORIZONTAL;
    }
}

void dump_tree(Node *n, char *rsp, int depth)
{
    int i;

    if (n == NULL)
        return;

    for (i = 0; i < depth; i++)
        sprintf(rsp, "%s", "  ");

    if (n->client == NULL)
        sprintf(rsp, "%s %.2f\n", (n->split_type == TYPE_HORIZONTAL ? "H" : "V"), n->split_ratio);
    else
        sprintf(rsp, "%X\n", n->client->window); 

    dump_tree(n->first_child, rsp, depth + 1);
    dump_tree(n->second_child, rsp, depth + 1);
}
