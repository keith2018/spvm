layout (location = 0) out int outAssert;

#define ASSERT_BEGIN(n)                     \
    int assertIdx = 0;                      \
    bool asserts[n];

#define ASSERT(condition)                   \
    asserts[assertIdx] = (condition);       \
    assertIdx++;

#define ASSERT_END                          \
    outAssert = -1;                         \
    for (int i = 0; i < assertIdx; i++) {   \
        if (!asserts[i]) {                  \
            outAssert = i;                  \
            break;                          \
        }                                   \
    }
