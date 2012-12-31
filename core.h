/* -*- mode: c++; indent-tabs-mode: nil; -*- */

#ifdef CORE_H_INCLUDED
#error "core.h multply included"
#endif
#define CORE_H_INCLUDED

namespace core {

    typedef size_t word_offset_t;
    typedef size_t byte_offset_t;

#ifndef CTORS_H_INCLUDED
#error "val.h requires previous include: ctors.h"
#endif

    // A word is a word-sized value.
    class Word {
    public:
        // The denotations of these names are explained in the comment
        // below the function definition for variant().
        enum Variant { snocref, consref, valref, intrref,
                       blobmdr, blobhdr, vechdr, bvlhdr,
                       literal, fixnum };
        typedef Variant variant_t;

        variant_t variant() {
            // These numbers are explained in the table in the comment
            // below the function definition.
            switch (val & 0x7) {
            case 3: return snocref;
            case 1: return consref;
            case 5: return valref;
            case 7: return intrref;

            case 2: case 6: {
                switch (val & 0x1f) {
                case 0x0a:            return blobmdr;
                case 0x06: case 0x16: return blobhdr;
                case 0x02:            return vechdr;
                case 0x1e:            return bvlhdr;
                case 0x1a:            return literal;
                default:            assert(0);
                }
            }

            case 0: case 4: return fixnum;
            }
            assert(0);
        }

        // Word format: (we assume a 32-bit word minimum).
        //
        // snocref : ... aaaa aaaa aaaa aaaa aaaa aaaa aaaa a011
        // consref : ... aaaa aaaa aaaa aaaa aaaa aaaa aaaa a001
        //  valref : ... aaaa aaaa aaaa aaaa aaaa aaaa aaaa a101
        // intrref : ... aaaa aaaa aaaa aaaa aaaa aaaa aaaa a111
        //
        // blobmdr : ... dddd dddd dddd dddd dddd dddd ddd0 1010
        // blobhdr : ... aaaa abbb bbcc cccl llll kkkk kkkk 0110
        //  vechdr : ... aaaa abbb bbcc cccl llll llll lll0 0010
        //  bvlhdr : ... aaaa abbb bbcc cccc cckk kkkk kkk1 1110
        // literal : ... xxxx xxxx xxxx xxxx xxxx xxxx xxx1 1010
        // (unused): ... xxxx xxxx xxxx xxxx xxxx xxxx xxx0 1110
        //
        //  fixnum : ... xxxx xxxx xxxx xxxx xxxx xxxx xxxx xx00
        //
        // shorthands above:
        //
        // a: address or tag name; b, c: tag name; d: delta;
        // k: byte length; l: word length; x: uninterpreted content.
        //
        // Explanation (in reverse order):
        // - fixnum is its own immediate contents: (w >> 2),
        //
        // - literal is some word-sized constant: (w >> 5),
        // - bvlhdr starts a byte-vector-like, length k in bytes (see below),
        // - vechdr starts a vector-like, length l in words (see below),
        // - blobhdr starts a blob, length l words + k bytes (see below),
        // - blobmdr is the interior marker of a blob; header is d words above,
        //
        // - intrref is ptr into tagged portion of object; scan up for header,
        // - valref is ptr to a header (or midder for a blob),
        // - consref is ptr to a list (rest); interpret self as [head] ++ rest,
        // - snocref is ptr to a list (prev); interpret self as prev ++ [last].
        //
        // All of the -hdr's are the starting word for a heap object.
        // The length of the object is *either* encoded in the header
        // (l+/k+) or, if the bits there are insufficient (in which
        // case l/k are both set to all ones), then the length is held
        // in the immediately following word (or following two words,
        // in the case of blobs, since they have two distinct parts).
        // This detail is important, since it means that the bit-widths
        // for l/k are not fundamental limits on the number of values/bytes
        // that one stores in a vec, bvl, or blob; it just means that there
        // is a little bit more overhead for each object that goes over
        // the limit.
        //
        // Note also that the lengths encoded in the headers does not
        // include the space for the header (or middler, or auxiliary
        // length fields).

    protected:
        Word(uintptr_t w) : val(w) {}
    protected:
        uintptr_t val;

