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

void register_chaos_effect(CozyChaosEffect *cozy_effect) {
    if (!cozy_effect->fake && cozy_effect->entity == NULL) {
        cozy_effect->entity = chaos_register_effect(&cozy_effect->effect, cozy_effect->disturbance, NULL);
        recomp_printf("Registered effect: %s\n", cozy_effect->effect.name);
    }
}
