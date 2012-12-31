#include <stdint.h>
#include <stdlib.h>
#include <cassert>

#include "ctors.h"
#include "status.h"
#include "recv.h"
#include "core.h"

namespace core {
    bool Tagged::is_fixint() {
        return (this->val & 0x3) == 0;
    }

    namespace headers {
        nym_t ref('r','e','f');
        nym_t  pr('_','p','r'); nym_t pair = pr;
        nym_t vec('v','e','c'); nym_t vectorlike = vec;
        nym_t bvl('b','v','l'); nym_t bytevectorlike = bvl;
        nym_t atm('a','t','m');
        nym_t rcd('r','c','d'); nym_t record = rcd;
        nym_t blb('b','l','b'); nym_t blob = blb;
        nym_t bsq('b','s','q'); nym_t bit_seq = bsq;
    }
}
