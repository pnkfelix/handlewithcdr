#ifdef RECV_H_INCLUDED
#error "recv.h multiply included"
#endif
#define RECV_H_INCLUDED

#define RECV_T(x_t) x_t *
#define SET_RECV(name, val) do { *name = val; } while(0)
