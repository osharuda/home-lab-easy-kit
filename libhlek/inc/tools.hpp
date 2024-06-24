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
 *   \brief Tools and utilities header
 *   \author Oleh Sharuda
 */

#pragma once
#include <mutex>
#include <chrono>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <functional>
#include <vector>
#include <set>
#include <cstdarg>
#include <regex>
#include <thread>

/// \defgroup group_tools Tools
/// \brief Miscellaneous and multipurpose tools
/// @{
/// \page page_tools
/// \tableofcontents
///
/// \section sect_tools_01 Tools - a set of miscellaneous and useful classes, functions, defines, etc.
///

/// \brief Returns current stack backtrace as a string.
/// \return String with current stack backtrace.
/// \details This function uses backtrace() function and abi::__cxa_demangle (GCC specific). This function should be used
///          in debug versions only.
std::string get_backtrace();

/// \def THROW_IF_EXPIRED
/// \brief This macro is used for convinience to check timeout and throw exception if expired.
#define THROW_IF_EXPIRED(sw)                                                  \
if ((sw).expired()) {                                                         \
        throw EKitException(func_name, EKIT_TIMEOUT, "Timeout is expired.");  \
    }


/// \enum UNITS
/// \brief Defines simple conversion units
enum UNITS : size_t {
    KB = 1024,
    MB = 1024 * KB
};

/// \brief namespace for tools
namespace tools {

#ifndef NDEBUG

    /// \class GlobalMutexVerifier
    /// \brief This class is used to track possible deadlock conditions. It's not implemented, just dummy methods are added.
    ///        Must be implemented in future because it is very useful thing.
	class GlobalMutexVerifier final {
	public:		
		GlobalMutexVerifier();
		~GlobalMutexVerifier();		

		/// \brief Call to register and track new mutex.
		/// \param nh - native object handle for the mutex to be tracked
		/// \param where - backtrace of the code created this mutex
		void new_mutex(std::mutex::native_handle_type nh, const std::string& where);

        /// \brief Call to deregister and to stop tracking mutex.
        /// \param nh - native object handle for the mutex to be tracked
        /// \param where - backtrace of the code created this mutex
		void delete_mutex(std::mutex::native_handle_type nh, const std::string& where);

        /// \brief Track mutex ownership.
        /// \param nh - native object handle for the mutex to be tracked
        /// \param where - backtrace of the code created this mutex
		void lock_mutex(std::mutex::native_handle_type nh, const std::string& where);

        /// \brief Track mutex release of the ownership.
        /// \param nh - native object handle for the mutex to be tracked
        /// \param where - backtrace of the code created this mutex
		void unlock_mutex(std::mutex::native_handle_type nh, const std::string& where);		
	};


	/// \struct MutexEntry
	/// \brief Mutex entry structure for @ref GlobalMutexVerifier and @ref TLSMutexVerifier classes.
	struct MutexEntry{
		std::string back_trace;             ///< Stack backtrace
		std::mutex::native_handle_type nh;  ///< Native object handle for the mutex to be tracked
	};


	/// \class TLSMutexVerifier
	/// \brief This class is used to check if some mutex is actually owned by the thread or no.
	/// \note This class must have thread local storage specification.
	class TLSMutexVerifier final {
		std::stack<MutexEntry> lock_trace;                          ///< storage for the stack back trace.
		std::set<std::mutex::native_handle_type> locked_mutexes;    ///< Set of the native object handles currently owned.

	public:
	    /// \brief Constructor
		TLSMutexVerifier();

	    /// \brief Destructor
		~TLSMutexVerifier();

		/// \brief Register mutex ownership
        /// \param nh - native object handle for the mutex to be tracked
        /// \param where - backtrace of the code created this mutex
		void lock_mutex(std::mutex::native_handle_type nh, const std::string& where);

        /// \brief Register mutex release of the ownership
        /// \param nh - native object handle for the mutex to be tracked
        /// \param where - backtrace of the code created this mutex
		void unlock_mutex(std::mutex::native_handle_type nh, const std::string& where);

