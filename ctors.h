#ifdef CTORS_H_INCLUDED
#error "ctors.h multiply included"
#endif
#define CTORS_H_INCLUDED

#define NO_NULL_CTOR(name) private: name();
#define NO_COPY_CTOR(name) private: name(name const&x);
#define NO_DEFAULT_CTORS(name) NO_NULL_CTOR(name); NO_COPY_CTOR(name);
