#include "cozy_chaos.h"

COMMON_FUNCS(rolling_link, cozy_rolling_link, "Rolling Link")

CozyChaosEffect cozy_rolling_link = {
    .effect = {
        .name = "Rolling Link",
        .duration = 20*20,
        .on_start_fun = on_rolling_link_activate,
        .update_fun = NULL,
        .on_end_fun = on_rolling_link_end,
    },
    .disturbance = CHAOS_DISTURBANCE_HIGH,
    .active = false,
    .entity = NULL,
};

RECOMP_HOOK("Player_Action_26") void before_player_rolling(Player* this, PlayState* play) {
    if (!cozy_rolling_link.active) {
        return;
    }

    if (this->av2.actionVar2 == 0) {
        f32 speedTarget = 15.0f;
        Math_AsymStepToF(&this->speedXZ, speedTarget, 0.1f, 0.1f);
        if (sControlStickMagnitude > 8) {
            s16 yawTarget = sControlStickWorldYaw;
            Math_ScaledStepToS(&this->yaw, yawTarget, 0x100);
            this->actor.shape.rot.y = this->yaw;
        }
        if (this->skelAnime.curFrame > 17) {
            this->speedXZ = speedTarget;
            this->skelAnime.curFrame = 4.0f;
        }
    }
}