        NO_NULL_CTOR(Word);
    public:
        Word(Word const &x) : val(x.val) {}
        Word operator<<(size_t x) { return Word(val << x); }
        Word operator|(Word x) { return Word(val | x.val); }
    };
    typedef Word word_t;


#define DECLARE_BOOL_METHODS(MyType)                    \
    bool bool_value(); /* req. this is boolean */

#define DECLARE_PAIR_METHODS(MyType)                                    \
    /* Pair primops (req. this is pair). */                             \
    MyType pair_car();                                                  \
    MyType pair_cdr();                                                  \
    void pair_setcar(MyType x);                                         \
    void pair_setcdr(MyType x);                                         \
    /* END PAIR METHODS */

#define DECLARE_VEC_METHODS(MyType)                                     \
    /* Vector-like primops (req. this is vector-like) */                \
    size_t vec_value_capacity(); /* number of elements */               \
    MyType vec_fetch(uintptr_t i); /* req. i < capacity */              \
    void   vec_store(uintptr_t i, MyType x); /* req. i < capacity */    \
    /* END VEC METHODS */

#define DECLARE_BVL_METHODS(MyType)                                     \
    /* ByteVec primops (req. this is byte-vector-like) */               \
    size_t  bvl_byte_capacity(); /* number of bytes */                  \
    uint8_t bvl_get(uintptr_t i); /* req. i < capacity */               \
    void    bvl_set(uintptr_t i, uint8_t x); /* req. i < capacity */    \
    /* END BVL METHODS */

#define DECLARE_BLOB_METHODS(MyType)                                    \
    /* Blob primops (req. this is blob [see below]) */                  \
    size_t  blob_val_capacity();                                        \
    size_t  blob_raw_capacity();                                        \
    MyType  blob_fetch(uintptr_t i); /* req. i < val_capacity */        \
    void    blob_store(uintptr_t i, MyType x); /* req. i < val_capacity */ \
    uint8_t blob_get(uintptr_t i);   /* req. i < byte_capacity */       \
    void    blob_set(uintptr_t i, uint8_t x); /* req. i < raw_capacity */ \
    /* END BLOB METHODS */

#define DECLARE_PRIMOP_METHODS(MyType)                                  \
    bool is_bool(); /* never fails; true for boolean (#t, #f)  */       \
    bool truth_value();  /* never fails; false solely for #f */         \
    DECLARE_BOOL_METHODS(MyType)                                        \
                                                                        \
    bool is_null(); /* never fails; true solely for #null. */           \
                                                                        \
    /* Dynamically-checked primops. */                                  \
                                                                        \
    bool is_fixint();                                                   \
    /* requires: this is fixint. */                                     \
    intptr_t fixint_value();                                            \
                                                                        \
    /* requires: this is heap allocated.  Includes header (if any).  */ \
    size_t allocated_length();                                          \
                                                                        \
    bool is_pair();                                                     \
    DECLARE_PAIR_METHODS(MyType)                                        \
                                                                        \
    bool is_vec();                                                      \
    DECLARE_VEC_METHODS(MyType)                                         \
                                                                        \
    bool is_bvl();                                                      \
    DECLARE_BVL_METHODS(MyType)                                         \
                                                                        \
    bool is_blob();                                                     \
    DECLARE_BLOB_METHODS(MyType)                                        \
    /* END OF PRIM OP LIST */

#define DECLARE_WORD_ORIENTED_RAW_ACCESSORS() \
    uint8_t   u8(word_offset_t i);            \
    uint16_t u16(word_offset_t i);            \
    uint32_t u32(word_offset_t i);            \
    uint64_t u64(word_offset_t i);            \
    int8_t    s8(word_offset_t i);            \
    int16_t  s16(word_offset_t i);            \
    int32_t  s32(word_offset_t i);            \
    int64_t  s64(word_offset_t i);            \
    float    flo(word_offset_t i);            \
    double   dbl(word_offset_t i);            \
    /* END OF WORD ORIENTED RAW ACCESSORS */

#define DECLARE_BYTE_ORIENTED_RAW_ACCESSORS() \
    uint8_t  u08(byte_offset_t i);            \
    uint16_t u16(byte_offset_t i);            \
    uint32_t u32(byte_offset_t i);            \
    uint64_t u64(byte_offset_t i);            \
    int32_t  i32(byte_offset_t i);            \
    float    flo(byte_offset_t i);            \
    double   dbl(byte_offset_t i);            \
    /* END OF BYTE ORIENTED RAW ACCESSORS */


