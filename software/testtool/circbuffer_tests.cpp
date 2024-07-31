#include "circbuffer_tests.hpp"
#include "testtool.hpp"
#include "utools.h"
#include "circbuffer.h"
#include <signal.h>
#include <string.h>

#define COMPARE_CIRC_WITH_REF_CIRC                                                                                      \
    /* Validate buffer pointer */                                                                                       \
    assert(circ.buffer == buffer);                                                                                      \
    assert(refcirc.buffer == refbuffer);                                                                                \
                                                                                                                        \
    /* Validate buffer end pointer */                                                                                   \
    assert(circ.buffer_end == buffer + buffer_size);                                                                    \
    assert(refcirc.buffer_end == refbuffer + buffer_size);                                                              \
                                                                                                                        \
    /* Validate writer state */                                                                                         \
    assert( (circ.writer_state.put_ptr - buffer) == (refcirc.writer_state.put_ptr - refbuffer));                        \
    assert( (circ.writer_state.put_ptr >= buffer) && (circ.writer_state.put_ptr < circ.buffer_end));                    \
    assert( (refcirc.writer_state.put_ptr >= refbuffer) && (refcirc.writer_state.put_ptr < refcirc.buffer_end));        \
                                                                                                                        \
    /* Validate reader state */                                                                                         \
    assert( (circ.reader_state.get_ptr - buffer) == (refcirc.reader_state.get_ptr - refbuffer));                        \
    assert( (circ.reader_state.get_ptr >= buffer) && (circ.reader_state.get_ptr < circ.buffer_end));                    \
    assert( (refcirc.reader_state.get_ptr >= refbuffer) && (refcirc.reader_state.get_ptr < refcirc.buffer_end));        \
    assert( (circ.reader_state.bytes_read) == (refcirc.reader_state.bytes_read));                                       \
    assert( (circ.reader_state.bytes_read) <= buffer_size+status_size );                                                \
    assert( (circ.reader_state.reader_ptr - buffer) == (refcirc.reader_state.reader_ptr - refbuffer));                  \
    assert( (circ.reader_state.reader_ptr >= buffer) && (circ.reader_state.reader_ptr < circ.buffer_end));              \
    assert( (refcirc.reader_state.reader_ptr >= refbuffer) && (refcirc.reader_state.reader_ptr < refcirc.buffer_end));  \
                                                                                                                        \
    /* Validate status */                                                                                               \
    assert( ( (circ.status == nullptr) && (refcirc.status == nullptr) ) ||                                              \
            ( (circ.status == status_buffer) && (refcirc.status == refstatus_buffer) ));                                \
                                                                                                                        \
    /* Validate put and get counters */                                                                                 \
    assert(circ.put_bytes_counter == refcirc.put_bytes_counter);                                                        \
    assert(circ.get_bytes_counter == refcirc.get_bytes_counter);                                                        \
                                                                                                                        \
    /* Validate sizes */                                                                                                \
    assert(circ.buffer_size == refcirc.buffer_size);                                                                    \
    assert(circ.buffer_size == buffer_size);                                                                            \
    assert(circ.status_size == refcirc.status_size);                                                                    \
    assert(circ.free_size == buffer_size - circ.block_size);                                                            \
    assert(circ.free_size == refcirc.free_size);                                                                        \
    assert(circ.block_size == refcirc.block_size);                                                                      \
    assert(circ.block_size > 0);                                                                                        \
    assert(circ.warn_low_thr == refcirc.warn_low_thr);                                                                  \
    assert((circ.warn_low_thr >= 0) && (circ.warn_low_thr <= buffer_size));                                             \
    assert(circ.warn_high_thr == refcirc.warn_high_thr);                                                                \
    assert((circ.warn_high_thr >= 0) && (circ.warn_high_thr <= buffer_size));                                           \
                                                                                                                        \
    /* Validate block_mode */                                                                                           \
    assert(circ.block_mode == refcirc.block_mode);                                                                      \
                                                                                                                        \
    /* Validate flags */                                                                                                \
    assert(circ.ovf == refcirc.ovf);                                                                                    \
    assert( (circ.ovf == 1) || (circ.ovf == 0) );                                                                       \
    assert(circ.wrn == refcirc.wrn);                                                                                    \
    assert( (circ.wrn == 1) || (circ.wrn == 0) );                                                                       \
    assert(memcmp(circ.buffer, refcirc.buffer, circ.buffer_size) == 0);

/// \brief Declares buffers and constants required for the circular buffer test.
/// \param size - size of the circular buffer.
/// \param stats_size - size of the status buffer, 0 if not required.
/// \param blck_size - size of the block size, 1 if byte mode.
#define DECLARE_CIRC_BUFFERS(size, stats_size, blck_size) \
    uint8_t opres,b,res;                                                      \
    CircBuffer circ;                     \
    CircBuffer refcirc;                                        \
    memset(&circ, 0xBB, sizeof(CircBuffer));    \
    memset(&refcirc, 0, sizeof(CircBuffer));   \
    constexpr int32_t buffer_size = (size); \
    uint8_t buffer[buffer_size + 1] = {0};      \
    uint8_t refbuffer[buffer_size + 1] = {0};   \
    constexpr int32_t status_size = (stats_size); \
    uint8_t status_buffer[status_size + 1] = {0};      \
    uint8_t refstatus_buffer[status_size + 1] = {0};    \
    constexpr int32_t block_size = (blck_size);                \
/* Initialize refcirc */                                                        \
memset(&refcirc, 0, sizeof(CircBuffer));            \
        refcirc.buffer = refbuffer;                         \
        refcirc.buffer_end = refbuffer+buffer_size;         \
        refcirc.writer_state.put_ptr = refbuffer;           \
        refcirc.reader_state.get_ptr = refbuffer;           \
        refcirc.reader_state.reader_ptr = refbuffer;        \
        refcirc.buffer_size = buffer_size;                  \
        refcirc.free_size = buffer_size - block_size;        \
        refcirc.block_size = block_size;                     \
        refcirc.block_mode = (block_size > 1);                 \
        refcirc.warn_high_thr = buffer_size;                \
        if (status_size) {                                         \
            refcirc.status = refstatus_buffer;                          \
            refcirc.status_size = status_size;                \
        }


#define REFCIRC_START_READ \
        refcirc.reader_state.bytes_read = 0;                            \
        refcirc.reader_state.reader_ptr = refcirc.reader_state.get_ptr;