		/// \brief Checks if the mutex is actually locked
        /// \param nh - native object handle for the mutex to be tracked
		/// \return true if mutex is owned, otherwise false.
		bool is_locked(std::mutex::native_handle_type nh);
	};

	/// \brief Thread local variable to track which mutexes are currently owned by the thread
	static thread_local TLSMutexVerifier tls_mtx_verif;

	/// \brief Global variable to track deadlock conditions
	// extern GlobalMutexVerifier g_mtx_verif;

	/// \class safe_mutex
	/// \brief This class is used as wrapper on std::mutex. For debug builds it tracks and verifies mutexes. For release
	///        builds it is an typedef of std::mutex.
	class safe_mutex : public std::mutex {

        /// \typedef super
        /// \brief Defines parent class
		typedef std::mutex super;

		std::mutex::native_handle_type hndl; ///< Native handle that represent std::mutex

	public:
	    /// \brief Constructor
		safe_mutex();

	    /// \brief Destructor
		virtual ~safe_mutex();

		/// \brief Takes ownership on the mutex
		void lock();

		/// \brief Release ownership on the mutex
		void unlock();

		/// \brief Check if mutex is owned.
		/// \details This method is inaccessible in release builds, therefore #CHECK_SAFE_MUTEX_LOCKED macro should be used
		///          to check if mutex is owned.
		void check_locked();
	};

    class safe_mutex_locker {
        safe_mutex* lk;
    public:
        /// \brief Constructor
        safe_mutex_locker(safe_mutex* sm) : lk(sm) {
            assert(sm != nullptr);
            lk->lock();
        }

        /// \brief Destructor
        ~safe_mutex_locker() {
            lk->unlock();
        }
    };

	/// \def CHECK_SAFE_MUTEX_LOCKED
	/// \brief Checks if the mutex is owned.
	#define CHECK_SAFE_MUTEX_LOCKED(m) {m.check_locked();}

    /// \def LOCK
    /// \brief Locks std::mutex (must be non-recursive mutex)
    #define LOCK(x) tools::safe_mutex_locker  safe_locker_##x(&(x));
#else
	// In release version safe_mutex is a mutex
	/// \typedef safe_mutex
	/// \brief safe_mutex is an alias of std::mutex in release builds.
	typedef std::mutex safe_mutex;

    /// \def CHECK_SAFE_MUTEX_LOCKED
	/// \brief Does nothing in release builds
	#define CHECK_SAFE_MUTEX_LOCKED(m) {}

    /// \def LOCK
    /// \brief Locks std::mutex (must be non-recursive mutex)
    ///  #define LOCK(x)  			std::unique_lock<std::mutex> _unique_lock(x);

#endif


    /// \brief Prints message in std::out
    /// \param format - printf format specified
    /// \param ... - parameters
    /// \details This function is adopted to be used in python extensions
	void debug_print(const char *format, ... );

	/// \class StopWatch
	/// \brief This class is used for proper timeout calculations
    /// \tparam DURATION_UNIT - unit to measure time. Must be std::chrono::duration<>.
	template<class DURATION_UNIT>
	class StopWatch {

        int timeout;        ///< Timeout parameter passed with constructor

        size_t accum_time;  ///< Accumulated time (except time elapsed between StopWatch#pause() and StopWatch#resume()
                            ///  calls.

        bool paused;        ///< true if paused, otherwise false.

        std::chrono::time_point<std::chrono::steady_clock> start_time;  ///< Time point corresponding to start time of #StopWatch.

    public:

	    StopWatch() = delete;   /// Deleted default constructor

	    /// \brief - Constructor
	    /// \param to - time out value in DURATION_UNITs. Zero or negative means infinite timeout.
		explicit StopWatch(int to) {
			timeout = to;
			restart();
		}

		/// \brief Restarts StopWatch
		void restart() {
	        paused = false;
	        accum_time = 0;
			start_time = std::chrono::steady_clock::now();
		}

		/// \brief Measures time elapsed
		/// \return Number of DURATION_UNITs elapsed since last start.
		size_t measure() {
	        if (paused) {
	            return accum_time;
	        } else {
	            return accum_time + std::chrono::duration_cast<DURATION_UNIT>(std::chrono::steady_clock::now() - start_time).count();
	        }
		}