    // A tagged-word has a identifier in its low bits.
    // (The tagging scheme is variable-width, so the number of tag bits is
    //  unspecified here.)
    class Tagged : private Word
    {
    public:
        DECLARE_PRIMOP_METHODS(Tagged);
    protected:
        Tagged(word_t w) : Word(w) {}

        NO_NULL_CTOR(Tagged);
    public:
        Tagged(Tagged const &x) : Word(x) {}
    };

    // A nym is a three-letter word used to describe the structure for
    // an object.  The letters are drawn from a limited alphabet
    // so that each letter can be encoded in 6 bits.
    //
    class Nym : private Word {
    private:
        static uintptr_t encode(char a, char b, char c) {
            assert(a <= 122);
            assert(a >= 91);
            assert(b <= 122);
            assert(b >= 91);
            assert(c <= 122);
            assert(c >= 91);
            return ((((a-91) << 5) | (b - 91)) << 5) | (c - 91);
        }
        static char* decode(uintptr_t x) {
            static char result[4] = {'X', 'X', 'X', '\0'};
            result[2] = char(((x >>  0) & 0x1f) + 91);
            result[1] = char(((x >>  5) & 0x1f) + 91);
            result[0] = char(((x >> 10) & 0x1f) + 91);
            return result;
        }
        uintptr_t tag(char a, char b, char c) { return encode(a,b,c) << 2; }
    public:
        char* decode() { return decode(val >> 2); }
        NO_NULL_CTOR(Nym);
    public:
        Nym(char a, char b, char c) : Word(tag(a,b,c)) {}
        Nym(Nym const&x) : Word(x) {}
    };
    typedef Nym nym_t;

    namespace headers {
        extern nym_t ref, pr, pair, vec, vectorlike, bvl, bytevectorlike, atm, rcd, record, blb, blob, bsq, bit_seq;
    }

    class Handle {
    public:
        DECLARE_PRIMOP_METHODS(Tagged);
    private:
        Handle *prev;
        Handle *next;
        Tagged value;
    };
    typedef Handle handle_t;

    // A space holds a collection of memory blocks, and also a set of
    // roots for the space.
    class Space {
        handle_t cons(handle_t ar, handle_t dr);
        handle_t make_vec(nym_t h, size_t num_vals, handle_t val);
        handle_t make_bvl(nym_t h, size_t num_bytes);
        handle_t make_blob(nym_t h, size_t num_vals, handle_t val, size_t num_bytes);
        handle_t *next;
    };

    // An atom-word (atm, atom) is a tagged self-contained word-sized value.
    class Atom : public Tagged {
    protected:
        Atom(uintptr_t x) : Tagged(x) {}
    protected:
        Atom(word_t w) : Tagged(w) {}

        NO_NULL_CTOR(Atom);
    public:
        Atom(Atom const &x) : Tagged(x) {}
    };
    typedef Atom atom_t;

    // A ref-word (ref) is a tagged reference to another object.
    // A ref-word may point to the beginning or to the interior of its
    // target.
    // Since it is tagged, it cannot be directly dereferenced;
    // one should instead use one of the getref/getraw methods
    // to extract its contents.
    class Ref : public Tagged {
    protected:
        Ref(word_t w) : Tagged(w) {}

        NO_NULL_CTOR(Ref);
    public:
        Ref(Ref const &c) : Tagged(c) {}
    };
    typedef Ref ref_t;

    // A fixed-point-integer (fxi, fixint) is a 30-bit integer.
    class FixInt : public Atom {
    public:
        static intptr_t tag(intptr_t x) { return x << 2 | 0x0; }
    public:
        FixInt(word_t w) : Atom(w) {}
        FixInt(intptr_t x) : Atom(tag(x)) {}
        NO_NULL_CTOR(FixInt);
    public:
        FixInt(FixInt const &x) : Atom(x) {}
    };

