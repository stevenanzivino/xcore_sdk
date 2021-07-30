#ifndef VISION_UTILS_HPP
#define VISION_UTILS_HPP
/**
 * The goal of this file is to contain small, likely inlined,
 * utility functions to reduce dependencies on the standard 
 * libraries. Function bodies should be guarded to 
 * check for things like platform and debug conditions.
 * 
 * */
#include <limits>   // numeric_limits<>::min() is_signed()

/** 
 * Debug printing based on known existing debug prints
 * 
 * @note if you want something to print in \emph{release} builds
 * use printf() from <cstdio>
 * **/
#ifndef NDEBUG
    #ifdef XCORE
        #if (HAS_LIB_RTOS_SUPPORT == 1)
            #include "rtos_printf.h"
            // __attribute__((weak))
            // extern "C" void debug_printf(const char * fmt, va_list args) {rtos_printf(fmt, args);}
            #ifdef debug_printf
                #undef debug_printf
            #endif
            #define debug_printf rtos_printf
        #else
            //#include "debug_print.h" //TODO
            // __attribute__((weak))
            // extern "C" void debug_printf(const char * fmt, va_list args){printf(fmt, fmt args);}
            #include <cstdio>
            #ifdef debug_printf
                #undef debug_printf
            #endif            
            #define debug_printf printf
        #endif
    #else
        #include <cstdio>
        #ifdef debug_printf
            #undef debug_printf
        #endif
        #define debug_printf printf
        // __attribute__((weak))
        //extern "C" void debug_printf(const char * fmt, va_list args){printf(fmt, args);} TODO
    #endif // XCORE
#else
    #include <cstdio>
    #ifdef debug_printf
        #undef debug_printf
    #endif
    inline void debug_printf(const char * fmt, ...){}
#endif

namespace vision{

inline int clamp(int x, int min, int max) {
    return (x <= min ? min : (x > max ? max : x));
}

template <typename T>
T abs(T x) { return x>=0 ? x :-x; }

// TODO rand()


// float tanhf(float x){return x;} todo LUT

/// Maps min sample value to zero and all other values in ascending order
inline constexpr unsigned int sample_to_index(sample_t x)
{
    if( std::numeric_limits<sample_t>::is_signed ){
        if(sizeof(sample_t) == 1) return (unsigned) (0x000000FF & ((int)x - std::numeric_limits<sample_t>::min()));
        if(sizeof(sample_t) == 2) return (unsigned) (0x0000FFFF & ((int)x - std::numeric_limits<sample_t>::min()));
    }
    return (unsigned) x;
}

/// Maps zero to min sample value and all other values in ascending order
inline constexpr sample_t index_to_sample( int x)
{
    if( std::numeric_limits<sample_t>::is_signed ){
        return static_cast<sample_t>(x + std::numeric_limits<sample_t>::min());
    }
    return (sample_t) x;
}


/*Uncollated_replication, replacates values in a vector
 *    [abc]->[aabbcc] {replication by 2}
 * 
 * Intended to expand the vectors associated with any kernel by n times to account for more in channels.
 */
inline void uncollated_replication(std::vector<vision::sample_t>& vector, int factor, bool resize=true){
  if(factor < 2)
    return;
  
  if(resize) vector.resize(vector.size()*factor);
  
  int startIndex = (vector.size()-1)/factor;

  for(/*startIndex*/;startIndex > -1; startIndex--){
    int dest = startIndex*factor+factor-1;
    for(int i = 0; i < factor; i++){
      vector[dest-i] = vector[startIndex];
    }
  }
}

} // namespace vision
#endif // include guard