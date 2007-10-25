#include <cuckoo/cuckoo.h>
#define UNUSED(a) ((void) (a))

#ifdef CK_DEBUG
#define DEBUG(fmt, args...) fprintf(stderr, "%s:%d:%s: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#else
#define DEBUG(...) ((void) 0)
#endif /* CK_DEBUG */