		/// \brief Checks if time out is expired
		/// \return true if time out is expired, otherwise false
		bool expired() {
			if (timeout>0) {
				return measure()>timeout;
			}
			return false;
		}

		/// \brief Pauses StopWatch
		void pause() {
	        if (!paused) {
	            accum_time = measure();
	            paused = true;
	        } else {
	            assert(false); // Logic error
	        }
		}

		/// \brief Resumes StopWatch
		void resume() {
	        if (paused) {
	            start_time = std::chrono::steady_clock::now();
	            paused = false;
	        } else {
	            assert(false); // Logic error
	        }
		}
	};

    /// \brief Concatenates and return two vectors appended.
    /// \tparam T - Type of the element.
    /// \param v1 - The first vector (goes first in resulting vector).
    /// \param v2 - The second vector (goes last in resulting vector).
    /// \return Concatenated result vector.
	template<class T>
	std::vector<T> append_vector(const std::vector<T>& v1, const std::vector<T>& v2) {
		size_t s1 = v1.size();
		size_t s2 = v2.size();
		std::vector<T> res(s1+s2);

		if (s1>0) std::copy(v1.begin(), v1.end(), res.begin());
		if (s2>0) std::copy(v2.begin(), v2.end(), res.begin()+s1);

		return res;
	}

    template<template<class, class, class...> class C, typename K, typename V, typename ... Args>
    V get_with_default(C<K,V,Args...>& container, const K& key, const V& default_value) {
        typename C<K,V,Args...>::iterator i = container.find(key);
        if (i==container.end()) {
            return default_value;
        } else {
            return i->second;
        }
    }

	/// \brief Appends one container to another
	/// \tparam _T - type of the container
	/// \param v1 - Container to be appended
	/// \param v2 - Container to add to v1
	template<class T>
	void join_containers(T& v1, const T& v2) {
		v1.insert(v1.end(), v2.begin(), v2.end());
	}

	/// \brief Calculates simple cyclic redundancy sum (XORed bytes)
	/// \param buffer - buffer to be calculated
	/// \param length - length of the buffer
	/// \param exclude_byte - offset of the byte to be excluded from calculation
	/// \return 8-bit unsigned value representing control sum.
	/// \note It is possible to optimize this function by XORing excluded byte twice, thus "if" statement will be moved out of the
    ///       for loop. Implement this optimization in the case, if performance requirements will be more significant.
    uint8_t calc_contol_sum(uint8_t* buffer, size_t length, size_t exclude_byte);

	/// \brief Simple wrapper on std::this_thread::sleep_for
	/// \param ms
    inline void sleep_ms(size_t ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    /// \brief Calculates timer parameters for STM32F103x timers.
    /// \param freq - Timer clock frequency (typical value - 36000000)
    /// \param delay_s - timer period in seconds
    /// \param prescaller - reference for output value for prescaller
    /// \param period - reference for output value for period
    /// \param eff_s - reference for output value effective period calculated using resulting values. Used to estimate accuracy.
    /// \return 0 if success, non-zero if input values can't be handled by a timer
	int stm32_timer_params(uint32_t freq, double delay_s, uint16_t& prescaller, uint16_t& period, double& eff_s);

    /// \brief Calculates timer parameters for STM32F103x timers
    /// \param freq - Timer clock frequency (typical value - 36000000)
    /// \param delay_s - timer period in seconds
    /// \param prescaller - reference for output value for prescaller
    /// \param period - reference for output value for period
    /// \param eff_s - reference for output value effective period calculated using resulting values. Used to estimate accuracy.
    /// \param div - clock divider to be used
    /// \return 0 if success, non-zero if input values can't be handled by a timer
    int stm32_timer_params_with_div(uint32_t freq, double delay_s, uint16_t& prescaller, uint16_t& period, double& eff_s, size_t& clock_divider);

    /// \brief Detects if system is little endian
    /// \return true if little endian, otherwise false.
    bool is_little_endian();

    /// \brief Makes pid file
    /// \return true if success, false if file is already created and can't be overwritten (likely another such program
    ///         is running).
    bool make_pid_file();
}

/// @}