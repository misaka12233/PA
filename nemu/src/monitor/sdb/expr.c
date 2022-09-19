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

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, 
  TK_EQ, 
  TK_DECNUM, 
  TK_HEXNUM, 
  TK_REGNUM,
  TK_NEQ,
  TK_AND,
  TK_NEG,
  TK_DEREF
  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},           // spaces
  {"\\+", '+'},                // plus
  {"==", TK_EQ},               // equal
  {"\\-", '-'},                // minus
  {"\\*", '*'},                // mutiply
  {"/", '/'},                  // divide
  {"0x[0-9]*", TK_HEXNUM},     // hexadecimal-number
  {"[0-9]+", TK_DECNUM},  // decimal-number
  {"\\(", '('},                // left bracket
  {"\\)", ')'},                // right bracket
  {"\\$.[a-z0-9]*", TK_REGNUM},// register
  {"!=", TK_NEQ},
  {"&&", TK_AND}
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE : break;
          default: 
            tokens[nr_token].type = rules[i].token_type;
            memset(tokens[nr_token].str, 0, sizeof(tokens[nr_token].str));
            memcpy(tokens[nr_token].str, substr_start, (substr_len <= 32)? substr_len : 32);
            //printf("%s\n", tokens[nr_token].str);
            nr_token++;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

bool check_parentheses(int p, int q, bool *success)
{
  int cnt = 0;
  for (int i = p; i <= q; i++)
  {
    if (tokens[i].type == '(') cnt++;
    else if (tokens[i].type == ')') cnt--;
    if (cnt < 0) *success = false;
  }
  if (cnt != 0) *success = false;
  if (*success && tokens[p].type == '(' && tokens[q].type == ')')
  {
    cnt = 0;
    for (int i = p + 1; i <= q - 1; i++)
    {
      if (tokens[i].type == '(') cnt++;
      else if (tokens[i].type == ')') cnt--;
      if (cnt < 0) return false;
    }
    return cnt == 0;
  }
  else return false;
}

uint32_t get_main_op(int p, int q, bool *success)
{
  int cnt = 0, pos = -1, rk = 0;
  for (int i = p; i <= q; i++)
  {
    switch (tokens[i].type)
    {
      case '(': 
        cnt++; 
        break;
      case ')': 
        cnt--; 
        break;
      case TK_AND:
        if (rk <= 4 && !cnt)
        {
          pos = i;
          rk = 4;
        }
        break;
      case TK_EQ: case TK_NEQ: 
        if (rk <= 3 && !cnt)
        {
          pos = i;
          rk = 3;
        }
        break;
      case '+': case '-': 
        if (rk <= 2 && !cnt)
        {
          pos = i;
          rk = 2;
        }
        break;
      case '*': case '/': 
        if (rk <= 1 && !cnt)
        {
          pos = i;
          rk = 1;
        }
        break;
      default: break;
    }
  }
  if (pos == -1) *success = false;
  return pos;
}

word_t isa_reg_str2val(const char *s, bool *success);
word_t vaddr_read(vaddr_t addr, int len);

uint32_t eval(int p, int q, bool *success) {
  if (p > q) {
    /* Bad expression */
    *success = false;
    puts("bad");
    return 0;
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    uint32_t x = 0;;
    if (tokens[p].type == TK_DECNUM)
      sscanf(tokens[p].str, "%u", &x);
    else if (tokens[p].type == TK_HEXNUM)
      sscanf(tokens[p].str, "%x", &x);
    else if (tokens[p].type == TK_REGNUM)
      x = isa_reg_str2val(tokens[p].str + 1, success);
    else
    {
      *success = false;
      puts("not number");
    }
    return x;
  }
  else if (check_parentheses(p, q, success) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    if (*success) return eval(p + 1, q - 1, success);
    else {puts("bracket"); return 0;}
  }
  else if (tokens[p].type == TK_DEREF)
    return vaddr_read(eval(p + 1, q, success), 4);
  else if (tokens[p].type == TK_NEG)
    return -eval(p + 1, q, success);
  else {
    if (!(*success)) {puts("bracket"); return 0;}
    int op = get_main_op(p, q, success);       // the position of 主运算符 in the token expression
    if (!(*success)) {puts("operator"); return 0;}
    uint32_t val1 = eval(p, op - 1, success);
    if (!(*success)) return 0;
    uint32_t val2 = eval(op + 1, q, success);
    if (!(*success)) return 0;

    switch (tokens[op].type) {
      case '+':    return val1 + val2;
      case '-':    return val1 - val2;
      case '*':    return val1 * val2;
      case TK_EQ:  return val1 == val2;
      case TK_NEQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      case '/': 
        if (val2 == 0)
        {
          *success = false;
          printf("zero: %d %d %d\n", p, q, op);
          return 0;
        }
        return val1 / val2;
      default: assert(0);
    }
  }
}

bool check_unary(int t)
{
  return (t != TK_DECNUM && t != TK_HEXNUM && t != TK_REGNUM && t != ')');
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */

  for (int i = 0; i < nr_token; i++) 
  {
    if (tokens[i].type == '*' && (i == 0 || check_unary(tokens[i - 1].type)) ) 
      tokens[i].type = TK_DEREF;
    else if (tokens[i].type == '-' && (i == 0 || check_unary(tokens[i - 1].type)) )
      tokens[i].type = TK_NEG;
  }

  return eval(0, nr_token - 1, success);
}
