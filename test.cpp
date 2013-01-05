#include <stdint.h>
#include <stdlib.h>
#include <cassert>

#include "ctors.h"
#include "status.h"
#include "recv.h"
#include "core.h"

namespace spaces {
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
    return 0;
}
