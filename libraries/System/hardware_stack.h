#ifndef HARDWARE_STACK_H
#define HARDWARE_STACK_H



//Stack sizes are in bytes
#ifndef UND_Stack_Size
#define UND_Stack_Size 0  //No stack for you! You are just an infinite loop
#endif
#ifndef SVC_Stack_Size
#define SVC_Stack_Size 0  //Likewise
#endif
#ifndef ABT_Stack_Size
#define ABT_Stack_Size 0  //Likewise
#endif
#ifndef FIQ_Stack_Size
#define FIQ_Stack_Size 0
#endif
#ifndef IRQ_Stack_Size
#define IRQ_Stack_Size 64
#endif
extern int stack_start[];
extern int ram_end[];

static inline __attribute__((always_inline)) int USR_Stack_Size() {
  return (ram_end-stack_start)*sizeof(int)-UND_Stack_Size
                                          -SVC_Stack_Size
                                          -ABT_Stack_Size
                                          -FIQ_Stack_Size
                                          -IRQ_Stack_Size;
}

const int stackPattern=0x6E61774B; // Appears as "Kwan" in little-endian ascii

int * const Stack_USR=stack_start;
int * const Stack_IRQ=Stack_USR+USR_Stack_Size/sizeof(int);
int * const Stack_FIQ=Stack_IRQ+IRQ_Stack_Size/sizeof(int);
int * const Stack_ABT=Stack_FIQ+FIQ_Stack_Size/sizeof(int);
int * const Stack_SVC=Stack_ABT+ABT_Stack_Size/sizeof(int);
int * const Stack_UND=Stack_SVC+SVC_Stack_Size/sizeof(int);

//Each one returns number of bytes on each stack which have never been touched
//rounded down to nearest multiple of 4. Negative if every byte has been touched
int checkFIQStack();
int checkIRQStack();
int checkUSRStack();

#endif
