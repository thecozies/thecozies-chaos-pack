#include "cozy_chaos.h"

#define BIG_HEAD_DURATION 20*20
#define BIG_HEAD_MIN_SIZE 1
#define BIG_HEAD_MAX_SIZE 3.5f
s32 big_head_counter = 0;
f32 big_head_size = BIG_HEAD_MIN_SIZE;


void on_big_head_activate(struct PlayState *play) {
    recomp_printf("Big Head Activated\n");
    cozy_big_head.active = true;
    big_head_counter = 0;
    big_head_size = BIG_HEAD_MIN_SIZE;
}

void on_big_head_update(struct PlayState *play) {
    f32 sine_size = Math_SinS(big_head_counter * 0x900) * 0.5f + 0.5f;
    sine_size += Math_SinS(big_head_counter * 0xA00) * 0.5f + 0.5f;
    sine_size += Math_SinS(big_head_counter * 0xE00) * 0.5f + 0.5f;
    sine_size += Math_SinS(big_head_counter * 0x1110) * 0.5f + 0.5f;
    sine_size /= 4.0f;
    f32 frames_from_half = ABS((BIG_HEAD_DURATION / 2.0f) - (f32)big_head_counter);
    f32 from_half_frac = 1.0f - (frames_from_half / (BIG_HEAD_DURATION / 2.0f));
    big_head_size = sine_size * from_half_frac * (BIG_HEAD_MAX_SIZE - BIG_HEAD_MIN_SIZE) + BIG_HEAD_MIN_SIZE;

    big_head_counter++;
    if (big_head_counter >= BIG_HEAD_DURATION) {
        big_head_counter = BIG_HEAD_DURATION;
        cozy_big_head.active = false;
    }
}

void on_big_head_end(struct PlayState *play) {
    recomp_printf("Big Head Deactivated\n");
    cozy_big_head.active = false;
}

CozyChaosEffect cozy_big_head = {
    .effect = {
        .name = "Bomb Spam",
        .duration = BIG_HEAD_DURATION,
        .on_start_fun = on_big_head_activate,
        .update_fun = on_big_head_update,
        .on_end_fun = on_big_head_end,
    },
    .disturbance = CHAOS_DISTURBANCE_LOW,
    .active = false,
    .entity = NULL,
};



s32 valid_drawing = false;
RECOMP_HOOK("Player_DrawGameplay") void on_Player_DrawGameplay(PlayState* play, Player* this, s32 lod, Gfx* cullDList, OverrideLimbDrawFlex overrideLimbDraw) {
    valid_drawing = cozy_big_head.active;
}

s32 valider_drawing = false;
RECOMP_HOOK("Player_OverrideLimbDrawGameplayCommon") void on_Player_OverrideLimbDrawGameplayCommon(
    PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, Actor* actor
) {
    if (!valid_drawing || limbIndex != PLAYER_LIMB_HEAD) {
        valider_drawing = false;
        return;
    }

    valider_drawing = true;
}

RECOMP_HOOK_RETURN("Player_OverrideLimbDrawGameplayCommon") void after_Player_OverrideLimbDrawGameplayCommon() {
    if (!valid_drawing || !valider_drawing) {
        valider_drawing = false;
        return;
    }

    Matrix_Scale(big_head_size, big_head_size, big_head_size, MTXMODE_APPLY);
}

RECOMP_HOOK_RETURN("Player_DrawGameplay") void Player_DrawGameplayReturn() {
    valid_drawing = false;
}
