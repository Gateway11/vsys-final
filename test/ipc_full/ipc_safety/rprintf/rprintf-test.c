//
//  rprintf-test.c
//
//  Created by 代祥 on 2023/6/22.
//

#include <stdlib.h>
#include <thread.h>
#include <lib/console.h>
#include <kernel/timer.h>

#include <rprintf.h>

static int32_t thread_loop(void * para) {
  uint32_t tid = *((uint32_t *)para), idx = 0;
  srand(current_time());

  while(true) {
    char str[RECV_DATA];
    uint32_t num = rand() & (128 - 1);

    for (uint32_t i = 0; i < num; i++) {
      str[i] = i;
    }
    str[num] = '\0';
    rprintf("%02d, %03d, %08ums, %s\n", tid, idx++, current_time(), str);
  }
  return 0;
}

static int cmd_rprintf(int argc, const cmd_args *argv) {
  uint32_t num_threads = 2;
  if (argc > 1) 
    num_threads = argv[1].u;

  for (uint32_t i = 10; i < (10 + num_threads); i++) {
    thread_detach_and_resume(thread_create(__func__, thread_loop, &i, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE));
  }
  return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("rprintf", "rprintf test", cmd_rprintf)
STATIC_COMMAND_END(rprintf_test);
