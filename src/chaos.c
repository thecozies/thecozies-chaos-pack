#include "chaos_dep.h"
#include "recomputils.h"

extern void Player_UseItem(PlayState* play, Player* this, ItemId item);
extern void Player_DestroyHookshot(Player* this);
extern void Player_DetachHeldActor(PlayState* play, Player* this);
extern void Player_InitItemActionWithAnim(PlayState* play, Player* this, PlayerItemAction itemAction);
extern void Player_Anim_PlayOnce(PlayState* play, Player* this, PlayerAnimationHeader* anim);
extern void Player_Action_39(Player* this, PlayState* play);
extern s32 Player_SetAction(PlayState* play, Player* this, PlayerActionFunc actionFunc, s32 arg3);
extern void func_8083D6DC(Player* this, PlayState* play);
extern void func_808409A8(PlayState* play, Player* this, f32 speed, f32 yVelocity);
extern PlayerItemAction Player_ItemToItemAction(Player* this, ItemId item);

extern s32 sPlayerUseHeldItem;
extern s32 sPlayerHeldItemButtonIsHeldDown;
extern PlayerAnimationHeader gPlayerAnim_link_silver_throw;

// #define DEBUG_BOMB_SPAM
#define BOMB_SPAM_DURATION 20*10

#ifndef DEBUG_BOMB_SPAM
bool bomb_spam_active = false;
#else
bool bomb_spam_active = true;
#endif

void on_bomb_spam_activate(GraphicsContext* gfxCtx, GameState* gameState) {
    bomb_spam_active = true;
}

void noop_update_func(GraphicsContext* gfxCtx, GameState* gameState) {
}

void on_bomb_spam_end(GraphicsContext* gfxCtx, GameState* gameState) {
#ifndef DEBUG_BOMB_SPAM
    bomb_spam_active = false;
#endif
}

RECOMP_HOOK("Player_UpdateCommon") void on_player_update_common(Player* this, PlayState* play, Input* input) {
    if (!bomb_spam_active) {
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

    if (!bomb_spam_active) {
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

ChaosEffect bomb_spam = {
    .name = "Bomb Spam",
    .duration = BOMB_SPAM_DURATION,

    .on_start_fun = on_bomb_spam_activate,
    .update_fun = noop_update_func,
    .on_end_fun = on_bomb_spam_end,
};

RECOMP_CALLBACK("mm_recomp_chaos_framework", chaos_on_init) void register_chaos_effects(void) {
    chaos_register_effect(&bomb_spam, CHAOS_DISTURBANCE_VERY_LOW, NULL);
}
