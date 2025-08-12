#include "chaos_dep.h"
#include "recomputils.h"
#include "../mm-decomp/src/overlays/actors/ovl_En_Rd/z_en_rd.h"

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
// #define DISABLE_BOMB_SPAM
#define DISABLE_SUDDEN_REDEAD
// #define DISABLE_ROLLING_LINK

// #define DEBUG_SUDDEN_REDEAD

#define BOMB_SPAM_DURATION 20*12
#define SUDDEN_REDEAD_DURATION 20*1
#define ROLLING_LINK_DURATION 20*20

#ifdef DEBUG_BOMB_SPAM
bool bomb_spam_active = true;
#else
bool bomb_spam_active = false;
#endif

#ifdef DEBUG_SUDDEN_REDEAD
bool sudden_redead_active = true;
#else
bool sudden_redead_active = false;
#endif

bool rolling_link_active = false;


void noop_update_func(GraphicsContext* gfxCtx, GameState* gameState) {
}

void on_bomb_spam_activate(GraphicsContext* gfxCtx, GameState* gameState) {
    bomb_spam_active = true;
}

void on_rolling_link_activate(GraphicsContext* gfxCtx, GameState* gameState) {
    rolling_link_active = true;
    recomp_printf("Rolling Link Activated\n");
}

void on_bomb_spam_end(GraphicsContext* gfxCtx, GameState* gameState) {
#ifndef DEBUG_BOMB_SPAM
    recomp_printf("Bomb Spam Deactivated\n");
    bomb_spam_active = false;
#endif
}

void on_rolling_link_end(GraphicsContext* gfxCtx, GameState* gameState) {
    recomp_printf("Rolling Link Deactivated\n");
    rolling_link_active = false;
}

void on_sudden_redead_activate(GraphicsContext* gfxCtx, GameState* gameState) {
    recomp_printf("Sudden Redead Activated\n");
    sudden_redead_active = true;
}

void on_sudden_redead_end(GraphicsContext* gfxCtx, GameState* gameState) {
    recomp_printf("Sudden Redead Deactivated\n");
#ifndef DEBUG_SUDDEN_REDEAD
    sudden_redead_active = false;
#endif
}

RECOMP_HOOK("Player_UpdateCommon") void on_player_update_common_bombspam(Player* this, PlayState* play, Input* input) {
    if (!bomb_spam_active) {
        return;
    }
    input->cur.button &= ~(Z_TRIG | R_TRIG);
    input->press.button &= ~(Z_TRIG | R_TRIG);
    input->rel.button &= ~(Z_TRIG | R_TRIG);
}

RECOMP_HOOK("Player_UpdateCommon") void on_player_update_common_redead(Player* this, PlayState* play, Input* input) {
    if (!sudden_redead_active) {
        return;
    }
    if (this != GET_PLAYER(play)){
        return;
    }

    Vec3f spawn_pos;
    f32 distToCam = Math3D_Dist2D(this->actor.world.pos.x, this->actor.world.pos.z,
                                   play->mainCamera.eye.x, play->mainCamera.eye.z);
    f32 distToRedead = 50 + distToCam;

    Vec3f cam_pos_at_link = {play->mainCamera.eye.x, this->actor.world.pos.y, play->mainCamera.eye.z};

    Math_Vec3f_Diff(&this->actor.world.pos, &cam_pos_at_link, &spawn_pos);
    Math3D_Normalize(&spawn_pos);
    Math_Vec3f_Scale(&spawn_pos, distToRedead);
    spawn_pos.x += this->actor.world.pos.x;
    spawn_pos.y += this->actor.world.pos.y + 20;
    spawn_pos.z += this->actor.world.pos.z;
    CollisionPoly *outPoly = NULL;
    f32 floorPos = BgCheck_EntityRaycastFloor1(&play->colCtx, &outPoly, &spawn_pos);
    if (outPoly != NULL) {
        spawn_pos.y = floorPos;

        Actor_Spawn(
            &play->actorCtx, play, ACTOR_EN_RD,
            spawn_pos.x, spawn_pos.y, spawn_pos.z,
            0, 0, 0, EN_RD_TYPE_REGULAR);
        sudden_redead_active = false;
    }

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

extern s16 sControlStickWorldYaw;
extern f32 sControlStickMagnitude;

RECOMP_HOOK("Player_Action_26") void before_player_rolling(Player* this, PlayState* play) {
    if (!rolling_link_active) {
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

ChaosEffect bomb_spam = {
    .name = "Bomb Spam",
    .duration = BOMB_SPAM_DURATION,

    .on_start_fun = on_bomb_spam_activate,
    .update_fun = noop_update_func,
    .on_end_fun = on_bomb_spam_end,
};

ChaosEffect sudden_redead = {
    .name = "Sudden Redead",
    .duration = SUDDEN_REDEAD_DURATION,

    .on_start_fun = on_sudden_redead_activate,
    .update_fun = noop_update_func,
    .on_end_fun = on_sudden_redead_end,
};

ChaosEffect rolling_link = {
    .name = "Rolling Link",
    .duration = ROLLING_LINK_DURATION,

    .on_start_fun = on_rolling_link_activate,
    .update_fun = noop_update_func,
    .on_end_fun = on_rolling_link_end,
};

RECOMP_CALLBACK("mm_recomp_chaos_framework", chaos_on_init) void register_chaos_effects(void) {
    #ifndef DEBUG_BOMB_SPAM
    #ifndef DISABLE_BOMB_SPAM
    chaos_register_effect(&bomb_spam, CHAOS_DISTURBANCE_HIGH, NULL);
    #endif
    #endif

    #ifndef DISABLE_SUDDEN_REDEAD
    chaos_register_effect(&sudden_redead, CHAOS_DISTURBANCE_VERY_LOW, NULL);
    chaos_register_effect(&sudden_redead, CHAOS_DISTURBANCE_LOW, NULL);
    chaos_register_effect(&sudden_redead, CHAOS_DISTURBANCE_MEDIUM, NULL);
    chaos_register_effect(&sudden_redead, CHAOS_DISTURBANCE_HIGH, NULL);
    chaos_register_effect(&sudden_redead, CHAOS_DISTURBANCE_VERY_HIGH, NULL);
    #endif

    #ifndef DISABLE_ROLLING_LINK
    chaos_register_effect(&rolling_link, CHAOS_DISTURBANCE_HIGH, NULL);
    #endif
}
