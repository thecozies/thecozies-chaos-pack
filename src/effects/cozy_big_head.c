#include "cozy_chaos.h"

REGISTER_CHAOS_EFFECT(cozy_big_head);
COMMON_FUNCS(cozy_big_head);

#define BIG_HEAD_DURATION 20*20
#define BIG_HEAD_MIN_SIZE 1
#define BIG_HEAD_MAX_SIZE 2.5f
s32 big_head_counter = 0;
f32 big_head_size = BIG_HEAD_MIN_SIZE;
f32 big_head_size_vel = 0;
void on_big_head_activate(struct PlayState *play);
void on_big_head_update(struct PlayState *play);
void on_big_head_end(struct PlayState *play);

CozyChaosEffect cozy_big_head = {
    .effect = {
        .name = "Big Head",
        .duration = BIG_HEAD_DURATION,
        .on_start_fun = on_big_head_activate,
        .update_fun = on_big_head_update,
        .on_end_fun = on_cozy_big_head_end,
    },
    .disturbance = CHAOS_DISTURBANCE_LOW,
    .active = false,
    .entity = NULL,
};

void on_big_head_activate(struct PlayState *play) {
    on_cozy_big_head_activate(play);
    big_head_counter = 0;
    big_head_size = BIG_HEAD_MIN_SIZE;
    big_head_size_vel = -2.0f;
}

f32 approach_f32_asymptotic(f32 current, f32 target, f32 multiplier) {
    return (current + ((target - current) * multiplier));
}
void elastic_approach(f32 *cur, f32 *curVel, f32 goal, f32 speed, f32 maxSpeed) {
    f32 diff = goal - *cur;
    *curVel = approach_f32_asymptotic(*curVel, diff, speed);
    *curVel = CLAMP(*curVel, -maxSpeed, maxSpeed);
    *cur = *cur + *curVel;
}

void on_big_head_update(struct PlayState *play) {
    // f32 sine_size = Math_SinS(big_head_counter * 0x900) * 0.5f + 0.5f;
    // sine_size += Math_SinS(big_head_counter * 0xA00) * 0.5f + 0.5f;
    // sine_size += Math_SinS(big_head_counter * 0xE00) * 0.5f + 0.5f;
    // sine_size += Math_SinS(big_head_counter * 0x1110) * 0.5f + 0.5f;
    // sine_size /= 4.0f;
    // f32 frames_from_half = ABS((BIG_HEAD_DURATION / 2.0f) - (f32)big_head_counter);
    // f32 from_half_frac = 1.0f - (frames_from_half / (BIG_HEAD_DURATION / 2.0f));
    // big_head_size = sine_size * from_half_frac * (BIG_HEAD_MAX_SIZE - BIG_HEAD_MIN_SIZE) + BIG_HEAD_MIN_SIZE;
    f32 goal_size = BIG_HEAD_MAX_SIZE;
    if (big_head_counter > BIG_HEAD_DURATION - 60) {
        goal_size = BIG_HEAD_MIN_SIZE;
    }

    elastic_approach(&big_head_size, &big_head_size_vel, goal_size, 0.12f, 10.0f);
    big_head_size = CLAMP(big_head_size, 0.75f, 4.0f);

    big_head_counter++;
    if (big_head_counter >= BIG_HEAD_DURATION) {
        big_head_counter = BIG_HEAD_DURATION;
        cozy_big_head.active = false;
    }
}

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
