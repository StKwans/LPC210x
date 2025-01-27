#include "irq.h"
#include "hardware_stack.h"

/* Configuration requirements and constraints. I would dearly like the 
libraries/ folder, in particular the System folder, to not have to change
for each program. However, I know that I would like some things that the
System code does to be configurable. The requirements are, in priority order:

1) Make certain things configurable
2) None of the code in libraries/System has to change to be configurable
3) Configuration options away from default can be expressed in a Makefile
4) Configuration defaults are specified in the libraries/System code, not
   the Makefile

This means:

* Configuration options are controlled by macros, not constants. Boo! I would
  like things to be controlled by constants, perhaps weak constants, but since
  most of these constants will be hard-coded, they don't necessarily have
  addresses to be weakly referenced.
* Defaults are defined in libraries/System code in #ifndef blocks
* Programs can change the defaults in their own Makefiles by putting in 

CDEFS += -DConfig=Value

So for instance if you want to give the abort handler a stack, you can do it 
like this:

CDEFS += -DABT_Stack_Size=512
*/

/*
There are two reasons that Startup has to be in asm:

1. Registers CPSR and SP are not directly available to C code.
     So, we write a crumb of inline asm to access these registers,
     then write the rest in C(++).
2. Exact control of the interrupt table is needed
     So, we use more inline asm 

Other complications: Setting up the stack before the stack is available (use naked)
                     IRQ wrapper proper return (use interrupt("IRQ"))
                     Vector table can have any name, but must be in section 
		       called .vectors so that it will link to the right place

This is tightly integrated with the linker script [lo|hi]mem_arm_eabi_cpp.ld and
depends on certain symbols there with the correct name and correct location.
 */

// Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs
static const int Mode_USR=0x10;
static const int Mode_FIQ=0x11;
static const int Mode_IRQ=0x12;
static const int Mode_SVC=0x13;
static const int Mode_ABT=0x17;
static const int Mode_UND=0x1B;
static const int Mode_SYS=0x1F;

//Modes as used by Loginator-type code:
//Normal code runs in system mode, so no restricted instructions. Fine, since there is no OS.
//IRQs run in IRQ mode with irqs disabled
//FIQ runs in FIQ mode with irqs and fiqs disabled, if we ever need/write an FIQ handler
//System starts in Supervisor mode but shifts to system mode before running main()

//A symbol is simply a name for an address. So, all of these are addresses. But,
//the best match in C or C++ is to define them as arrays, which are always usable
//as named pointer constants. Frequently we want to treat them as arrays, anyway.
//Generally sections are delimited by start and end symbols, which can be subtracted
//to get the size of the section. The start symbol is perfectly usable as an
//array, and the end symbol, while never used directly, is the same array type as
//the start symbol so that their pointers are compatible.
//All of these symbols are created by the linker, and their names are controlled
//by the linker script.
extern int bss_start[];
extern int bss_end[];
extern int bdata[];
extern int data[];
extern int edata[];
//ctors_start actually points to a proper array, so we can actually use 
//the mechanism as intended. It is an array of pointers to void functions.
typedef void (*fvoid)(void);
extern fvoid ctors_start[];
extern fvoid ctors_end[];

// Declarations for exception handlers. Definitions are below reset handler
// so that they appear after it in binary.

extern "C" void Undef_Handler(void);
extern "C" void SWI_Handler(void);
extern "C" void PAbt_Handler(void);
extern "C" void DAbt_Handler(void);
extern "C" void FIQ_Handler(void);
/**This fills the vtable slots of a class where the method is abstract. Why not
just zero? That would cause a spontaneous reset on an ARM processor, and "Thou
shalt not follow the NULL pointer, for chaos and madness await thee at its
end." But, if you call an abstract method, you get what you deserve.
Defined as weak so that if you want to write a function that beats the
programmer about the head when called, you can. Defined as extern C
because the compiler generates references to this exact function name. */
extern "C" void __cxa_pure_virtual();

//Sketch main routines. We actually put the symbol weakness to work here - we
//expect to replace these functions.
void __attribute__ ((weak)) setup(void) {}
void __attribute__ ((weak)) loop(void) {}

//No setup code because stack isn't available yet
//No cleanup code because function won't return (what would it return to?)
extern "C" void __attribute__ ((naked))            Reset_Handler(void);

extern "C" void __attribute__ ((naked))
__attribute__ ((section(".vectors")))
vectorg(void) {
  asm("ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      ".word 0xb8a06f58\n\t" //NXP checksum, constant as long as the other 7 instructions in first 8 are constant
      "ldr pc,[pc,#24]\n\t"
      "ldr pc,[pc,#24]\n\t"
      ".word Reset_Handler\n\t"   //Startup code location
      ".word Undef_Handler\n\t"   //Undef
      ".word SWI_Handler\n\t"   //SWI
      ".word PAbt_Handler\n\t"   //PAbt
      ".word DAbt_Handler\n\t"   //DAbt
      ".word 0\n\t"               //Reserved (hole in vector table above)
      ".word IRQ_Wrapper\n\t"     //IRQ (wrapper so that normal C functions can be installed in VIC)
      ".word FIQ_Handler"); //FIQ
}

//These two routines ABSOLUTELY MUST be inlined. Without the __attribute__((always_inline)) we don't get inlined at optimization level -O0
static inline void __attribute__((always_inline)) set_sp(const int* val) {
  // Must be inlined since we monkey with the stack pointer and are likely to lose our
  // return address. ARM chips use a link register which doesn't hit the stack,
  // but the compiler may do so in a function prolog/epilog.
  asm volatile (" mov  sp, %0" : : "r" (val));
}

