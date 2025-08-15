#include "cozy_chaos.h"

#define MAKE_FAKE(effect_code_name) CozyChaosEffect cozy_##effect_code_name = { .fake = true };

#ifdef ENABLE_BOMB_SPAM
    #include "effects/cozy_bomb_spam.c"
#else
    MAKE_FAKE(bomb_spam);
#endif

#ifdef ENABLE_SUDDEN_REDEAD
    #include "effects/cozy_sudden_redead.c"
#else
    MAKE_FAKE(sudden_redead);
#endif

#ifdef ENABLE_ROLLING_LINK
    #include "effects/cozy_rolling_link.c"
#else
    MAKE_FAKE(rolling_link);
#endif

#ifdef ENABLE_BIG_HEAD
    #include "effects/cozy_big_head.c"
#else
    MAKE_FAKE(big_head);
#endif

#ifdef ENABLE_NA_AIM
    #include "effects/cozy_na_aim.c"
#else
    MAKE_FAKE(na_aim);
#endif

#ifdef ENABLE_TOP_DOWN_ZELDA
    #include "effects/cozy_top_down_zelda.c"
#else
    MAKE_FAKE(top_down_zelda);
#endif
