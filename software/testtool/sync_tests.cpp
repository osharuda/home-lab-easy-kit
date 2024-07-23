#include "sync_tests.hpp"
#include <cassert>
#include <testtool.hpp>

#include <mutex>
#define SEQ_LOCK_TEST 1
#include "seqlock.h"


size_t reader_calls_counter = 100000000;
size_t writer_calls_counter = 100000000;

void test_seq_lock_multithread_reader(struct sequential_lock* lock, volatile uint32_t* data) {

    do {
        seq_lock_read_acquire(lock);
        uint32_t r = *data;
        assert((r & 1) == 0); // Must not be odd value
        seq_lock_read_release(lock);

        reader_calls_counter--;
    } while (reader_calls_counter);
}
void test_seq_lock_multithread_writer(struct sequential_lock* lock, volatile uint32_t* data) {

    do {
        seq_lock_write_acquire(lock) ;
        // Do nothing here
        seq_lock_write_update(lock) ;
        *data = *data+1;
        *data = *data+1;
        seq_lock_write_release(lock);

        writer_calls_counter--;
    } while(writer_calls_counter);
}
void test_seq_lock_multithread() {
    DECLARE_TEST(test_seq_lock_multithread)

    REPORT_CASE
    {
        struct sequential_lock lock;
        std::mutex mtx;
        uint32_t data = 0;
        seq_lock_init(&lock, &mtx);

        std::thread reader_thread(test_seq_lock_multithread_reader, &lock, &data);
        std::thread writer_thread(test_seq_lock_multithread_writer, &lock, &data);

        reader_thread.join();
        writer_thread.join();
    }
}


void test_safe_mutex() {
    DECLARE_TEST(test_safe_mutex)
    REPORT_CASE
    {
        tools::safe_mutex a;
        a.lock();
        CHECK_SAFE_MUTEX_LOCKED(a);
        a.unlock();
    }

    REPORT_CASE
    {
        tools::safe_mutex a;
        tools::safe_mutex b;
        tools::safe_mutex c;
        a.lock();
        CHECK_SAFE_MUTEX_LOCKED(a);
        b.lock();
        CHECK_SAFE_MUTEX_LOCKED(b);
        c.lock();
        CHECK_SAFE_MUTEX_LOCKED(c);

        c.unlock();
        b.unlock();
        a.unlock();
    }
}