#ifndef irq_h
#define irq_h

typedef void (*fvoid)(void);

#include <cinttypes>

//The following ABSOLUTELY MUST be inlined, because it is used in startup code where we don't have a stack yet.
//When compiled with -O0, inline functions are not inlined
inline void __attribute__((always_inline)) set_cpsr_c(const uint32_t val) {
  asm volatile (" msr  cpsr_c, %0" : /* no outputs */ : "ir" (val)  );	
}
//#define set_cpsr_c(val) asm volatile (" msr  cpsr_c, %0" : /* no outputs */ : "ir" (val)  )

inline uint32_t get_cpsr_c() {
  uint32_t result;
  asm volatile (" mrs  %0, cpsr" :  "=r" (result) : /* no inputs */ );	
  return result;
}

static const uint32_t I_Bit=0x80;    // when I bit is set, IRQ is disabled 
static const uint32_t F_Bit=0x40;    // when F bit is set, FIQ is disabled 

inline void enable_irq() {
  set_cpsr_c(get_cpsr_c() & ~I_Bit);
}
inline void enable_fiq() {
  set_cpsr_c(get_cpsr_c() & ~F_Bit);
};
inline void disable_irq() {
  set_cpsr_c(get_cpsr_c() | I_Bit);
};
inline void disable_fiq(){
  set_cpsr_c(get_cpsr_c() | F_Bit);
};
inline void enable_ints() {;
  set_cpsr_c(get_cpsr_c() & ~(I_Bit|F_Bit));
}
inline void disable_ints() {
  set_cpsr_c(get_cpsr_c() | (I_Bit|F_Bit));
}

#endif
