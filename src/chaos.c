#include "cozy_chaos.h"

PlayerUpdateCache player_update_cache;
RECOMP_HOOK("Player_Update") void on_player_update_store_cache(Actor* thisx, PlayState* play) {
    Player* this = (Player*)thisx;
    if (this == GET_PLAYER(play)) {
        player_update_cache.player = this;
        player_update_cache.play = play;
        player_update_cache.is_valid = true;
    } else {
        player_update_cache.is_valid = false;
    }
}

struct CozyChaosEffect *all_effects[] = {
    &cozy_bomb_spam,
    &cozy_sudden_redead,
    &cozy_rolling_link,
    &cozy_big_head,
    &cozy_na_aim,
    &cozy_top_down_zelda,
};


RECOMP_CALLBACK("mm_recomp_chaos_framework", chaos_on_init) void register_chaos_effects(void) {
    for (int i = 0; i < ARRAY_COUNT(all_effects); i++) {
        CozyChaosEffect *cozy_effect = all_effects[i];
        if (!cozy_effect->fake && cozy_effect->entity == NULL) {
            cozy_effect->entity = chaos_register_effect(&cozy_effect->effect, cozy_effect->disturbance, NULL);
            recomp_printf("Registered cozy effect: %s\n", cozy_effect->effect.name);
        }
    }
}
