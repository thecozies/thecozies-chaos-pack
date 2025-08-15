#pragma once

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

extern s16 sControlStickWorldYaw;
extern f32 sControlStickMagnitude;


#include "../mm-decomp/src/overlays/actors/ovl_En_Rd/z_en_rd.h"
#include "../mm-decomp/src/overlays/actors/ovl_En_Arrow/z_en_arrow.h"