    // A literal (lit) is a tagged constant.
    class Literal : public Atom {
    public:
        Literal(word_t w) : Atom(w) {}

        NO_NULL_CTOR(Literal);
    public:
        Literal(Literal const &x) : Atom(x) {}
    };

    // The heap structure is a delicate topic.
    //
    // Here are some desiderata of interest to Felix:
    // - Garbage-Collection (automatic memory reclamation)
    // - Reflection (self-description)
    // - Interoperability (passing certain pointer-sized words to native code)
    // - Expressiveness (ability to construct semi-arbitrary bit-patterns)
    // - Self-hosted (using heap-allocated structure as meta-data for heap)
    //
    // The above goals interact in sometimes conflicting ways; e.g.,
    // it is easier to write a GC for a heap made up of uniform pairs
    // than for blocks holding arbitrary bit-patterns.
    //
    // The heap structures are described in layers, in part to enable
    // self-hosting.

    // A word-sequence (wsq, word-seq) is a contiguous series of words.
    //
    // word-sequences are allocated on the managed-heap.  This class
    // does not represent that allocated state directly, but rather
    // a managed handle pointing to it.
    class WordSeq : private Handle {

        NO_DEFAULT_CTORS(WordSeq);
        size_t wordsize(); // interface method, do not call.
    };

    // A cons-pair (pr, pair) is a header-less word-seq of 2 words.
    class Cons : private WordSeq {

        NO_DEFAULT_CTORS(Cons);
        size_t wordsize() { return 2; }
    };

    // A byte-vector-like (bvl, bytevec) is a word-sequence made solely
    // of bits that will not be interpreted as references by the GC.
    class ByteVec : private WordSeq {

        NO_DEFAULT_CTORS(ByteVec);
        size_t wordsize() { assert(0); }
    };

    // A record (rcd) is a word sequence, where each word is either
    // - a tagged ref-word, (low-order bit set to 1)
    // - a "tagged" raw-word, (low-order bit set to 0)
    // - an untagged word of meta-data (for the record or its block)
    class Record : private WordSeq {

        NO_DEFAULT_CTORS(Record);
    };



    // A layout is a compact bitstring describing a sequence of words.
    class Layout {
        // Each word can be either:
        // - a tagged-value, i.e. ref or atom (tagged)
        // - an untagged uninterpreted datum  (nonptr)
        // - an untagged same-space pointer   (closep)
        // - an untagged far-space pointer    (farptr)
        //
        // Each entry requires 2 bits.
        // We call each two-bit entry a 'crumb', following a
        // less-than-universal (but semi-sane in analogy with 'bit',
        // 'nybble', and 'byte') convention

        class Kind {
            friend class Layout;
            uint8_t val;
            Kind(uint8_t v) : val(v) {}
            static Kind tagged() { return Kind(0); }
            static Kind nonptr() { return Kind(2); }
            static Kind closep() { return Kind(1); }
            static Kind farptr() { return Kind(3); }

            NO_NULL_CTOR(Kind);
        public: Kind(Kind const&x) : val(x.val) {}
        };
        typedef Kind kind_t;

        static const kind_t tagged;
        static const kind_t nonptr;
        static const kind_t closep;
        static const kind_t farptr;

        size_t len; // length of the layout in crumbs (#entries)
        // the allocated size for bits array will be rounded up to the
        // word-boundary for this object.
        int8_t bits[1];
        kind_t describe(size_t i) {
            
            return bits[i];
        }
    };


    // A blob (blb) is a record immediately followed by a sequence of
    // untagged words.

    // A bit-sequence (bsq, bit-seq) is a blob = {header, length}::[bytes ...]

    // A record-layout is a bit-sequence describing a record



    bool is_ref(word_t x);
    bool is_raw(word_t x);

    // Layout descriptors:

    enum {
        word_kind_ref, word_kind_raw, word_kind_any, word_kind_meta
    } word_kind_t;

#ifndef RECV_H_INCLUDED
#error "val,h requires previous include: recv.h"
#endif

};
