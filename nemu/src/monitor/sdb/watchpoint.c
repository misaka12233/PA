/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  char e[1000];
  uint32_t val;
  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

void free_wp(int NO)
{
  WP *wp = wp_pool + NO;
  if (wp == head)
    head = head->next;
  else
  {
    for (WP *i = head; i != NULL; i = i->next)
    {
      if (i->next == wp)
      {
        i->next = wp->next;
        break;
      }
    }
  }
  wp->next = free_;
  free_ = wp;
}

word_t expr(char *e, bool *success);

void new_wp(char *e)
{
  if (free_ == NULL) assert(0);
  WP* newnode = free_;
  free_ = free_->next;
  newnode->next = head;
  head = newnode;
  memset(newnode->e, 0, sizeof(newnode->e));
  strcpy(newnode->e, e);
  bool success = true;
  newnode->val = expr(e, &success);
  if (!success)
  {
    puts("wrong expression!");
    free_wp(newnode - wp_pool);
  }
}

bool check_watchpoint()
{
  bool change = false;
  for (WP *i = head; i != NULL; i = i->next)
  {
    bool success = true;
    uint32_t new_val = expr(i->e, &success);
    //printf("check watchpoint expression: %s\n", i->e);
    if (new_val != i->val)
    {
      change = true;
      printf("watchpoint %d : %s\nOld value: %u\nNew value: %u\n", i->NO, i->e, i->val, new_val);
      i->val = new_val;
    }
  }
  return change;
}

void watchpoint_display()
{
  for (WP *i = head; i != NULL; i = i->next)
    printf("watchpoint %d : %s\n     value: %u\n", i->NO, i->e, i->val);
}
