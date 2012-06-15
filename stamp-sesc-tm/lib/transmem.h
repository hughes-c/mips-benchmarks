#define BEGIN_TRANSACTION(n) do {                       \
        __asm__ __volatile__ (".word 0x70000000+" #n);  \
} while (0)

#define COMMIT_TRANSACTION(n) do {                      \
        __asm__ __volatile__ (".word 0x7C000000+" #n);  \
} while (0)

#define ABORT_TRANSACTION(n) do {                      \
        __asm__ __volatile__ (".word 0x74000000+" #n);  \
} while (0)

