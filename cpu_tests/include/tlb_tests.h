#ifndef TLB_TESTS_H_INCLUDED
#define TLB_TESTS_H_INCLUDED

void test_tlb_setup(void);
void test_tlb_tear_down(void);

void test_fifo(void);
void test_lru(void);
void test_tlb_create(void);
void test_tlb_flush(void);

#endif