static inline void __attribute__((always_inline)) setModeStack(int stack[], const int size, const int modeflags) {
  // Must be inlined since we monkey with the stack pointer and are likely to lose our
  // return address. ARM chips use a link register which doesn't hit the stack,
  // but the compiler may do so in a function prolog/epilog.
  set_cpsr_c(modeflags);
  set_sp(stack+size/sizeof(int));
  for(unsigned int i=0;i<(size/sizeof(int));i++) stack[i]=stackPattern;
}
/*
#define setModeStack(stack,size,modeflags)  set_cpsr_c(modeflags);\
     set_sp(stack+size/sizeof(int)); \
     for(unsigned int i=0;i<(size/sizeof(int));i++) stack[i]=stackPattern 
*/

void Reset_Handler() {
  //Fill the stack space with a known pattern, so we can check stack usage

  //Set up stacks...
  setModeStack(Stack_UND,UND_Stack_Size  , Mode_UND | I_Bit | F_Bit);  //...for Undefined Instruction mode
  setModeStack(Stack_ABT,ABT_Stack_Size  , Mode_ABT | I_Bit | F_Bit);  //...for Abort mode
  setModeStack(Stack_FIQ,FIQ_Stack_Size  , Mode_FIQ | I_Bit | F_Bit);  //...for FIQ mode
  setModeStack(Stack_IRQ,IRQ_Stack_Size  , Mode_IRQ | I_Bit | F_Bit);  //...for IRQ mode
  setModeStack(Stack_SVC,SVC_Stack_Size  , Mode_SVC | I_Bit | F_Bit);  //...for Supervisor mode
  setModeStack(Stack_USR,USR_Stack_Size(), Mode_SYS | I_Bit | F_Bit);  //...for User and System mode. Interrupts are on, but nothing should have any interrupts set up yet

//Now stay in system mode for good                

// We get to be part of the implementation at this point. The implementation is required to do the following
//   in the following order. Section 6.2.2 of C++17 now says do either zero-initialization or constant
//   initialization, which then subsumes the zero-initialization. It looks like g++ with -std=c++17 still
//   does separate constant and zero initialization, which saves space in the constant initialization image.
//   Order of static and zero initialization don't matter, because no code can run in between these two
//   initializations.

// Static-initialize the memory for the static-intialized static variables (constant initialization). The G++ compiler builds a .data.*
// section for each variable in each translation unit, calculating the values at compile-time. The linker 
// combines all of the sections into the .data section of the final object, and provides symbols data and edata
// at each end of the section in RAM where the variables will live, and the symbol bdata pointing to the beginning
// of the image of the section in ROM. This code then copies that section from ROM to RAM. 
  for(int i=0;i<(edata-data);i++) data[i]=bdata[i];
// Zero-initialize the memory for the the zero-initialized static variables (zero initialization). The G++ compiler builds a .bss.*
// section for each zero-initialized variable. The linker combines them all into a .bss section with no image in the final
// object, and symbols bss_start and bss_end at each end of the section in RAM. This code then zeros out that section of RAM.
  for(int i=0;i<(bss_end-bss_start);i++) bss_start[i]=0;

// call C++ constructors of global objects (Dyanmic initialization). G++ compiler makes a function (_GLOBAL__sub_I__*) 
// for each translation unit which is equivalent to calling constructors for static objects, and calling functions
// needed to calculate initialization values for non-objects. It also makes a section which has a pointer
//Each pointer is the pointer to a short block of code, and appears to be a static
//function which which constructs all the global objects in the same compilation unit.
//This code is free to call constructors and/or perform inline constructors directly.
//Note that the compiler is also free to construct an object by putting data into
//the .data pre-initialized variable section.
//As a static function, the pointee code uses BX lr to return
  for(int i=0;i<ctors_end-ctors_start;i++) ctors_start[i]();

//We have finally broken the tyranny of main(). Directly call user's setup()
  setup();
//Directly call user's loop();
  for(;;) loop();
}

//Handlers for various exceptions, defined as weak so that they may be replaced
//by user code. These default handlers just go into infinite loops. Reset and
//IRQ are handled by real (strong) functions, as they do the right thing and
//it doesn't make sense for user code to replace them.
void __attribute__ ((weak,noreturn)) Undef_Handler(void) {for(;;);}
void __attribute__ ((weak,alias("Undef_Handler"))) SWI_Handler(void);
void __attribute__ ((weak,alias("Undef_Handler"))) PAbt_Handler(void);
void __attribute__ ((weak,alias("Undef_Handler"))) DAbt_Handler(void);
void __attribute__ ((weak,alias("Undef_Handler"))) FIQ_Handler(void);
/**This fills the vtable slots of a class where the method is abstract. Why not
just zero? That would cause a spontaneous reset on an ARM processor, and "Thou
shalt not follow the NULL pointer, for chaos and madness await thee at its
end." But, if you call an abstract method, you get what you deserve.
Defined as weak so that if you want to write a function that beats the
programmer about the head when called, you can. Defined as extern C
because the compiler generates references to this exact function name. */
void __attribute__ ((weak,alias("Undef_Handler"))) __cxa_pure_virtual();

#include "scb.h"
#include "vic.h"
