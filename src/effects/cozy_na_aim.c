#include "cozy_chaos.h"

COMMON_FUNCS(na_aim, cozy_na_aim, "N/A Aim")

CozyChaosEffect cozy_na_aim = {
    .effect = {
        .name = "N/A Aim",
        .duration = 20*20,
        .on_start_fun = on_na_aim_activate,
        .update_fun = NULL,
        .on_end_fun = on_na_aim_end,
    },
    .disturbance = CHAOS_DISTURBANCE_HIGH,
    .active = false,
    .entity = NULL,
};

RECOMP_HOOK("func_8088ACE0") void on_func_8088ACE0(EnArrow* this, PlayState* play) {
    static s16 wobble_timer = 0;
    if (!cozy_na_aim.active) {
        return;
    }

    wobble_timer++;

    this->actor.shape.rot.y += Math_SinS(wobble_timer * 0x1800) * 0x1000;
    this->actor.world.rot.y = this->actor.shape.rot.y;
    this->actor.velocity.y += Math_SinS(wobble_timer * 0x1573) * 8.0f;
}
