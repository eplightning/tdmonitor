#pragma once

#include <iostream>
#include <type_traits>

namespace TDM {

    template<typename TF>
    void write_debug_output( std::ostream & out, TF const& f ) {
        out << f;
    }

    struct tracer {
        std::ostream & out;
        tracer( std::ostream & out)
            : out( out ) {
        }
        ~tracer() {
            out << std::endl;
        }

        template<typename TF, typename ... TR>
        void write( TF const& f, TR const& ... rest ) {
            write_debug_output( out, f );
            out << " ";
            write( rest... );
        }
        template<typename TF>
        void write( TF const& f ) {
            write_debug_output( out, f );
        }
        void write() {
            //handle the empty params case
        }
    };

}

#define TDM_LOG(...) TDM::tracer(std::cerr).write( __VA_ARGS__ )
