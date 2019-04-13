#include "hardware_stack.h"

static int checkStack(int* stack, int len) {
  for(int i=0;i<len;i++) {
    if(stack[i]!=stackPattern) return (i-1)*sizeof(int);
  }
  return len*sizeof(int);
}

int checkFIQStack() {return checkStack(Stack_FIQ,FIQ_Stack_Size/sizeof(int));}
int checkIRQStack() {return checkStack(Stack_IRQ,IRQ_Stack_Size/sizeof(int));}
int checkUSRStack() {return checkStack(Stack_USR,USR_Stack_Size()/sizeof(int));}
