/* -*- mode: c++; indent-tabs-mode: nil; -*- */

#ifdef STATUS_H_INCLUDED
#error "status.h multiply included"
#endif
#define STATUS_H_INCLUDED

namespace status {
    // typedef uintptr_t status_t;
    class Status {
        NO_NULL_CTOR(Status);
    private:
        uintptr_t value;
        Status(uintptr_t v) : value(v) {}
    public:
        Status(Status const&x) : value(x.value) {}
    public:
        static Status success() { return Status(0); }
        static Status failure() { return Status(-1); }
    };
    typedef Status status_t;

    extern const status_t success;
    extern const status_t failure;
}