void test_circbuffer_initialization() {
    DECLARE_TEST(test_circbuffer_initialization)

    REPORT_CASE // Simple initialization
    {
        DECLARE_CIRC_BUFFERS(42, 0, 1);
        circbuf_init(&circ, buffer, buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;
    }

    REPORT_CASE // Initialization with status (state)
    {
        DECLARE_CIRC_BUFFERS(42, 4, 1);
        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_status(&circ, status_buffer, status_size);
        COMPARE_CIRC_WITH_REF_CIRC;
    };

    REPORT_CASE // Initialization with block mode
    {
        DECLARE_CIRC_BUFFERS(45, 0, 5);
        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_block_mode(&circ, 5);
        COMPARE_CIRC_WITH_REF_CIRC;
    };

    REPORT_CASE // Initialization with block mode and status (state)
    {
        DECLARE_CIRC_BUFFERS(45, 3, 5);
        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_status(&circ, status_buffer, status_size);
        circbuf_init_block_mode(&circ, 5);
        COMPARE_CIRC_WITH_REF_CIRC;
    };

    REPORT_CASE // Initialization with block mode and status (state), two last calls swapped
    {
        DECLARE_CIRC_BUFFERS(45, 3, 5);
        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_block_mode(&circ, 5);
        circbuf_init_status(&circ, status_buffer, status_size);
        COMPARE_CIRC_WITH_REF_CIRC;
    };
}

void test_circbuffer_failed_initialization() {
    DECLARE_TEST(test_circbuffer_failed_initialization)

    REPORT_CASE // Zero length buffer is not allowed
    {
        DECLARE_CIRC_BUFFERS(0, 0, 1);

        TEST_ASSERTION_BEGIN;
        circbuf_init(&circ, buffer, buffer_size);
        TEST_ASSERTION_END;
    }

    REPORT_CASE // Buffer length should be greater than block size
    {
        DECLARE_CIRC_BUFFERS(5, 0, 10);
        TEST_ASSERTION_BEGIN;
            circbuf_init(&circ, buffer, buffer_size);
            circbuf_init_block_mode(&circ, block_size);
        TEST_ASSERTION_END;
    }

    REPORT_CASE // Buffer length should be multiple to block size
    {
        DECLARE_CIRC_BUFFERS(13, 0, 5);
        TEST_ASSERTION_BEGIN;
            circbuf_init(&circ, buffer, buffer_size);
            circbuf_init_block_mode(&circ, block_size);
        TEST_ASSERTION_END;
    }
}

void test_circbuffer_single_byte() {
    DECLARE_TEST(test_circbuffer_single_byte)

    // block mode: block_size==1
    REPORT_CASE
    {
        DECLARE_CIRC_BUFFERS(1, 0, 1);

        circbuf_init(&circ, buffer, buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // Start read and verify circbuf_start_read() initializes required data.
        circ.reader_state.reader_ptr = 0;
        circ.reader_state.bytes_read = 1;
        circbuf_start_read(&circ);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==0);

        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        assert(opres==0);
        assert(b == COMM_BAD_BYTE);
        refcirc.ovf = 1;

        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==1);

        // stop reading
        res = circbuf_stop_read(&circ, 1);
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;

        opres = circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(opres == 0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // write byte (1)
        circbuf_put_byte(&circ, 1);
        refbuffer[0] = 1;
        refcirc.put_bytes_counter++;
        refcirc.wrn = 1;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==1);

        // read byte
        circbuf_start_read(&circ);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==1);

        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read = 1;

        assert(opres==1);
        assert(b==1);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);

        // stop reading
        res = circbuf_stop_read(&circ, 1);
        refcirc.wrn = 0;
        refcirc.get_bytes_counter += 1;
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // write byte
        circbuf_put_byte(&circ, 2);
        refbuffer[0] = 2;
        refcirc.put_bytes_counter++;
        refcirc.wrn = 1;

        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circ.wrn == 1);

        // write byte (overflow)
        circbuf_put_byte(&circ, 3);
        refcirc.ovf = 1;

        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==1);

        opres = circbuf_clear_ovf(&circ); // Must not work, if more data will be put it will rise again
        refcirc.ovf = 0;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==1);

        // read byte
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==1);

        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        assert(opres==1);
        assert(b==2);
        refcirc.reader_state.reader_ptr = refbuffer;
        refcirc.reader_state.bytes_read++;
        COMPARE_CIRC_WITH_REF_CIRC;

        opres = circbuf_clear_ovf(&circ);
        assert(opres == 0);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // stop reading
        res = circbuf_stop_read(&circ, 1);
        refcirc.get_bytes_counter++;
        refcirc.wrn = 0;
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // Clear overflow
        opres = circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(opres == 0);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;
    }
}

