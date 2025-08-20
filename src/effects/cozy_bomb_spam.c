#include "cozy_chaos.h"

REGISTER_CHAOS_EFFECT(cozy_bomb_spam);

COMMON_FUNCS(cozy_bomb_spam)

CozyChaosEffect cozy_bomb_spam = {
    .effect = {
        .name = "Bomb Spam",
        .duration = 20*12,
        .on_start_fun = on_cozy_bomb_spam_activate,
        .update_fun = NULL,
        .on_end_fun = on_cozy_bomb_spam_end,
    },
    .disturbance = CHAOS_DISTURBANCE_MEDIUM,
    .active = false,
    .entity = NULL,
};


RECOMP_HOOK("Player_UpdateCommon") void on_player_update_common_bombspam(Player* this, PlayState* play, Input* input) {
    if (this != GET_PLAYER(play)){
        return;
    }
    if (!cozy_bomb_spam.active) {
        return;
    }
    input->cur.button &= ~(Z_TRIG | R_TRIG);
    input->press.button &= ~(Z_TRIG | R_TRIG);
    input->rel.button &= ~(Z_TRIG | R_TRIG);
}

RECOMP_HOOK("Player_ProcessItemButtons") void Player_ProcessItemButtons(Player* this, PlayState* play) {
    static ItemId bomb_beat[] = {ITEM_BOMB,0,ITEM_BOMB,0,ITEM_BOMBCHU,0,0,ITEM_BOMB,0,ITEM_BOMB,0,ITEM_BOMB,ITEM_BOMB,0,ITEM_BOMB,0};
    static int bomb_beat_index = 0;
    static int num_funny_bombs = 0;
    static int bomb_beat_index_inc = 0;

    static ItemId item = ITEM_BOMB;
    
    static bool spawn_keg = false;
    static bool do_throw = true;

    if (!cozy_bomb_spam.active) {
        bomb_beat_index = 0;
        bomb_beat_index_inc = 0;
        do_throw = true;
        item = ITEM_BOMB;
        return;
    }
    
    if (this->stateFlags1 & (PLAYER_STATE1_CARRYING_ACTOR | PLAYER_STATE1_20000000)) {
        func_808409A8(play, this, this->speedXZ + 8.0f, 12.0f);
        return;
    }
    if (this->stateFlags2 & PLAYER_STATE2_2000000) {
        return;
    }
    if (this->stateFlags3 & PLAYER_STATE3_20000000) {
        return;
    }
    if (func_801240DC(this)) {
        return;
    }
    
    if (!do_throw) {
        bomb_beat_index_inc++;
        bomb_beat_index_inc %= 2;
        if (bomb_beat_index_inc == 0) {
            if (bomb_beat[bomb_beat_index]) {
                do_throw = true;
                item = bomb_beat[bomb_beat_index];
            }
    
            bomb_beat_index++;
            bomb_beat_index %= 16;
    
            if (num_funny_bombs++ == 16*32) {
                spawn_keg = true;
                num_funny_bombs = 0;
            }
        }
    }

    if (spawn_keg) {
        item = ITEM_POWDER_KEG;
    }

    PlayerItemAction itemAction = Player_ItemToItemAction(this, item);
    if (itemAction != this->heldItemAction && do_throw) {
        this->nextModelGroup = Player_ActionToModelGroup(this, itemAction);
        Player_DestroyHookshot(this);
        Player_DetachHeldActor(play, this);
        Player_InitItemActionWithAnim(play, this, itemAction);
        sPlayerUseHeldItem = true;
        sPlayerHeldItemButtonIsHeldDown = true;
        this->heldItemId = item;
        this->stateFlags3 |= PLAYER_STATE3_START_CHANGING_HELD_ITEM;
        if (itemAction == PLAYER_IA_POWDER_KEG) {
            spawn_keg = false;
        }
        do_throw = false;
    }
}
