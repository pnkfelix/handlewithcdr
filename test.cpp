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
    core::FixInt fixint(0);
    core::Atom a = fixint;
    std::cout << "Hello World\n";
    std::cout << "      a.is_fixint:" << a.is_fixint() << "\n";
    std::cout << " fixint.is_fixint:" << fixint.is_fixint() << "\n";
    std::cout << " vec nym:" << core::headers::vec.decode() << "\n";
    std::cout << "blob nym:" << core::headers::blob.decode() << "\n";
    return 0;
}