void test_circbuffer_byte_mode() {
    DECLARE_TEST(test_circbuffer_byte_mode)

    REPORT_CASE
    {
        DECLARE_CIRC_BUFFERS(10, 0, 1);
        circbuf_init(&circ, buffer, buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==0);
    }

    REPORT_CASE
    {
        DECLARE_CIRC_BUFFERS(10, 0, 1);
        circbuf_init(&circ, buffer, buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==0);

        // put a byte (1)
        circbuf_put_byte(&circ, 1);
        refbuffer[0] = 1;
        refcirc.writer_state.put_ptr++;
        refcirc.put_bytes_counter++;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);

        // start reading from circular buffer
        circbuf_start_read(&circ);
        REFCIRC_START_READ;

        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==1);


        // read a byte
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;

        assert(opres==1);
        assert(b==1);

        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==0);

        // stop reading
        int32_t len = circbuf_stop_read(&circ, 1);
        refcirc.get_bytes_counter++;
        refcirc.reader_state.get_ptr++;
        assert(len == 0); // no bytes remains

        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
    }

    REPORT_CASE
    {
        DECLARE_CIRC_BUFFERS(10, 0, 1);
        circbuf_init(&circ, buffer, buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        // Fill buffer completely
        int i;
        for (i=0; i<buffer_size; i++) {
            assert(circbuf_len(&circ)==i);
            circbuf_put_byte(&circ, (uint8_t)i);
            refcirc.writer_state.put_ptr++;
            if (i+1==buffer_size) {
                refcirc.writer_state.put_ptr = refbuffer;
                refcirc.wrn = 1;
            }
            refcirc.put_bytes_counter++;
            refbuffer[i % buffer_size] = i;
            COMPARE_CIRC_WITH_REF_CIRC;
            assert(circbuf_len(&circ)==(i+1));
            assert(circbuf_get_ovf(&circ)==0);
        }

        // Add one more byte (failed + ovf mist set)
        circbuf_put_byte(&circ, (uint8_t)i);
        refcirc.ovf = 1;
        COMPARE_CIRC_WITH_REF_CIRC;

        // Failed attempt to clear overflow
        opres = circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(opres == 0);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert_param(circbuf_len(&circ)==buffer_size);

        // Read data
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refcirc.reader_state.get_ptr;
        COMPARE_CIRC_WITH_REF_CIRC;

        for (i=buffer_size-1; i>=0; i--) {
            circbuf_get_byte(&circ, &b);
            refcirc.reader_state.reader_ptr++;
            refcirc.reader_state.bytes_read++;
            if (i==0) {
                refcirc.reader_state.reader_ptr = refbuffer;
            }
            assert(b==buffer_size-i-1);
            assert(circbuf_len(&circ)==buffer_size);
            COMPARE_CIRC_WITH_REF_CIRC;

            // Clear ovw and wrn - both must be set until circbuf_stop_read() is called.
            circbuf_clear_ovf(&circ);
            refcirc.ovf = 0;
            refcirc.wrn = 1;
            assert(circbuf_get_ovf(&circ)==0);
            COMPARE_CIRC_WITH_REF_CIRC;
        }

        // Stop read. Note: overflow is not cleared!
        circbuf_stop_read(&circ, buffer_size);
        refcirc.get_bytes_counter += buffer_size;
        refcirc.reader_state.get_ptr = refbuffer;
        refcirc.wrn = 0;
        refcirc.ovf = 0;
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // Clear overflow
        opres = circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(opres == 0);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;
    }

    REPORT_CASE {
        DECLARE_CIRC_BUFFERS(10, 0, 1);
        circbuf_init(&circ, buffer, buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        // start reading from circular buffer
        circbuf_start_read(&circ);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);

        // Read a byte from empty buffer, must not change state, but overflow should be raised!
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.ovf = 1;
        assert(opres==0);
        assert(b == COMM_BAD_BYTE);
        COMPARE_CIRC_WITH_REF_CIRC;

        // stop reading 1 byte from empty buffer, must not change state
        int32_t len = circbuf_stop_read(&circ, 1);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(len == 0); // no bytes remains
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==1);

        // Clear overflow - must be successful because buffer is empty
        opres = circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(opres == 0);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==0);
    }


    REPORT_CASE {
        DECLARE_CIRC_BUFFERS(2, 0, 1);
        circbuf_init(&circ, buffer, buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);

        // put a byte (1)
        circbuf_put_byte(&circ, 1);
        refbuffer[0] = 1;
        refcirc.writer_state.put_ptr++;
        refcirc.put_bytes_counter++;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==1);

        // put a byte (2)
        circbuf_put_byte(&circ, 2);
        refbuffer[1] = 2;
        refcirc.writer_state.put_ptr = refbuffer;
        refcirc.put_bytes_counter++;
        refcirc.wrn = 1;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==2);

        // put a byte (overflow must occur)
        circbuf_put_byte(&circ, 3);
        refcirc.ovf=1;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==2);


        // Clear overflow - must fail because buffer is full
        opres = circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(opres == 0);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==2);

        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.reader_state.reader_ptr = refbuffer;
        refcirc.reader_state.bytes_read = 0;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==2);
        assert(circ.wrn == 1);

        // read a byte (1)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.reader_ptr++;
        refcirc.reader_state.bytes_read++;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(opres==1);
        assert(b==1);

        // Attempt to clear ovw - must fail, will success only after circbuf_stop_read is called.
        opres = circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(opres == 0);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==2);

        // read a byte (2)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.reader_ptr = refbuffer;
        refcirc.reader_state.bytes_read++;
        assert(opres==1);
        assert(b==2);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==2);


        // read a byte (no bytes must be read, buffer is empty)
        // Note ovw and wrn are already set!
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.ovf = 1;

        assert(opres==0);
        assert(circ.wrn == 1);
        assert(circ.ovf == 1);
        assert(b == COMM_BAD_BYTE);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==2);

        // stop reading just 1 byte, 1 byte should remain in buffer
        int32_t len = circbuf_stop_read(&circ, 1);
        refcirc.get_bytes_counter++;
        refcirc.reader_state.get_ptr++;
        refcirc.wrn = 1; // Must not clear, because hysteresis !!!
        assert(len == 1); // no bytes remains
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==1);

        // Attempt to clear ovw - must be success
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);

        // stop reading another 1 byte (there were 2 in total)
        len = circbuf_stop_read(&circ, 1);
        refcirc.get_bytes_counter++;
        refcirc.reader_state.get_ptr = refbuffer;
        refcirc.wrn = 0; // Now, it should be cleared !!!

        assert(len == 0); // no bytes remains
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
    }

    REPORT_CASE { // test buffer with reading in the middle

        DECLARE_CIRC_BUFFERS(3, 0, 1);
        circbuf_init(&circ, buffer, buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        // put a byte (1)
        circbuf_put_byte(&circ, 1);
        refbuffer[0] = 1;
        refcirc.writer_state.put_ptr++;
        refcirc.put_bytes_counter++;

        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==1);


        // put a byte (2)
        circbuf_put_byte(&circ, 2);
        refbuffer[1] = 2;
        refcirc.writer_state.put_ptr++;
        refcirc.put_bytes_counter++;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==2);


        // put a byte (3)
        circbuf_put_byte(&circ, 3);
        refbuffer[2] = 3;
        refcirc.writer_state.put_ptr = refbuffer;
        refcirc.put_bytes_counter++;
        refcirc.wrn = 1;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==3);

        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;

        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==3);


        // Read a byte
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;

        assert(opres==1);
        assert(b==1);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==3);

        // start reading from circular buffer (again, previos read will be discarded)
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==3);

        // read that byte again
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(opres==1);
        assert(b==1);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==3);

        // stop reading this byte
        int32_t len = circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr++;
        refcirc.get_bytes_counter++;

        assert(len == 2); // no bytes remains
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==2);

        // put a byte (overflow must NOT occur)
        circbuf_put_byte(&circ, 4);
        refbuffer[0]=4;
        refcirc.writer_state.put_ptr++;
        refcirc.put_bytes_counter++;
        assert(circ.wrn == 1);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);

        // put a byte (overflow must occur)
        circbuf_put_byte(&circ, 4);
        refcirc.ovf = 1;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==3);

        // Failed attempt to clear overflow - buffer is full, problem is not resolved
        opres = circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(opres == 0);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);

        // start reading from circular buffer again (next byte will be 2)
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer + 1;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);

        // read a byte (2)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;

        assert(opres==1);
        assert(b==2);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);

        // read a byte (3)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr = refbuffer;
        assert(opres==1);
        assert(b==3);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);

        // read a byte (4)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(opres==1);
        assert(b==4);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==3);


        // read a byte (no bytes must be read, buffer is empty)
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.ovf = 1;
        assert(opres==0);
        assert(b == COMM_BAD_BYTE);
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==3);

        // stop reading 5 bytes (more than buffer allows) - buffer will read 3 bytes anyway
        res = circbuf_stop_read(&circ, 5);
        refcirc.reader_state.reader_ptr = refbuffer+1;
        refcirc.get_bytes_counter = 4;
        refcirc.wrn = 0;
        assert(circbuf_len(&circ)==0);
        assert(circ.put_bytes_counter == 4);
        assert(circ.put_bytes_counter == circ.get_bytes_counter);
        assert(res == 0); // no bytes remains
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==0);

        // stop reading another 1 byte (none left)
        len = circbuf_stop_read(&circ, 1);
        assert(len == 0); // no bytes remains
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==0);

        // Clear ovf
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
    }

}

