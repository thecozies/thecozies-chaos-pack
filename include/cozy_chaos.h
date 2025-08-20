#pragma once

#include "chaos_dep.h"
#include "recomputils.h"
#include "rt64_extended_gbi.h"
#include "cozy_effect_externs.h"

/**
 * Wrapper around `ChaosEffect` that adds some convenience.
 * `active` state is handled automatically if you use the `COMMON_FUNCS` macro,
 * otherwise must be handled manually.
 * Example:
 ```c
 CozyChaosEffect example_effect = {
    .effect = {
        .name = "Example Effect",
        .duration = 20*12, // 12 seconds (20fps * 12)
        .on_start_fun = on_example_effect_activate, // Runs when the effect is activated
        .update_fun = on_effect_update, // Runs every frame the effect is active
        .on_end_fun = on_example_effect_end, // Runs when the effect is deactivated
    },
    .disturbance = CHAOS_DISTURBANCE_VERY_LOW, // Sets the disturbance level
};
 ```
 */
typedef struct CozyChaosEffect {
    ChaosEffect effect;
    ChaosDisturbance disturbance;
    bool active;
    ChaosEffectEntity* entity;
    bool fake;
} CozyChaosEffect;

// Do not use directly. Use `REGISTER_CHAOS_EFFECT` macro with your effect.
void register_chaos_effect(CozyChaosEffect *cozy_effect);

/*
### Required
    Registers your effect.
    Put this at the top of your effect's file.
#### Example
```c
REGISTER_CHAOS_EFFECT(effect_variable_name);
```
#### Note:
    You must include the effect file in `src/effect_includes.c`
*/
#define REGISTER_CHAOS_EFFECT(cozy_effect) \
extern CozyChaosEffect cozy_effect; \
RECOMP_CALLBACK("mm_recomp_chaos_framework", chaos_on_init) void register_chaos_effect_##cozy_effect(void) { \
    register_chaos_effect(&cozy_effect); \
}

// The following macros are helpers. They handle setting CozyChaosEffect.active to true/false when your effect starts/ends.
// If you don't use these you will need to manage the active state manually if you want to track CozyChaosEffect.active in your patches
// These also print when the effect is activated or deactivated.

// Creates an activation function for an effect when it starts, sets `<CozyChaosEffect>.active` to `true`
#define COMMON_ACTIVATE_FUNC(cozy_effect) \
    void on_##cozy_effect##_activate(struct PlayState *play) { \
        recomp_printf("Activated: '%s'\n", cozy_effect.effect.name); \
        cozy_effect.active = true; \
    }
// Creates a deactivation function for an effect when it completes, sets `<CozyChaosEffect>.active` to `false`
#define COMMON_END_FUNC(cozy_effect) \
    void on_##cozy_effect##_end(struct PlayState *play) { \
        recomp_printf("Deactivated: '%s'\n", cozy_effect.effect.name); \
        cozy_effect.active = false; \
    }

/**
 * @brief Creates both an activation and deactivation function for the effect.
 * @param cozy_effect The `CozyChaosEffect` variable name.
 */
#define COMMON_FUNCS(cozy_effect) \
    COMMON_ACTIVATE_FUNC(cozy_effect) \
    COMMON_END_FUNC(cozy_effect)

typedef struct PlayerUpdateCache {
    Player *player;
    PlayState *play;
    bool is_valid;
} PlayerUpdateCache;

extern PlayerUpdateCache player_update_cache;

/**
 * @brief Creates a return hook that runs after Player_Update, giving you the Player* and PlayState*. 
 * @attention Does NOT check if your effect is active.
 * @param effect_code_name can just be the variable name of your effect.
 * @param player_update_callback The function to call with (Player* this, PlayState* play) parameters.
 */
#define PLAYER_UPDATE_RETURN_HOOK(effect_code_name, player_update_callback) \
    RECOMP_HOOK_RETURN("Player_Update") void post_player_update_##effect_code_name() { \
        if (!player_update_cache.is_valid) return; \
        Player* this = player_update_cache.player; \
        PlayState* play = player_update_cache.play; \
        { player_update_callback(this, play); }; \
    }
