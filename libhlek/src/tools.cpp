/**
 *   Copyright 2021 Oleh Sharuda <oleh.sharuda@gmail.com>
 *
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

/*!  \file
 *   \brief Tools and utilities implementation
 *   \author Oleh Sharuda
 */

#include <thread>
#include <execinfo.h>
#include "tools.hpp"
#include "i2c_proto.h"
#include <cxxabi.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include <signal.h>

#ifdef __PYTHON_MODULE__
#include <Python.h>
#endif

void tools::debug_print(const char *format, ... )
{
#ifdef __PYTHON_MODULE__
    char buffer[1000];
#endif
    std::string s;

    va_list argptr;
    va_start(argptr, format);
#ifdef __PYTHON_MODULE__
    vsprintf(buffer, format, argptr);
    va_end(argptr);
    s = buffer;
    s+="\n";
    PySys_WriteStdout(s.c_str());
#else
    s = format;
    s+="\n";
    vprintf(s.c_str(), argptr);
    va_end(argptr);
#endif
}

std::string get_backtrace() {
    static const int STACK_TRACE_COUNT = 100;
    void* stack_trace[STACK_TRACE_COUNT];
    int nentries = backtrace(stack_trace, STACK_TRACE_COUNT);
    char** strings = backtrace_symbols(stack_trace, nentries);

    assert(strings!=nullptr);

    std::string res;
    for (int i=0; i<nentries; i++) {
        char* mangled = strings[i];
        char* mangled_brace = nullptr;
        char* mangled_offset = nullptr;
        char* demangled = nullptr;

        int pos;
        char* pc;

        for (pc=mangled; *pc!='\0' && *pc!='('; pc++) {};
        if (*pc=='(') {
            mangled_brace = pc;

            for (; *pc!='\0' && *pc!='+'; pc++) {};
            if (*pc=='+') {
                int status = 0;
                mangled_offset = pc;
                *mangled_brace =  '\0';                
                *mangled_offset = '\0';


                demangled = abi::__cxa_demangle(mangled_brace+1, nullptr, 0, &status);
                if (demangled==nullptr) {
                    *mangled_brace = '(';
                    *mangled_offset = '+';
                }
            }
        }

        if (demangled!=nullptr) {
            res += mangled;
            res += '(';
            res += demangled;
            res += '+';
            res += mangled_offset+1;
            free(demangled);
        } else {
            res += mangled;
        }
        res+="\n";
    }

    free(strings);
    return res;
}

#ifndef NDEBUG

static tools::GlobalMutexVerifier g_mtx_verif;

tools::GlobalMutexVerifier::GlobalMutexVerifier(){
}

tools::GlobalMutexVerifier::~GlobalMutexVerifier(){
}

void tools::GlobalMutexVerifier::new_mutex(std::mutex::native_handle_type nh, const std::string& where){
}

void tools::GlobalMutexVerifier::delete_mutex(std::mutex::native_handle_type nh, const std::string& where){
}

void tools::GlobalMutexVerifier::lock_mutex(std::mutex::native_handle_type nh, const std::string& where){
}


void tools::GlobalMutexVerifier::unlock_mutex(std::mutex::native_handle_type nh, const std::string& where){
}

tools::TLSMutexVerifier::TLSMutexVerifier(){
    
}

tools::TLSMutexVerifier::~TLSMutexVerifier(){
    
}

void tools::TLSMutexVerifier::lock_mutex(std::mutex::native_handle_type nh, const std::string& where){
    auto ins_res = locked_mutexes.insert(nh);
    assert(ins_res.first != locked_mutexes.end());
    assert(ins_res.second);
    MutexEntry me;
    me.nh = nh;
    me.back_trace = where;
    lock_trace.push(std::move(me));
}

void tools::TLSMutexVerifier::unlock_mutex(std::mutex::native_handle_type nh, const std::string& where){
    size_t n_removed = locked_mutexes.erase(nh);
    assert(n_removed==1); // must be already locked
    assert(!lock_trace.empty()); // check lock trace is not empty (obviously)
    assert(lock_trace.top().nh==nh); // check for correct order of unlock calls
    lock_trace.pop();
}

