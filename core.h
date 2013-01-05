/* -*- mode: c++; indent-tabs-mode: nil; -*- */

#ifdef CORE_H_INCLUDED
#error "core.h multply included"
#endif
#define CORE_H_INCLUDED

#include <iostream>

namespace core {

    typedef size_t word_offset_t;
    typedef size_t byte_offset_t;

#ifndef CTORS_H_INCLUDED
#error "val.h requires previous include: ctors.h"
#endif

    template <size_t n> class WordBut;

    // A word is a word-sized value.
    class Word {
        friend class WordBut<2>;
        friend class WordBut<3>;
        friend class WordBut<4>;
        friend class WordBut<5>;

    public:
        // The denotations of these names are explained in the comment
        // below the function definition for variant().
        enum Variant { snokref, konsref, valref, intrref,
                       blobmdr, blobhdr, vechdr, bvlhdr,
                       literal, fixnum };
        typedef Variant variant_t;

        variant_t variant() {
            // These numbers are explained in the table in the comment
            // below the function definition.
            switch (val & 0x7) {
            case 3: return snokref;
            case 1: return konsref;
            case 5: return valref;
            case 7: return intrref;

            case 2: case 6: {
                switch (val & 0x1f) {
                case 0x0a:            return blobmdr;
                case 0x06: case 0x16: return blobhdr;
                case 0x02: case 0x12: return vechdr;
                case 0x0e: case 0x1e: return bvlhdr;
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
        // snokref : ... aaaa aaaa aaaa aaaa aaaa aaaa aaaa a011 ==  0x3 |3
        // konsref : ... aaaa aaaa aaaa aaaa aaaa aaaa aaaa a001 ==  0x1 |3
        //  valref : ... aaaa aaaa aaaa aaaa aaaa aaaa aaaa a101 ==  0x5 |3
        // intrref : ... aaaa aaaa aaaa aaaa aaaa aaaa aaaa a111 ==  0x7 |3
        //
        // blobmdr : ... dddd dddd dddd dddd dddd dddd ddd0 1010 == 0x0a |5
        // blobhdr : ... aaaa abbb bbcc cccl llll kkkk kkkk 0110 ==  0x6 |4
        //  vechdr : ... aaaa abbb bbcc cccl llll llll llll 0010 ==  0x2 |4
        //  bvlhdr : ... aaaa abbb bbcc cccc cckk kkkk kkkk 1110 ==  0xe |4
        // literal : ... xxxx xxxx xxxx xxxx xxxx xxxx xxx1 1010 == 0x1a |5
        //
        //  fixnum : ... xxxx xxxx xxxx xxxx xxxx xxxx xxxx xx00 ==  0x0 |2
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
        // - konsref is ptr to a list (rest); interpret self as [head] ++ rest,
        // - snokref is ptr to a list (prev); interpret self as prev ++ [last].
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

    public:
        uintptr_t uint() const { return val; }
    protected:
        uintptr_t val;

        NO_NULL_CTOR(Word);
    public:
        Word(Word const &x) : val(x.val) {}
        Word operator<<(intptr_t x) { return Word(val << x); }
        Word operator>>(intptr_t x) { return Word(val >> x); }
        Word operator&(uintptr_t x) { return Word(val & x); }
        Word operator&(Word x) { return Word(val & x.val); }
        bool operator==(uintptr_t x) { return val == x; }
        Word operator|(uintptr_t x) { return Word(val | x); }
        Word operator|(Word x) { return Word(val | x.val); }
    };
    typedef Word word_t;

    template <size_t n>
    class WordBut {
        static bool tagbitsclear(uintptr_t w) {
            return (w >> (sizeof(uintptr_t)*8 - n)) == 0;
        }
    public:
        WordBut(uintptr_t w) : content(w) { assert(tagbitsclear(w)); }
    public:
        Word tag(uintptr_t t) { assert((t >> n) == 0);
            return Word(content << n | t);
        }
        NO_NULL_CTOR(WordBut);
        private:
            uintptr_t content;
    };
    class Content2 : public WordBut<2> { };
    class Content3 : public WordBut<3> { };
    class Content4 : public WordBut<4> { };
    class Content5 : public WordBut<5> {
    public:
        Content5(uintptr_t c) : WordBut(c) { }
    };

#define DECLARE_BOOL_METHODS(MyType)                    \
    bool bool_value(); /* req. this is boolean */

#define DECLARE_SEQ_METHODS(MyType)                                    \
    MyType seq_car();                                                  \
    MyType seq_cdr();                                                  \
    /* END SEQ METHODS */

#define DECLARE_PAIR_METHODS(MyType)                                    \
    /* Pair primops (req. this is pair). */                             \
    MyType pair_car();                                                  \
    MyType pair_cdr();                                                  \
    void pair_setcar(MyType x);                                         \
    void pair_setcdr(MyType x);                                         \

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
    bool truth();  /* never fails; false solely for #f */               \
    DECLARE_BOOL_METHODS(MyType)                                        \
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
    bool is_null(); /* infallible; true solely for #null. */            \
    bool is_void(); /* infallible; true solely for #void. */            \
    bool is_kons(); /* infallible; true solely for konsref */           \
    bool is_snok(); /* infallible; true solely for snokref */           \
    bool is_seq();  /* infallible; true for #null konsref + snokref  */ \
    bool is_pair(); /* infallible; true for kons/snok/val<_pr> ref  */  \
    DECLARE_SEQ_METHODS(MyType)                                         \
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

    // A formatted-word has a identifier in its low bits.
    // A tagged-word is formatted and can be treated as a value.
    // (The tagging scheme is variable-width, so the number of tag bits is
    //  unspecified here.)

    class Formatted : protected Word
    {
    protected:
        Formatted(word_t w) : Word(w) {}

        NO_NULL_CTOR(Formatted);
    public:
        Formatted(Formatted const &x) : Word(x) {}
    };
    typedef Formatted formatted_t;

    class Tagged : public Formatted
    {
    public:
        DECLARE_PRIMOP_METHODS(Tagged);
    protected:
        Tagged(word_t w);

        NO_NULL_CTOR(Tagged);
    public:
        Tagged(Tagged const &x);
    public:
        uintptr_t uint() const { return Word::uint(); }
    };
    typedef Tagged tagged_t;

    // A nym is a three-letter word used to describe the structure for
    // an object.  The letters are drawn from a limited alphabet
    // so that each letter can be encoded in 6 bits.
    //
    class Nym : public Formatted {
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
        Nym(char a, char b, char c) : Formatted(tag(a,b,c)) {}
        Nym(Nym const&x) : Formatted(x) {}
    };
    typedef Nym nym_t;

    namespace headers {
        // nym_t are used both as headers and to express class relationships.
        extern nym_t 
            pr, pair,
            vec, vectorlike, bvl, bytevectorlike, atm, rcd, record, blb, blob, bsq, bit_seq;
    }

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

    class Space;
    class Handle {
        friend class Space;
    public:
        DECLARE_PRIMOP_METHODS(Handle);
    private:
        Handle(Handle *prev,
               Handle *next,
               Tagged v) : prev(prev), next(next), value(v) {}
    public:
        Handle& operator=(Handle const& h) {
            this->value = h.value;
            return *this;
        }

        Handle(Handle const& h) : value(h.value) {
            this->prev = h.prev;
            if (h.prev) h.prev->next = this;
            h.prev = this;
            this->next = &h;
        }

        Handle(Handle const& h, tagged_t v) : value(v) {
            this->prev = h.prev;
            if (h.prev) h.prev->next = this;
            h.prev = this;
            this->next = &h;
        }

        ~Handle() {
            if (this->prev)
                this->prev->next = this->next;
            if (this->next)
                this->next->prev = this->prev;
        }
    private:
        mutable Handle *prev;
        const Handle *next;
    private:
        Tagged value;
    public:
        uintptr_t uint() const { return value.uint(); }
    };
    typedef Handle handle_t;

    // A space holds a collection of memory blocks, and also a set of
    // roots for the space.
    class Space {
    public:
        Space();
        // These are all named "cons"/"snoc" rather than "kons"/"snok" because
        // they handle arbitrary inputs (and then dispatch to kons/snok when
        // appropriate for the inputs).

        handle_t cons(handle_t ar, handle_t dr);
        handle_t cons(Atom ar, handle_t dr);
        handle_t cons(handle_t ar, Atom dr);
        handle_t cons(Atom ar, Atom dr);

        handle_t snoc(handle_t ar, handle_t dr);
        handle_t snoc(Atom ar, handle_t dr);
        handle_t snoc(handle_t ar, Atom dr);
        handle_t snoc(Atom ar, Atom dr);

        handle_t make_vec(nym_t h, size_t num_vals, handle_t val);
        handle_t make_vec(nym_t h, size_t num_vals, Atom val);

        handle_t make_bvl(nym_t h, size_t num_bytes);
        handle_t make_blob(nym_t h, size_t num_vals, handle_t val, size_t num_bytes);
        handle_t make_blob(nym_t h, size_t num_vals, Atom val, size_t num_bytes);

        handle_t null(); // even though this does not allocate heap-space,
        // we need to create a handle for #null so that it can be passed
        // in the same uniform manner to the other methods of Space.
        //
        // Is this crazy?  It certainly seems crazy.  Maybe there should
        // be two kinds of handles, rooted and unrooted, and then use
        // method dispatch.  It seems like a tradeoff between:
        //   1. virtual-method overhead + some amount of manual management
        // versus
        //   2. root-list-overhead + handle-size overhead
        //
        // But since experimenting with automatic handle management
        // was the whole point of this exercise, let us follow through
        // with it.
        //
        // (Also, the expected case is that the handles *will* tend to
        //  be used in contexts where they do carry useful state, not
        //  places where they are statically known to be non-heap
        //  values.)
        //
        // UPDATE: The other solution, also incorporated above, is to
        // overload the cons+make_x methods so that anything that
        // takes a handle_t can also take a Atom instead.  This way,
        // when you *do* know you are dealing with an atom, you need
        // not acquire a handle to manage it.

    public:
        void print_roots();

    private:
        // h, n       -> [h, x_2, x_3, ..., x_n] where x_i *unformatted*
        virtual void* gcalloc(formatted_t h, size_t n) = 0;
        // h, w, n    -> [h, w_2, w_3, ..., w_n]
        virtual void* gcalloc(formatted_t a, word_t w, size_t n) {
            word_t *m = (word_t*) this->gcalloc(a, n);
            for (size_t i = 1; i < n; i++) m[i] = w;
            return m;
        }
        // a, b       -> [a, b]
        virtual void* gcalloc(formatted_t a, formatted_t b) {
            formatted_t *m = (formatted_t*) this->gcalloc(a, 2);
            m[1] = b;
            return m;
        }
    private:
        handle_t *next;
    };

    // A ref-word (ref) is a tagged reference to another object.
    // A ref-word may point to the beginning or to the interior of its
    // target.
    // Since it is tagged, it cannot be directly dereferenced;
    // one should instead use one of the getref/getraw methods
    // to extract its contents.
    class Ref : public Tagged {
        friend class Space;
    public:
        static void checkaligned(uintptr_t x) { assert((x & 0x7) == 0); }
        static intptr_t tagsnok(uintptr_t x) { return x | 0x3; }
        static intptr_t tagkons(uintptr_t x) { return x | 0x1; }
        static intptr_t  tagval(uintptr_t x) { return x | 0x5; }
        static intptr_t tagintr(uintptr_t x) { return x | 0x7; }
        static intptr_t tagvariant(uintptr_t x, variant_t variant) {
            checkaligned(x);
            switch (variant) {
            case snokref: return tagsnok(x);
            case konsref: return tagkons(x);
            case valref:  return tagval(x);
            case intrref: return tagintr(x);
            default: assert(0);
            }
        }
    protected:
        Ref(intptr_t w, variant_t variant);

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
        explicit Literal(Content5 c) : Atom(c.tag(0x1a)) { }

        NO_NULL_CTOR(Literal);
    public:
        Literal(Literal const &x) : Atom(x) {}
    };

    namespace constants {
#define DEFLITERAL(x, v)                                \
        const Content5 Content ## x (v);               \
        const Literal Literal ## x (Content ## x);

        DEFLITERAL(_true, 0x0);  // canonical truth, #t (for pure bool fcns)
        DEFLITERAL(_false, 0x1); // the false value, #f
        DEFLITERAL(_void, 0x2);  // the undisplayed value, #void
        DEFLITERAL(_null, 0x3);  // the empty list, #null
#undef DEFLITERAL
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
    //
    // The layer before this comment is sufficient for representing
    // Scheme-like systems: e.g. references carry tags to classify
    // their targets (to some degree; this mainly enables header-less
    // pairs), and all objects have fairly uniform representations
    // (contiguous sequences of tagged words, or contiguous sequences
    // of reference-free bytes).
    //
    // However, interoperability with native libraries may require
    // constructing objects with more interesting layouts.  At this
    // point, the text below is just some not-well-thought-out notes
    // on one tack I took for that topic.  The end reality will
    // probably differ significantly.  In any case, I may end up
    // needing to reshuffle the header bit patterns to grab one for
    // usage here.  (Best bet there is probably to take some bits away
    // from literal and/or blobmdr).

    // A word-sequence (wsq, word-seq) is a contiguous series of words.
    //
    // word-sequences are allocated on the managed-heap.  This class
    // does not represent that allocated state directly, but rather
    // a managed handle pointing to it.
    class WordSeq : private Handle {

        NO_DEFAULT_CTORS(WordSeq);
        size_t wordsize(); // interface method, do not call.
    };

    // A cons-pair is a header-less word-seq of 2 words, where the second word
    // is always a seq.
    class Cons : private WordSeq {

        NO_DEFAULT_CTORS(Cons);
        size_t wordsize() { return 2; }
    };

    // A snoc-pair is a header-less word-seq of 2 words, where the second word
    // is always a seq.
    class Snoc : private WordSeq {

        NO_DEFAULT_CTORS(Snoc);
        size_t wordsize() { return 2; }
    };

    // A pair is a _pr headered (!) word-seq of 2 arbitrary words.
    // It is constructed by cons when the second argument is a non-seq.
    class Pair : private WordSeq {

        NO_DEFAULT_CTORS(Pair);
        size_t wordsize() { assert(0); }
    };

    class Vec : private WordSeq {

        NO_DEFAULT_CTORS(Vec);
        size_t wordsize() { assert(0); }
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
