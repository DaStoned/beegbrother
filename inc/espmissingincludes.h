#ifndef ESPMISSINGINCLUDES_H
#define ESPMISSINGINCLUDES_H

#ifdef __cplusplus
extern "C" {
#endif

// Wow. Apparently I'm required to declare these SDK methods myself.
void ets_isr_attach(int intr, void (*handler)(void *), void *arg);
void ets_isr_mask(unsigned intr);
void ets_isr_unmask(unsigned intr);

#ifdef __cplusplus
}
#endif

#endif