bool tools::TLSMutexVerifier::is_locked(std::mutex::native_handle_type nh){
    return locked_mutexes.find(nh)!=locked_mutexes.end();
}

tools::safe_mutex::safe_mutex() : std::mutex() {
    std::string bt = get_backtrace();
    hndl = super::native_handle();
    g_mtx_verif.new_mutex(hndl, bt);
}

tools::safe_mutex::~safe_mutex() {
    std::string bt = get_backtrace();
    g_mtx_verif.delete_mutex(hndl, bt);
}


void tools::safe_mutex::lock() {
    std::string bt = get_backtrace();
    super::lock();
    tls_mtx_verif.lock_mutex(hndl,bt);
    g_mtx_verif.lock_mutex(hndl,bt);
}

void tools::safe_mutex::unlock() {
    std::string bt = get_backtrace();
    g_mtx_verif.unlock_mutex(hndl, bt);
    tls_mtx_verif.unlock_mutex(hndl, bt);
    super::unlock();
}

// This method is not accessible in release build, so it should be called with conditional compilation check
void tools::safe_mutex::check_locked() {
    assert(tls_mtx_verif.is_locked(hndl));
}

#endif

uint8_t tools::calc_contol_sum(uint8_t* buffer, size_t length, size_t exclude_byte) {
    uint8_t crc = COMM_CRC_INIT_VALUE;
    for (size_t i = 0; i<length; i++) {

        if (i==exclude_byte) {
            // skip this byte, it is CRC
        } else {
            crc ^= buffer[i];
        }
    }
    return crc;
}

int tools::stm32_timer_params(uint32_t freq, double delay_s, uint16_t& prescaller, uint16_t& period, double& eff_s) {
    static const char* const func_name = "EKitVirtualDevice::get_timer_params";
    double us_delay = delay_s * 1.0e6;

    if (us_delay > (double)UINT32_MAX) {
        return 1;
    }

    uint32_t us = (uint32_t)us_delay;
    uint64_t k = (uint64_t)(us) * (uint64_t)freq / 1000000;
    uint32_t psc = (uint32_t)(k >> 16);

    if (psc > UINT16_MAX) {
        return 1;
    }

    prescaller = psc;
    if (psc > 0) {
        period = (uint32_t)(k / (psc+1));
    } else {
        period = (uint32_t)k;
    }

    if (period) period--;

    eff_s =  ((double)(period+1) * double(psc+1))/(double)freq;

    return 0;
}

bool tools::is_little_endian() {
    uint32_t test = 1;
    uint8_t* ptest = (uint8_t*)&test;
    return *ptest==1;
}


constexpr size_t max_signal_number = _NSIG;
void tools::install_signal_handler(    int signum,
                                std::vector<struct sigaction>& prev_sig_actions,
                                std::vector<struct sigaction>& new_sig_actions,
                                __sighandler_t handler) {
    assert(signum < max_signal_number);
    assert(signum != SIGKILL); // May not be intercepted
    assert(signum != SIGSTOP); // May not be intercepted


    size_t size = prev_sig_actions.size();
    if (size < max_signal_number) {
        prev_sig_actions.resize(max_signal_number);
        memset(prev_sig_actions.data() + size, 0, max_signal_number - size);
    }

    size = new_sig_actions.size();
    if (size < max_signal_number) {
        new_sig_actions.resize(max_signal_number);
        memset(new_sig_actions.data() + size, 0, max_signal_number - size);
    }

    new_sig_actions[signum].sa_handler = handler;
    new_sig_actions[signum].sa_flags = SA_NODEFER | SA_NOMASK;
    sigaction( signum, &new_sig_actions[signum], &prev_sig_actions[signum]);
}

