#ifndef ESPMISSINGINCLUDES_H
#define ESPMISSINGINCLUDES_H

// Wow. Apparently I'm required to declare these SDK methods myself.
void ets_isr_attach(int intr, void (*handler)(void *), void *arg);
void ets_isr_mask(unsigned intr);
void ets_isr_unmask(unsigned intr);

#endif