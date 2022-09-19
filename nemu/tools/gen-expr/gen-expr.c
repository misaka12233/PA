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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";
int len = 0;

void gen_num()
{
  
  int k = rand() % 5 + 1;
  if (len >= 65535) return;
  buf[len++] = rand() % 9 + '1';
  for (int i = 1; i < k; i++)
  {
    if (len >= 65535) break;
    buf[len++] = rand() % 10 + '0';
  }
  buf[len++] = 'u';
}

void gen(char c)
{
  if (len >= 65535) return;
  buf[len++] = c;
}

void gen_rand_op()
{
  if (len >= 65535) return;
  switch (rand() % 4)
  {
  case 0:
    buf[len++] = '+';
    break;
  case 1:
    buf[len++] = '-';
    break;
  case 2:
    buf[len++] = '*';
    break;
  default:
    buf[len++] = '/';
    break;
  }
}

static void gen_rand_expr() {
  switch (rand() % 3) {
    case 0: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    default: gen_num(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    len = 0;
    memset(buf, 0, sizeof(buf));
    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    int p = fscanf(fp, "%d", &result);
    p = 1;
    assert(p);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