void test_circbuffer_byte_mode_with_status() {
    DECLARE_TEST(test_circbuffer_byte_mode_with_status)

    REPORT_CASE
    {
        DECLARE_CIRC_BUFFERS(10, 1, 1);
        status_buffer[0] = 0xDA;
        refstatus_buffer[0] = 0xDA;
        refstatus_buffer[1] = 0xFF;

        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_status(&circ, status_buffer, status_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_put_byte(&circ, 42);
        circbuf_put_byte(&circ, 43);

        refbuffer[0] = 42;
        refbuffer[1] = 43;

        refcirc.writer_state.put_ptr = refbuffer + 2;
        refcirc.put_bytes_counter = 2;
        COMPARE_CIRC_WITH_REF_CIRC;


        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        COMPARE_CIRC_WITH_REF_CIRC;

        // Read status, note reader_ptr doesn't change!
        res = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==0xDA);
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;

        res = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==42);
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;

        res = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==43);
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;

        res = circbuf_get_byte(&circ, &b);
        refcirc.ovf = 1;
        assert(res==0);
        assert(circbuf_get_ovf(&circ) == 1);
        assert(b==COMM_BAD_BYTE);
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;

        int32_t len = circbuf_stop_read(&circ, 4);
        refcirc.reader_state.get_ptr = refbuffer + 2;
        refcirc.get_bytes_counter = 2;
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ) == 1);
        assert(len == 0);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        COMPARE_CIRC_WITH_REF_CIRC;
    }

    REPORT_CASE
    {
        DECLARE_CIRC_BUFFERS(10, 1, 1);
        status_buffer[0] = 0xDA;
        refstatus_buffer[0] = 0xDA;
        refstatus_buffer[1] = 0xFF;

        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_status(&circ, status_buffer, status_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_put_byte(&circ, 42);
        circbuf_put_byte(&circ, 43);

        refbuffer[0] = 42;
        refbuffer[1] = 43;

        refcirc.writer_state.put_ptr = refbuffer + 2;
        refcirc.put_bytes_counter = 2;
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;


        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;


        res = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(circbuf_len(&circ)==2);
        assert(b==0xDA);
        COMPARE_CIRC_WITH_REF_CIRC;


        res = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(circbuf_len(&circ)==2);
        assert(b==42);
        COMPARE_CIRC_WITH_REF_CIRC;


        int32_t len = circbuf_stop_read(&circ, 2);
        refcirc.reader_state.get_ptr = refbuffer+1;
        refcirc.get_bytes_counter = 1;
        assert(len == 1);
        assert(circbuf_len(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer+1;
        assert(circbuf_len(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        res = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==0xDA);
        assert(circbuf_len(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        res = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==43);
        assert(circbuf_len(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        len = circbuf_stop_read(&circ, 2);
        refcirc.get_bytes_counter++;
        refcirc.reader_state.get_ptr = refbuffer + 2;
        assert(len == 0);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        circbuf_put_byte(&circ, 44);
        circbuf_put_byte(&circ, 45);
        refbuffer[2] = 44;
        refbuffer[3] = 45;
        refcirc.put_bytes_counter += 2;
        refcirc.writer_state.put_ptr += 2;
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer + 2;
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;

        res = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==0xDA);
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;

        res = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.reader_ptr++;
        refcirc.reader_state.bytes_read++;
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==44);
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;

        res = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.reader_ptr++;
        refcirc.reader_state.bytes_read++;
        assert(res==1);
        assert(circbuf_get_ovf(&circ) == 0);
        assert(b==45);
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;

        len = circbuf_stop_read(&circ, 3);
        refcirc.reader_state.get_ptr=refbuffer+4;
        refcirc.get_bytes_counter = 4;
        assert(len == 0);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;
    }
}

void test_circbuffer_single_block() {
    DECLARE_TEST(test_circbuffer_single_block)

    REPORT_CASE // block mode: block_size==buffer_size
    {
        DECLARE_CIRC_BUFFERS(2, 0, 2);
        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_block_mode(&circ, block_size);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // reserve, write and commit one block, no state change
        uint8_t* block = (uint8_t*)circbuf_reserve_block(&circ);
        assert(block==buffer);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // write data
        block[0] = 1;
        block[1] = 2;
        refbuffer[0] = 1;
        refbuffer[1] = 2;
        COMPARE_CIRC_WITH_REF_CIRC;

        // commit block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer;
        refcirc.put_bytes_counter += 2;
        refcirc.wrn = 1;
        assert(circbuf_len(&circ)==buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        // reserve one more block (overflow)
        block = (uint8_t*)circbuf_reserve_block(&circ);
        refcirc.ovf = 1;
        assert(block==0);
        assert(circbuf_get_ovf(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==2);

        // Start reading
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==2);

        // Read one byte
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.reader_ptr++;
        refcirc.reader_state.bytes_read++;
        assert(opres==1);
        assert(b==1);
        assert(circbuf_len(&circ)==2);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Stop reading
        res = circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr++;
        refcirc.get_bytes_counter++;
        assert(circbuf_len(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // reserve one more block (overflow)
        block = (uint8_t*)circbuf_reserve_block(&circ);
        refcirc.ovf = 1;
        assert(block==0);
        assert(circbuf_get_ovf(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Read one byte (the last one)
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer+1;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==1);
        assert(circ.wrn == 1);
        COMPARE_CIRC_WITH_REF_CIRC;


        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.reader_ptr = refbuffer;
        refcirc.reader_state.bytes_read = 1;
        assert(circ.wrn == 1);
        assert(opres==1);
        assert(b==2);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // stop reading
        res = circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr = refbuffer;
        refcirc.get_bytes_counter++;
        refcirc.wrn = 0;
        assert(circ.wrn == 0);
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;



        // reserve one more block (success)
        block = (uint8_t*)circbuf_reserve_block(&circ);
        assert(block==buffer);
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // reserve, write and commit one block (success)
        block = (uint8_t*)circbuf_reserve_block(&circ);
        assert(block==buffer);
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // write data
        block[0] = 3;
        block[1] = 4;
        refbuffer[0] = 3;
        refbuffer[1] = 4;
        assert(circ.wrn == 0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // commit a block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer;
        refcirc.put_bytes_counter += block_size;
        refcirc.wrn = 1;
        assert(circ.wrn == 1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // read two bytes
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        assert(circ.wrn == 1);
        assert(circbuf_len(&circ)==block_size);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        for (int i=0; i<2; i++) {
            b = 0;
            opres = circbuf_get_byte(&circ, &b);
            refcirc.reader_state.bytes_read++;
            refcirc.reader_state.reader_ptr = refbuffer + ((i+1) % block_size);
            assert(opres==1);
            assert(b==3+i);
            assert(circbuf_len(&circ)==2);
            assert(circbuf_get_ovf(&circ)==0);
            COMPARE_CIRC_WITH_REF_CIRC;
        }
        assert(circ.wrn==1);

        // stop reading
        res = circbuf_stop_read(&circ, 2);
        refcirc.reader_state.get_ptr = refbuffer;
        refcirc.get_bytes_counter += block_size;
        refcirc.wrn = 0;
        assert(circbuf_len(&circ)==0);
        assert(circ.wrn==0);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Start reading from empty buffer
        circbuf_start_read(&circ);
        refcirc.reader_state.reader_ptr = refbuffer;
        refcirc.reader_state.bytes_read = 0;
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circ.wrn==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // read 1 byte, must fail.
        b = 0;
        opres = circbuf_get_byte(&circ, &b);
        refcirc.ovf = 1;
        assert(opres==0);
        assert(b == COMM_BAD_BYTE);
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // clear ovf
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // read more bytes than actually read! Overflow shouldn't be changed!
        res = circbuf_stop_read(&circ, 1);
        assert(circbuf_len(&circ)==0);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;
    }
}

void test_circbuffer_block_mode_work() {
    DECLARE_TEST(test_circbuffer_block_mode_work)

    REPORT_CASE
    {
        DECLARE_CIRC_BUFFERS(8, 0, 4);
        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_block_mode(&circ, block_size);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // reserve a block
        uint8_t* block = (uint8_t*)circbuf_reserve_block(&circ);

        block[0] = 1;
        block[1] = 2;
        block[2] = 3;
        block[3] = 4;

        refbuffer[0] = 1;
        refbuffer[1] = 2;
        refbuffer[2] = 3;
        refbuffer[3] = 4;

        assert(block==buffer);
        COMPARE_CIRC_WITH_REF_CIRC;

        // commit block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr += block_size;
        refcirc.put_bytes_counter += block_size;
        COMPARE_CIRC_WITH_REF_CIRC;


        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);
        assert(block == buffer + block_size);
        COMPARE_CIRC_WITH_REF_CIRC;


        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);
        assert(block == buffer + block_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        // write data
        block[0] = 5;
        block[1] = 6;
        block[2] = 7;
        block[3] = 8;

        refbuffer[4] = 5;
        refbuffer[5] = 6;
        refbuffer[6] = 7;
        refbuffer[7] = 8;
        COMPARE_CIRC_WITH_REF_CIRC;

        // commit block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer;
        refcirc.put_bytes_counter += block_size;
        refcirc.wrn = 1;
        assert(circbuf_len(&circ) == buffer_size);
        assert(circbuf_get_ovf(&circ) == 0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_len(&circ)==8);



        // read 3 bytes (1)
        for (uint8_t i=0; i<4; i++)
        {
            opres = circbuf_get_byte(&circ, &b);
            refcirc.reader_state.bytes_read++;
            refcirc.reader_state.reader_ptr++;
            assert(opres==1);
            assert(b==1+i);
            assert(circbuf_len(&circ)==8);
            assert(circbuf_get_ovf(&circ)==0);
            COMPARE_CIRC_WITH_REF_CIRC;
        }



        // stop reading
        int32_t len = circbuf_stop_read(&circ, 3);
        refcirc.reader_state.get_ptr = refbuffer + 3;
        refcirc.get_bytes_counter += 3;
        refcirc.wrn = 1; // Must read completely to drop it
        assert(len == 5); // 5 bytes remains
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==5);
        COMPARE_CIRC_WITH_REF_CIRC;

        // there are 5 bytes in buffer and just 3 bytes are free, therefor we can't reserve new block, test it!
        block = (uint8_t*)circbuf_reserve_block(&circ);
        refcirc.ovf = 1;
        assert(block==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // read one more byte to free space for new block
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer + 3;
        assert(circbuf_len(&circ)==5);
        COMPARE_CIRC_WITH_REF_CIRC;

        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(opres==1);
        assert(b==4);
        assert(circbuf_len(&circ)==5);
        assert(circbuf_get_ovf(&circ)==1);        // flag is still set, we'll clear it later
        COMPARE_CIRC_WITH_REF_CIRC;

        // stop reading
        len = circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr = refbuffer + 4;
        refcirc.get_bytes_counter++;
        refcirc.wrn = 1;
        assert(len == 4); // 4 bytes remains
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==4);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==4);
        COMPARE_CIRC_WITH_REF_CIRC;


        // reserve block again
        block = (uint8_t*)circbuf_reserve_block(&circ);

        // write
        block[0] = 9;
        block[1] = 10;
        block[2] = 11;
        block[3] = 12;

        refbuffer[0] = 9;
        refbuffer[1] = 10;
        refbuffer[2] = 11;
        refbuffer[3] = 12;

        COMPARE_CIRC_WITH_REF_CIRC;

        // commit block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer + 4;
        refcirc.put_bytes_counter += block_size;
        refcirc.wrn = 1;
        COMPARE_CIRC_WITH_REF_CIRC;

        // prepare for read
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer+4;
        assert(circbuf_len(&circ)==8);
        COMPARE_CIRC_WITH_REF_CIRC;


        // read all the data in the buffer (8 bytes)
        for (uint8_t i=0; i<8; i++)
        {
            opres = circbuf_get_byte(&circ, &b);
            refcirc.reader_state.bytes_read++;
            refcirc.reader_state.reader_ptr++;
            if (refcirc.reader_state.reader_ptr >= refcirc.buffer_end) {
                refcirc.reader_state.reader_ptr = refbuffer;
            }
            assert(opres==1);
            assert(b==5+i);
            assert(circbuf_len(&circ)==8);
            assert(circbuf_get_ovf(&circ)==0);
            COMPARE_CIRC_WITH_REF_CIRC;
        }


        // read one byte from empty buffer
        opres = circbuf_get_byte(&circ, &b);
        refcirc.ovf=1;
        assert(opres==0);
        assert(b == COMM_BAD_BYTE);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==8);
        COMPARE_CIRC_WITH_REF_CIRC;

        len = circbuf_stop_read(&circ, 8);
        refcirc.reader_state.get_ptr = refbuffer + 4;
        refcirc.get_bytes_counter  += 8;
        refcirc.wrn = 0;
        assert(len == 0); // 0 bytes remains
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // Clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
    }


    REPORT_CASE // Special case: 1 byte block size
    {
        DECLARE_CIRC_BUFFERS(2, 0, 1);
        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_block_mode(&circ, block_size);
        refcirc.block_mode = 1;
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // reserve a block
        uint8_t* block = (uint8_t*)circbuf_reserve_block(&circ);
        block[0] = 6;
        refbuffer[0] = 6;

        assert(block==buffer);
        COMPARE_CIRC_WITH_REF_CIRC;


        // commit block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer+1;
        refcirc.put_bytes_counter += block_size;
        refcirc.wrn = 0;
        COMPARE_CIRC_WITH_REF_CIRC;

        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);
        assert(block == (buffer + 1));
        COMPARE_CIRC_WITH_REF_CIRC;

        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);
        assert(block == (buffer + 1));
        COMPARE_CIRC_WITH_REF_CIRC;

        block[0] = 7;
        refbuffer[1] = 7;
        COMPARE_CIRC_WITH_REF_CIRC;

        // commit block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer;
        refcirc.put_bytes_counter += block_size;
        refcirc.wrn = 1;
        assert(circbuf_len(&circ) == buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        assert(circbuf_len(&circ)==2);
        COMPARE_CIRC_WITH_REF_CIRC;


        // read 2 bytes (1)
        for (uint8_t i=0; i<2; i++)
        {
            opres = circbuf_get_byte(&circ, &b);
            refcirc.reader_state.bytes_read++;
            refcirc.reader_state.reader_ptr++;
            if (refcirc.reader_state.reader_ptr >= refcirc.buffer_end) {
                refcirc.reader_state.reader_ptr = refbuffer;
            }
            assert(opres==1);
            assert(b==6+i);
            assert(circbuf_len(&circ)==buffer_size);
            assert(circbuf_get_ovf(&circ)==0);
            COMPARE_CIRC_WITH_REF_CIRC;
        }

        // stop reading
        int32_t len = circbuf_stop_read(&circ, 3);
        refcirc.reader_state.get_ptr = refbuffer;
        refcirc.get_bytes_counter += 2;
        refcirc.wrn = 0;
        assert(len == 0); // 2 bytes remains
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // reserve block again
        block = (uint8_t*)circbuf_reserve_block(&circ);

        // write
        block[0] = 8;
        refbuffer[0] = 8;
        assert(block == buffer);
        COMPARE_CIRC_WITH_REF_CIRC;

        // commit block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer + 1;
        refcirc.put_bytes_counter += block_size;
        refcirc.wrn = 0;
        COMPARE_CIRC_WITH_REF_CIRC;

        // prepare for read
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        assert(circbuf_len(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // read all the data in the buffer (1 bytes)
        for (uint8_t i=0; i<1; i++)
        {
            opres = circbuf_get_byte(&circ, &b);
            refcirc.reader_state.bytes_read++;
            refcirc.reader_state.reader_ptr++;
            if (refcirc.reader_state.reader_ptr >= refcirc.buffer_end) {
                refcirc.reader_state.reader_ptr = refbuffer;
            }
            assert(opres==1);
            assert(b==8+i);
            assert(circbuf_len(&circ)==1);
            assert(circbuf_get_ovf(&circ)==0);
            COMPARE_CIRC_WITH_REF_CIRC;
        }

        // read one byte from empty buffer
        opres = circbuf_get_byte(&circ, &b);
        refcirc.ovf=1;
        assert(opres==0);
        assert(b == COMM_BAD_BYTE);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_get_ovf(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Stop reading
        res = circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr =  refbuffer+1;
        refcirc.get_bytes_counter++;
        refcirc.wrn = 0;
        assert(res == 0); // 0 bytes remains
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;
    }

    // Special case: 1 byte block size and test overflow
    REPORT_CASE
    {
        DECLARE_CIRC_BUFFERS(2, 0, 1);
        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_block_mode(&circ, block_size);
        refcirc.block_mode = 1;
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        uint8_t start_val=9;
        uint8_t* block;

        for (size_t i=0; i<buffer_size; i++) {
            // reserve a block
            block = (uint8_t*)circbuf_reserve_block(&circ);
            block[0] = start_val+i;
            refbuffer[i] = start_val+i;
            assert(block==buffer+i);
            COMPARE_CIRC_WITH_REF_CIRC;

            // commit block
            circbuf_commit_block(&circ);
            refcirc.writer_state.put_ptr++;
            if (refcirc.writer_state.put_ptr >= refcirc.buffer_end) {
                refcirc.writer_state.put_ptr = refbuffer;
            }
            refcirc.put_bytes_counter += block_size;
            refcirc.wrn = i>=buffer_size-1;
            assert(circbuf_get_ovf(&circ)==0);
            COMPARE_CIRC_WITH_REF_CIRC;
        }

        // attempt reserve block again, must fail
        block = (uint8_t*)circbuf_reserve_block(&circ);
        refcirc.ovf = 1;
        assert(block==0);
        assert(circbuf_get_ovf(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;

        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        assert(circbuf_len(&circ)==buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        // read 1 byte
        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr = refbuffer + 1;
        assert(opres==1);
        assert(b==start_val);
        assert(circbuf_len(&circ)==buffer_size);
        assert(circbuf_get_ovf(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // stop reading
        int32_t len = circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr = refbuffer + 1;
        refcirc.get_bytes_counter++;
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==1);
        assert(len == 1); // 1 bytes remains
        COMPARE_CIRC_WITH_REF_CIRC;


        // clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // attempt to write one more byte again:
        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);
        block[0] = start_val+4;
        refbuffer[0] = start_val+4;
        assert(block==buffer);
        COMPARE_CIRC_WITH_REF_CIRC;


        // commit block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer+1;
        refcirc.put_bytes_counter += block_size;
        refcirc.wrn = 1;
        COMPARE_CIRC_WITH_REF_CIRC;
        assert(circbuf_get_ovf(&circ)==0);

        // attempt reserve block again, must fail
        block = (uint8_t*)circbuf_reserve_block(&circ);
        refcirc.ovf = 1;
        assert(block==0);
        assert(circbuf_get_ovf(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;
    }
}

void test_circ_buffer_warning() {
    DECLARE_TEST(test_circ_buffer_warning)
    bool must_warn;

    REPORT_CASE { // uninitialized (default) operation
        DECLARE_CIRC_BUFFERS(3, 0, 1);
        circbuf_init(&circ, buffer, buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        for (size_t i=0; i<buffer_size; i++) {
            circbuf_put_byte(&circ, 0);
            must_warn = ( i>=(buffer_size-1) );
            assert( circbuf_check_warning(&circ, circbuf_len(&circ)) ==  must_warn);
            assert(circ.wrn == must_warn);
        }

        circbuf_put_byte(&circ, 0);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==must_warn);
        assert(circ.wrn == must_warn);
        assert(circbuf_get_ovf(&circ) == 1);
        assert(circ.ovf == 1);
    }

    REPORT_CASE { // Normal operation
        DECLARE_CIRC_BUFFERS(4, 0, 1);
        circbuf_init(&circ, buffer, buffer_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        // init warning l=1, h=2
        circbuf_init_warning(&circ, 1, buffer_size-2);
        refcirc.warn_low_thr = 1;
        refcirc.warn_high_thr = buffer_size - 2;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==0);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // add 1 byte, len = 1
        circbuf_put_byte(&circ, 42);
        refbuffer[0] = 42;
        refcirc.writer_state.put_ptr ++;
        refcirc.put_bytes_counter++;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==0);
        assert(circbuf_len(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;

        // add 1 byte, len = 2
        circbuf_put_byte(&circ, 42);
        refbuffer[1] = 42;
        refcirc.writer_state.put_ptr ++;
        refcirc.put_bytes_counter++;
        refcirc.wrn = 1;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        assert(circbuf_len(&circ)==2);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // add 1 byte, len = 3
        circbuf_put_byte(&circ, 42);
        refbuffer[2] = 42;
        refcirc.writer_state.put_ptr ++;
        refcirc.put_bytes_counter++;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        assert(circbuf_len(&circ)==3);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // add 1 byte, len = 4
        circbuf_put_byte(&circ, 42);
        refbuffer[3] = 42;
        refcirc.writer_state.put_ptr=refbuffer;
        refcirc.put_bytes_counter++;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        assert(circbuf_len(&circ)==4);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // Add 1 byte, but buffer is full, ovf should set
        circbuf_put_byte(&circ, 42);
        refcirc.ovf = 1;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        assert(circbuf_len(&circ)==4);
        assert(circbuf_get_ovf(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;

        // Clear ovf, warn is still active
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        assert(circbuf_len(&circ)==4);
        assert(circbuf_get_ovf(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // Read 1 byte, len 3
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        COMPARE_CIRC_WITH_REF_CIRC;

        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(opres==1);
        assert(b==42);
        assert(circbuf_len(&circ)==4);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr = refbuffer + 1;
        refcirc.get_bytes_counter++;
        refcirc.wrn = 1;
        assert(circbuf_len(&circ)==3);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Read 1 byte, len 2
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer + 1;
        COMPARE_CIRC_WITH_REF_CIRC;


        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(opres==1);
        assert(b==42);
        assert(circbuf_len(&circ)==3);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr = refbuffer + 2;
        refcirc.get_bytes_counter  ++;
        refcirc.wrn = 1;
        assert(circbuf_len(&circ)==2);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Read 1 byte, len 1, warn->0 as low threshold is reached
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer + 2;
        COMPARE_CIRC_WITH_REF_CIRC;


        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr = refbuffer + 3;
        assert(opres==1);
        assert(b==42);
        assert(circbuf_len(&circ)==2);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr = refbuffer + 3;
        refcirc.get_bytes_counter ++;
        refcirc.wrn = 0;
        assert(circbuf_len(&circ)==1);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Read 1 byte, len 0
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer + 3;
        COMPARE_CIRC_WITH_REF_CIRC;

        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr = refbuffer;
        assert(opres==1);
        assert(b==42);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr = refbuffer;
        refcirc.get_bytes_counter++;
        assert(circbuf_len(&circ)==0);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==0);
        COMPARE_CIRC_WITH_REF_CIRC;
    }

    REPORT_CASE { // Block mode operation
        DECLARE_CIRC_BUFFERS(4, 0, 2);
        circbuf_init(&circ, buffer, buffer_size);
        assert(circbuf_len(&circ)==0);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==0);
        assert(circ.wrn == 0);
        circbuf_init_block_mode(&circ, block_size);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // init warning l=1, h=2
        circbuf_init_warning(&circ, 1, buffer_size-2);
        refcirc.warn_low_thr = 1;
        refcirc.warn_high_thr = 2;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==0);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // add 1 block, len = 2
        volatile void* block = circbuf_reserve_block(&circ);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==0);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
        assert(block==buffer);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer + block_size;
        refcirc.put_bytes_counter += block_size;
        refcirc.wrn = 1;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==block_size);
        COMPARE_CIRC_WITH_REF_CIRC;


        // add 1 more block, len = 4
        block = circbuf_reserve_block(&circ);
        assert(block == buffer+block_size);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==block_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer;
        refcirc.put_bytes_counter += block_size;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==block_size*2);
        COMPARE_CIRC_WITH_REF_CIRC;


        // add 1 more block, failed due overflow, len = 4
        block = circbuf_reserve_block(&circ);
        refcirc.ovf = 1;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==block_size*2);
        assert(block == 0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // clear overflow, len = 4
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==block_size*2);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Read 1 byte, len 3
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        COMPARE_CIRC_WITH_REF_CIRC;

        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(opres==1);
        assert(circbuf_len(&circ)==4);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr = refbuffer + 1;
        refcirc.get_bytes_counter++;
        assert(circbuf_len(&circ)==3);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Read 1 byte, len 2
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer + 1;
        COMPARE_CIRC_WITH_REF_CIRC;


        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(opres==1);
        assert(circbuf_len(&circ)==3);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr = refbuffer + 2;
        refcirc.get_bytes_counter++;
        assert(circbuf_len(&circ)==2);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Read 1 byte, len 1, warn->0 as low threshold is reached
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer + 2;
        COMPARE_CIRC_WITH_REF_CIRC;


        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr = refbuffer + 3;
        assert(opres==1);
        assert(circbuf_len(&circ)==2);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr = refbuffer + 3;
        refcirc.get_bytes_counter++;
        refcirc.wrn = 0;
        assert(circbuf_len(&circ)==1);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Read 1 byte, len 0, warn=0
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer+3;
        COMPARE_CIRC_WITH_REF_CIRC;

        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr = refbuffer;
        assert(opres==1);
        assert(circbuf_len(&circ)==1);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        circbuf_stop_read(&circ, 1);
        refcirc.reader_state.get_ptr = refbuffer;
        refcirc.get_bytes_counter++;
        assert(circbuf_len(&circ)==0);
        assert(circbuf_check_warning(&circ, circbuf_len(&circ))==0);
        COMPARE_CIRC_WITH_REF_CIRC;
    }
}


void test_circbuffer_block_mode_work_with_status() {
    DECLARE_TEST(test_circbuffer_block_mode_work_with_status)

    REPORT_CASE
    {
        DECLARE_CIRC_BUFFERS(8, 1, 4);
        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_block_mode(&circ, block_size);
        circbuf_init_status(&circ, status_buffer, status_size);
        status_buffer[0] = 0xDA;
        refstatus_buffer[0] = status_buffer[0];
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // reserve a block and write something
        uint8_t* block = (uint8_t*)circbuf_reserve_block(&circ);
        block[0] = 1;
        block[1] = 2;
        block[2] = 3;
        block[3] = 4;

        refbuffer[0] = 1;
        refbuffer[1] = 2;
        refbuffer[2] = 3;
        refbuffer[3] = 4;

        assert(block==buffer);
        COMPARE_CIRC_WITH_REF_CIRC;


        // commit block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer+block_size;
        refcirc.put_bytes_counter += block_size;
        COMPARE_CIRC_WITH_REF_CIRC;

        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);
        assert(block == buffer + block_size);
        COMPARE_CIRC_WITH_REF_CIRC;

        // reserve a block
        block = (uint8_t*)circbuf_reserve_block(&circ);
        assert(block == buffer + block_size);

        block[0] = 5;
        block[1] = 6;
        block[2] = 7;
        block[3] = 8;

        refbuffer[4] = 5;
        refbuffer[5] = 6;
        refbuffer[6] = 7;
        refbuffer[7] = 8;
        COMPARE_CIRC_WITH_REF_CIRC;

        // commit block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer;
        refcirc.put_bytes_counter += block_size;
        refcirc.wrn = 1;
        COMPARE_CIRC_WITH_REF_CIRC;

        // start reading from circular buffer
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer;
        assert(circbuf_len(&circ)==8);
        COMPARE_CIRC_WITH_REF_CIRC;

        res = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        assert(res != 0);
        assert(b==0xDA);
        assert(circbuf_len(&circ)==8);
        COMPARE_CIRC_WITH_REF_CIRC;

        // read 4 bytes (1)
        for (uint8_t i=0; i<4; i++)
        {
            opres = circbuf_get_byte(&circ, &b);
            refcirc.reader_state.bytes_read++;
            refcirc.reader_state.reader_ptr++;
            assert(opres==1);
            assert(b==1+i);
            assert(circbuf_len(&circ)==8);
            assert(circbuf_get_ovf(&circ)==0);
            COMPARE_CIRC_WITH_REF_CIRC;
        }

        // stop reading. Note: we read 5 bytes above (icluding status byte), but stop read with 4 bytes, it means that
        // read pointer will advance by 3 bytes only.
        int32_t len = circbuf_stop_read(&circ, 4);
        refcirc.reader_state.get_ptr = refbuffer + 3;
        refcirc.get_bytes_counter += 3;
        assert(len == 5); // 5 bytes remains
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==5);
        COMPARE_CIRC_WITH_REF_CIRC;


        // there are 5 bytes in buffer, and 3 free, we can't reserve new block, test it
        block = (uint8_t*)circbuf_reserve_block(&circ);
        refcirc.ovf = 1;
        assert(block==0);
        COMPARE_CIRC_WITH_REF_CIRC;

        // read one more byte to free space for new block
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer + 3;
        assert(circbuf_len(&circ)==5);
        COMPARE_CIRC_WITH_REF_CIRC;

        opres = circbuf_get_byte(&circ, &b); // status byte
        refcirc.reader_state.bytes_read++;
        assert(b==0xDA);
        assert(opres==1);
        COMPARE_CIRC_WITH_REF_CIRC;

        opres = circbuf_get_byte(&circ, &b); // a byte from buffer
        refcirc.reader_state.bytes_read++;
        refcirc.reader_state.reader_ptr++;
        assert(opres==1);
        assert(b==4);
        assert(circbuf_len(&circ)==5);
        assert(circbuf_get_ovf(&circ)==1);        // flag is still set, we'll clear it later
        COMPARE_CIRC_WITH_REF_CIRC;

        // stop reading, 1 status byte and 1 byte from buffer
        len = circbuf_stop_read(&circ, 2);
        refcirc.reader_state.get_ptr = refbuffer + 4;
        refcirc.get_bytes_counter += 1;
        assert(len == 4); // 4 bytes remains
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==4);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==4);
        COMPARE_CIRC_WITH_REF_CIRC;

        // reserve block again
        block = (uint8_t*)circbuf_reserve_block(&circ);

        // write
        block[0] = 9;
        block[1] = 10;
        block[2] = 11;
        block[3] = 12;

        refbuffer[0] = 9;
        refbuffer[1] = 10;
        refbuffer[2] = 11;
        refbuffer[3] = 12;
        assert(block == buffer);
        COMPARE_CIRC_WITH_REF_CIRC;


        // commit block
        circbuf_commit_block(&circ);
        refcirc.writer_state.put_ptr = refbuffer + block_size;
        refcirc.put_bytes_counter += block_size;
        refcirc.wrn = 1;
        COMPARE_CIRC_WITH_REF_CIRC;

        // prepare for read
        circbuf_start_read(&circ);
        refcirc.reader_state.bytes_read = 0;
        refcirc.reader_state.reader_ptr = refbuffer + block_size;
        assert(circbuf_len(&circ)==8);
        COMPARE_CIRC_WITH_REF_CIRC;


        opres = circbuf_get_byte(&circ, &b);
        refcirc.reader_state.bytes_read++;
        assert(b==0xDA);
        COMPARE_CIRC_WITH_REF_CIRC;


        // read all the data in the buffer (8 bytes)
        for (uint8_t i=0; i<8; i++)
        {
            opres = circbuf_get_byte(&circ, &b);
            refcirc.reader_state.bytes_read++;
            refcirc.reader_state.reader_ptr++;
            if (refcirc.reader_state.reader_ptr >= refcirc.buffer_end) {
                refcirc.reader_state.reader_ptr = refbuffer;
            }
            assert(opres==1);
            assert(b==5+i);
            assert(circbuf_len(&circ)==8);
            assert(circbuf_get_ovf(&circ)==0);
            COMPARE_CIRC_WITH_REF_CIRC;
        }

        // read one byte from empty buffer
        opres = circbuf_get_byte(&circ, &b);
        refcirc.ovf=1;
        assert(opres==0);
        assert(b == COMM_BAD_BYTE);
        assert(circbuf_len(&circ)==8);
        assert(circbuf_get_ovf(&circ)==1);
        COMPARE_CIRC_WITH_REF_CIRC;


        // stop reading
        res = circbuf_stop_read(&circ, 9);
        refcirc.reader_state.get_ptr = refbuffer + block_size;
        refcirc.get_bytes_counter  += 8;
        refcirc.wrn = 0;
        assert(res == 0); // 0 bytes remains
        assert(circbuf_get_ovf(&circ)==1);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;


        // Clear overflow
        circbuf_clear_ovf(&circ);
        refcirc.ovf = 0;
        assert(circbuf_get_ovf(&circ)==0);
        assert(circbuf_len(&circ)==0);
        COMPARE_CIRC_WITH_REF_CIRC;
    }
}


void test_circbuffer_asserts() {
    DECLARE_TEST(test_circbuffer_asserts)

    // byte mode: circbuf_reserve_block() must assert
    REPORT_CASE
    {
        CircBuffer circ;
        CircBuffer refcirc;
        memset(&circ, 0, sizeof(circ));
        memset(&refcirc, 0, sizeof(refcirc));
        const uint16_t buffer_size = 8;
        const uint16_t block_size = 4;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);
        uint8_t opres,b,res;

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.warn_high_thr = refcirc.buffer_size;
        assert(circbuf_len(&circ)==0);
        assert(g_assert_param_count==0);
        TEST_ASSERTION_BEGIN
        block = (uint8_t*)circbuf_reserve_block(&circ);
        TEST_ASSERTION_END
        assert(g_assert_param_count>0); // undefined behaviour, assertion must be triggered because buffer is in byte mode.
    }

    REPORT_CASE
    {
        CircBuffer circ;
        memset(&circ, 0, sizeof(circ));
        const uint16_t buffer_size = 8;
        const uint16_t block_size = 4;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);
        uint8_t opres,b,res;
        assert(circbuf_len(&circ)==0);

        assert(g_assert_param_count==0);
        TEST_ASSERTION_BEGIN
        circbuf_commit_block(&circ);
        TEST_ASSERTION_END
        assert(g_assert_param_count>0); // undefined behaviour, assertion must be triggered (one for not being in block mode, another for unalocated block)
    }

    REPORT_CASE // block mode: attempt to init with buffer not multiple by block size
    {
        CircBuffer circ;
        memset(&circ, 0, sizeof(circ));
        const uint16_t buffer_size = 9;
        const uint16_t block_size = 4;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);
        uint8_t opres,b,res;
        assert(circbuf_len(&circ)==0);

        assert(g_assert_param_count==0);
        TEST_ASSERTION_BEGIN
        circbuf_init_block_mode(&circ, block_size);
        TEST_ASSERTION_END
        assert(g_assert_param_count>0); // undefined behaviour, assertion must be triggered)
    }

    REPORT_CASE // block mode: attempt to init with block size == 0
    {
        CircBuffer circ;
        CircBuffer refcirc;
        memset(&circ, 0, sizeof(circ));
        memset(&refcirc, 0, sizeof(refcirc));
        const uint16_t buffer_size = 9;
        const uint16_t block_size = 0;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);
        uint8_t opres,b,res;

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.warn_high_thr = refcirc.buffer_size;
        assert(circbuf_len(&circ)==0);

        TEST_ASSERTION_BEGIN;
        circbuf_init_block_mode(&circ, block_size);
        TEST_ASSERTION_END;

        assert(g_assert_param_count!=0); // undefined behaviour, assertion must be triggered)
    }

    REPORT_CASE // block mode: attempt to init with block size > buffer_size
    {
        CircBuffer circ;
        CircBuffer refcirc;
        memset(&circ, 0, sizeof(circ));
        memset(&refcirc, 0, sizeof(refcirc));
        const uint16_t buffer_size = 8;
        const uint16_t block_size = 16;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);
        uint8_t opres,b,res;

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.warn_high_thr = refcirc.buffer_size;

        assert(circbuf_len(&circ)==0);

        TEST_ASSERTION_BEGIN;
        circbuf_init_block_mode(&circ, block_size);
        TEST_ASSERTION_END;
        assert(g_assert_param_count>0); // undefined behaviour

    }

    REPORT_CASE // block mode: attempt to init block mode while in block mode
    {
        CircBuffer circ;
        CircBuffer refcirc;
        memset(&circ, 0, sizeof(circ));
        memset(&refcirc, 0, sizeof(refcirc));
        const uint16_t buffer_size = 16;
        const uint16_t block_size = 16;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        uint8_t refbuffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);
        uint8_t opres,b,res;

        refcirc.buffer=buffer;
        refcirc.buffer_size=buffer_size;
        refcirc.free_size=buffer_size-1;
        refcirc.block_size=1;
        refcirc.warn_high_thr = refcirc.buffer_size;
        assert(circbuf_len(&circ)==0);

        circbuf_init_block_mode(&circ, block_size);
        refcirc.block_mode=1;
        refcirc.block_size=block_size;
        assert(g_assert_param_count==0); // must be ok
        TEST_ASSERTION_BEGIN;
        circbuf_init_block_mode(&circ, block_size);
        TEST_ASSERTION_END;
        assert(g_assert_param_count!=0); // not ok - already in block mode
    }

    REPORT_CASE // block mode attempt to call circbuf_put_byte()
    {
        CircBuffer circ;
        memset(&circ, 0, sizeof(circ));
        const uint16_t buffer_size = 16;
        const uint16_t block_size = 16;
        uint8_t* block;
        uint8_t buffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);
        uint8_t opres,b,res;

        assert(circbuf_len(&circ)==0);

        circbuf_init_block_mode(&circ, block_size);
        assert(g_assert_param_count==0); // must be ok
        b = 0;
        TEST_ASSERTION_BEGIN;
        circbuf_put_byte(&circ, b);
        TEST_ASSERTION_END;
        assert(g_assert_param_count>0); // not ok - in block mode
    }
}

