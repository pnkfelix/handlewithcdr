#include <stdint.h>
#include <stdlib.h>
#include <cassert>

#include "ctors.h"
#include "status.h"
#include "recv.h"
#include "core.h"

namespace spaces {

    class SimpleSpace : public core::Space {
        virtual void* gcalloc(core::formatted_t h, size_t n) {
            size_t bytes = n*sizeof(uintptr_t);
            void *m = malloc(bytes);
            assert(m != 0); // GUMP: assume mallocs don't fail
            assert((uintptr_t(m) & 0x7) == 0); // GUMP: assume malloc is aligned
            return m;
        }
    };

    class Policy {
        
    };

    template<typename Block>
    class Space {
        status::status_t request(size_t size, Block *recv);
    };
};

#include <iostream>

int main()
{
    core::FixInt i(0);
    core::Atom a = i;
    core::Literal t = core::constants::Literal_true;
    core::Literal f = core::constants::Literal_false;
    std::cout << "Hello World\n";
    std::cout << "      a.is_fixint:" << a.is_fixint() << "\n";
    std::cout << "      i.is_fixint:" << i.is_fixint() << "\n";
    std::cout << "      t.is_bool:"   << t.is_bool() << "\n";
    std::cout << "      t.truth:"     << (t.truth() ? "true" : "false")  << "\n";
    std::cout << "      f.is_bool:"   << f.is_bool() << "\n";
    std::cout << "      f.truth:"     << (f.truth() ? "true" : "false") << "\n";
    std::cout << " vec nym:" << core::headers::vec.decode() << "\n";
    std::cout << "blob nym:" << core::headers::blob.decode() << "\n";

    spaces::SimpleSpace s;
    core::handle_t l = s.null();
    l = s.cons(core::FixInt(3), l);
    l = s.cons(core::FixInt(2), l);
    l = s.cons(core::FixInt(1), l);
    std::cout << "     l0:is_fixint:" << l.is_fixint() << "\n";

    core::handle_t x = l.seq_car();
    std::cout << "     l0:is_seq:" << l.is_seq() << "\n";
    std::cout << "     x0:is_fixint:" << x.is_fixint() << "\n";

    l = l.seq_cdr();
    x = l.seq_car();
    std::cout << "     l1:is_seq:" << l.is_seq() << "\n";
    std::cout << "     l1:is_null:" << l.is_null() << "\n";
    std::cout << "     x1:is_fixint:" << x.is_fixint() << "\n";

    l = l.seq_cdr();
    x = l.seq_car();
    std::cout << "     l2:is_seq:" << l.is_seq() << "\n";
    std::cout << "     l2:is_null:" << l.is_null() << "\n";
    std::cout << "     x2:is_fixint:" << x.is_fixint() << "\n";

    l = l.seq_cdr();
    std::cout << "     l3:is_seq:" << l.is_seq() << "\n";
    std::cout << "     l3:is_null:" << l.is_null() << "\n";

    // core::handle_t x = l.pair_car();
    return 0;
}
