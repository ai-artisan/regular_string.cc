#pragma once

#include <list>
#include <memory>

namespace regular_string {
    class Regular {
    public:
        virtual ~Regular() = 0;

        class Match {
        public:
            const bool success;
            const unsigned span;

            explicit Match(const bool &success_, const unsigned &span_) :
                    success(success_),
                    span(span_) {}

            virtual ~Match() = 0;
        };
    };

    namespace regular {
        class Empty {
        public:
            class Match : public Regular::Match {
            public:
                explicit Match() : Regular::Match(true, 0) {}
            };
        };

        class Singleton {
            const char _value;
        public:
            explicit Singleton(const char &value) : _value(value) {}

        };

        class Union {
            const std::list<std::shared_ptr<Regular>> _value;
        public:
            explicit Union(const decltype(_value) &value) : _value(value) {}
        };

        class Concatenation {
            const std::list<std::shared_ptr<Regular>> _value;
        public:
            explicit Concatenation(const decltype(_value) &value) : _value(value) {}
        };

        class KleeneStar {
            const std::shared_ptr<Regular> _value;
        public:

        };
    }
}