constexpr uint64_t test_op_count = 1000000/*0000*/;

void reader_single_byte_thread_func(CircBuffer* circ) {
    uint64_t byte_count = 0;
    uint8_t b = 0;
    uint8_t tb = 0;
    do {
        circbuf_start_read(circ);
        b = 0;
        uint8_t res = circbuf_get_byte(circ, &b);
        circbuf_stop_read(circ, res);

        if (res) {
            assert(tb==b);
            tb++;
            byte_count++;
        }
    } while (byte_count < test_op_count);
}

void writer_single_byte_thread_func(CircBuffer* circ) {
    uint64_t op_count = 0;
    uint8_t b = 0;
    do {
        uint8_t res  = circbuf_put_byte(circ, b);
        op_count += res;
        b+=res;
    } while (op_count < test_op_count);
}

constexpr int32_t MT_TEST_BLOCK_SIZE = 4;

void reader_block_thread_func(CircBuffer* circ) {
    uint64_t op_count = 0;
    uint8_t b = 0;
    uint8_t tb = 0;
    do {
        circbuf_start_read(circ);
        b = 0;
        uint8_t res = circbuf_get_byte(circ, &b);

        if (res) {
            // note: all block should be available
            assert(tb==b);

            for (int i = 0; i<MT_TEST_BLOCK_SIZE-1; i++) {
                b = 0;
                res = circbuf_get_byte(circ, &b);
                assert(res == 1);
                assert(tb == b);
            }

            tb++;
            op_count++;

            circbuf_stop_read(circ, MT_TEST_BLOCK_SIZE);
        }
    } while (op_count < test_op_count);
}

