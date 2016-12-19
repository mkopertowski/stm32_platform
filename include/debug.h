#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG_ON

void debug_init(void);
int debug(const char* format, ...);

#define DEBUG_INIT() debug_init()
#define DEBUG_PRINTF(...) debug(__VA_ARGS__)

#else

#define DEBUG_INIT()
#define DEBUG_PRINTF(...)

#endif

#endif // DEBUG_H
