#pragma once

#include "chaos_dep.h"
#include "recomputils.h"
#include "rt64_extended_gbi.h"
#include "cozy_effect_externs.h"

typedef struct CozyChaosEffect {
    ChaosEffect effect;
    ChaosDisturbance disturbance;
    bool active;
    ChaosEffectEntity* entity;
    bool fake;
} CozyChaosEffect;

#define COMMON_ACTIVATE_FUNC(effect_code_name, effect, effect_name) \
    void on_##effect_code_name##_activate(struct PlayState *play) { \
        recomp_printf(#effect_name " Activated\n"); \
        effect.active = true; \
    }

#define COMMON_END_FUNC(effect_code_name, effect, effect_name) \
    void on_##effect_code_name##_end(struct PlayState *play) { \
        recomp_printf(#effect_name " Deactivated\n"); \
        effect.active = false; \
    }

#define COMMON_FUNCS(effect_code_name, effect, effect_name) \
    COMMON_ACTIVATE_FUNC(effect_code_name, effect, effect_name) \
    COMMON_END_FUNC(effect_code_name, effect, effect_name)

typedef struct PlayerUpdateCache {
    Player *player;
    PlayState *play;
    bool is_valid;
} PlayerUpdateCache;

extern PlayerUpdateCache player_update_cache;

#define PLAYER_UPDATE_RETURN_HOOK(effect_code_name, player_update_callback) \
    RECOMP_HOOK_RETURN("Player_Update") void post_player_update_##effect_code_name() { \
        if (!player_update_cache.is_valid) return; \
        Player* this = player_update_cache.player; \
        PlayState* play = player_update_cache.play; \
        { player_update_callback(this, play); }; \
    }

void register_chaos_effect(CozyChaosEffect *cozy_effect);

#define REGISTER_CHAOS_EFFECT(cozy_effect) \
extern CozyChaosEffect cozy_effect; \
RECOMP_CALLBACK("mm_recomp_chaos_framework", chaos_on_init) void register_chaos_effect_##cozy_effect(void) { \
    register_chaos_effect(&cozy_effect); \
}