void writer_block_thread_func(CircBuffer* circ) {
    uint64_t op_count = 0;
    uint8_t b = 0;
    do {
        uint8_t* block = (uint8_t*)circbuf_reserve_block(circ);
        if (block) {
            memset(block, b, MT_TEST_BLOCK_SIZE);
            circbuf_commit_block(circ);

            op_count += 1;
            b+=1;
        }
    } while (op_count < test_op_count);
}

void test_circ_buffer_multithreaded() {
    DECLARE_TEST(test_circ_buffer_multithreaded)

    REPORT_CASE // Multi thread test in byte mode
    {
        CircBuffer circ;
        const int32_t buffer_size = 16;
        uint8_t buffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);

        std::thread reader_thread(reader_single_byte_thread_func, &circ);
        std::thread writer_thread(writer_single_byte_thread_func, &circ);

        reader_thread.join();
        writer_thread.join();
        assert(circbuf_len(&circ)==0);
    }

    REPORT_CASE // Multi thread test in block mode
    {
        CircBuffer circ;
        constexpr int32_t buffer_size = 16;
        constexpr int32_t block_size = MT_TEST_BLOCK_SIZE;
        uint8_t buffer[buffer_size] = {0};
        circbuf_init(&circ, buffer, buffer_size);
        circbuf_init_block_mode(&circ, block_size);

        std::thread reader_thread(reader_block_thread_func, &circ);
        std::thread writer_thread(writer_block_thread_func, &circ);

        reader_thread.join();
        writer_thread.join();
        assert(circbuf_len(&circ)==0);
    }
}