void tools::call_previous_signal_handler(int signum,
                                         std::vector<struct sigaction>& prev_sig_actions,
                                         std::vector<struct sigaction>& new_sig_actions) {
    assert(signum < max_signal_number);
    assert(prev_sig_actions.size() >= max_signal_number);

    // restore old sigaction: new will become an old signal handler, prev will become a new signal handler
    std::vector<struct sigaction> tmp;
    install_signal_handler(signum, tmp, prev_sig_actions, prev_sig_actions[signum].sa_handler);

    if ( (prev_sig_actions[signum].sa_handler == SIG_IGN) ||
         (prev_sig_actions[signum].sa_handler == SIG_DFL) ||
         (prev_sig_actions[signum].sa_handler == SIG_ERR)) {
        raise(signum);
    } else {
        prev_sig_actions[signum].sa_handler(signum);
    }

    // swap again
    install_signal_handler(signum, prev_sig_actions, new_sig_actions, new_sig_actions[signum].sa_handler);
}

static std::string get_pid_file_name() {
    // Get absolute executable file path and form pid file name
    constexpr const char* prefix = ".pid";
    char buffer[PATH_MAX+1+strlen(prefix)];
    char* pid_file_name = realpath("/proc/self/exe", buffer);
    strcat(pid_file_name, prefix);

    return buffer;
}

void tools::delete_pid_file() {
    std::string pid_file_name = get_pid_file_name();
    unlink(pid_file_name.c_str());
}

std::vector<struct sigaction> auto_delete_pid_file_new_sig_actions;
std::vector<struct sigaction> auto_delete_pid_file_prev_sig_actions;
static void autodelete_pid_file(int signum) {
    tools::delete_pid_file();
    tools::call_previous_signal_handler(  signum,
                                        auto_delete_pid_file_prev_sig_actions,
                                        auto_delete_pid_file_new_sig_actions);
}

bool tools::make_pid_file() {
    bool result = false;
    int fd;

    // Get pid and it's string representation
    pid_t pid = gettid(); // Current pid
    std::string str_pid = std::to_string(pid);

    // Get absolute executable file path and form pid file name
    std::string pid_file_name = get_pid_file_name();

    // Open and lock pid file
    fd = open(pid_file_name.c_str(),
              O_CREAT | O_RDWR | O_EXCL,
              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) goto done;
    if (flock(fd, LOCK_EX | LOCK_NB) < 0) goto done;

    // set handlers
    install_signal_handler(SIGSEGV,
                           auto_delete_pid_file_prev_sig_actions,
                           auto_delete_pid_file_new_sig_actions,
                           autodelete_pid_file);

    install_signal_handler(SIGABRT,
                           auto_delete_pid_file_prev_sig_actions,
                           auto_delete_pid_file_new_sig_actions,
                           autodelete_pid_file);


    install_signal_handler(SIGINT,
                           auto_delete_pid_file_prev_sig_actions,
                           auto_delete_pid_file_new_sig_actions,
                           autodelete_pid_file);


    install_signal_handler(SIGTERM,
                           auto_delete_pid_file_prev_sig_actions,
                           auto_delete_pid_file_new_sig_actions,
                           autodelete_pid_file);



    // Write file and exit. Note, fd remains opened until process is terminated.
    do {
        result = write(fd, str_pid.c_str(), str_pid.length()) > 0;
    } while (!result && errno==EINTR);

done:
    return result;
}

uint8_t tools::reverse_bits(uint8_t b) {
    b = (b >> 4) | (b << 4);
    b = ((b & 0b11001100) >> 2) | ((b & 0b00110011) << 2);
    return ((b & 0b10101010) >> 1) | ((b & 0b01010101) << 1);
}

double tools::normalize_value(double value, double min_value, double max_value) {
    assert(min_value < max_value);
    assert(min_value <= value);
    assert(value <= max_value);
    value = (value - min_value) / (max_value - min_value);
    assert(value>=0.0L && value<=1.0L);
    return value;
}

double tools::denormalize_value(double value, double min_value, double max_value) {
    assert(min_value < max_value);
    assert(min_value <= value);
    assert(value <= max_value);
    assert(value>=0.0L && value<=1.0L);
    value = min_value + (max_value - min_value)*value;
    return value;
}



