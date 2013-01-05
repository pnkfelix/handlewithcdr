#include <stdint.h>
#include <stdlib.h>
#include <cassert>

#include "ctors.h"
#include "status.h"
#include "recv.h"
#include "core.h"

#include <iostream>
#include <iomanip>

std::ostream& operator << (std::ostream& os, const core::Word& w) {
    return os << "Word{val:"
              << "0x" << std::hex << std::setw(8) << std::setfill('0')
              << w.uint()
              << "}";
}

std::ostream& operator << (std::ostream& os, const core::Handle& h) {
    return os << "Handle{value:"
              << "0x" << std::hex << std::setw(8) << std::setfill('0')
              << h.uint()
              << "}";
}

template <typename U>
static void dbmsgln(const char *name, const char *msg, U val, const char *post) {
    std::cout << std::setw(8) << std::setfill(' ') << name << msg  << val << post << "\n";
}

template <typename U>
static void dbmsgln(const char *name, const char *msg, U val) {
    std::cout << std::setw(8) << std::setfill(' ') << name << msg  << val << "\n";
}

template <typename U, typename V>
static void dbmsgln(const char *name, const char *msg, U val, const char *msg2, V val2) {
    std::cout << std::setw(8) << std::setfill(' ') << name << msg  << val << "\n";
}

static void dbmsglnold(const char *pre, uintptr_t val, const char *post) {
    std::cout << pre << std::hex << val << "[0]" << "\n";
}

namespace core {
    Tagged::Tagged(word_t w) : Formatted(w) {
        dbmsgln("Tagged", " construction of ", w);
    }
    Tagged::Tagged(Tagged const &x) : Formatted(x) {
        dbmsgln("Tagged", " construction of ", word_t(x));
    }


    bool Tagged::is_fixint() {
        return (this->variant() == fixnum);
    }

    bool Tagged::is_bool() {
        return (this->val == constants::Literal_true.val ||
                this->val == constants::Literal_false.val);
    }

    bool Tagged::is_null() {
        return (this->val == constants::Literal_null.val);
    }

    bool Tagged::is_void() {
        return (this->val == constants::Literal_void.val);
    }

    bool Tagged::truth() {
        return (this->val != constants::Literal_false.val);
    }

    bool Tagged::is_kons() { return (this->variant() == konsref); }
    bool Tagged::is_snok() { return (this->variant() == snokref); }

    bool Tagged::is_seq() {
        return (this->is_null() || this->is_kons() || this->is_snok());
    }

    Tagged Tagged::seq_car() {
        assert(this->is_kons() || this->is_snok());
        tagged_t *m = (tagged_t*)(this->val & ~0x7);
        dbmsgln("car dereferencing ", "", uintptr_t(m), "[0]\n");
        return m[0];
    }

    Tagged Tagged::seq_cdr() {
        assert(this->is_kons() || this->is_snok());
        tagged_t *m = (tagged_t*)(this->val & ~0x7);
        return m[1];
    }

    Ref::Ref(intptr_t w, variant_t variant) : Tagged(tagvariant(w, variant)) {
        dbmsgln("Ref ", " construction of w:", w);
    }


#define HANDLE_WRAPPED_METHOD_0(type_t, m) \
    type_t Handle::m() { return value.m(); }

    HANDLE_WRAPPED_METHOD_0(bool, is_seq);
    HANDLE_WRAPPED_METHOD_0(bool, is_fixint);
    HANDLE_WRAPPED_METHOD_0(bool, is_null);
#undef HANDLE_WRAPPED_METHOD_0

    handle_t Handle::seq_car() { return Handle(*this, value.seq_car()); }
    handle_t Handle::seq_cdr() { return Handle(*this, value.seq_cdr()); }

    namespace headers {
        nym_t ref('r','e','f');
        nym_t  pr('_','p','r'); nym_t pair = pr;
        nym_t cns('c','n','s'); nym_t cons = cns;
        nym_t snc('s','n','c'); nym_t snoc = snc;
        nym_t vec('v','e','c'); nym_t vectorlike = vec;
        nym_t bvl('b','v','l'); nym_t bytevectorlike = bvl;
        nym_t rcd('r','c','d'); nym_t record = rcd;
        nym_t blb('b','l','b'); nym_t blob = blb;
        nym_t bsq('b','s','q'); nym_t bit_seq = bsq;
        nym_t seq('s','e','q');
        nym_t lst('l','s','t'); nym_t list = lst;
        nym_t deq('d','e','q'); nym_t deque = deq;
        nym_t fcn('f','c','n'); nym_t function = fcn;
    }

    Space::Space() : next(0) {}

    handle_t Space::null() {
        Handle h(0, this->next, constants::Literal_null);
        this->next = &h;
        return h;
    }

    handle_t Space::cons(core::atom_t ar, core::handle_t dr) {
        if (dr.is_seq()) {
            // 1. gc-allocate 2-word-seq s; s[0] = ar; s[1] = dr.value
            // 2. create a consref to S
            // 3. return handle for the consref
            void* s = this->gcalloc(ar, dr.value);
            Handle h(0, this->next, Ref(uintptr_t(s), Word::konsref));
            dbmsgln("cons gcalloced", " s:", s, " h:", h);
            return h;
        } else {
            // 1. gc-allocate 3-word-seq S; 
            //    S[0] = headers::pair, S[1] = ar; S[2] = dr.value
            // 2. create a valref to S
            // 3. return handle for the valref
            void* s = this->gcalloc(headers::pair, 3);
            ((tagged_t*)s)[1] = ar;
            ((tagged_t*)s)[2] = dr.value;
            Handle h(0, this->next, Ref(uintptr_t(s), Word::valref));
            dbmsgln("cons gcalloced", " s:", s, " h:", h);
            return h;
        }
    }
}
