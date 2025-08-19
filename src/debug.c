#include "cozy_chaos.h"

RECOMP_HOOK_RETURN("Sram_OpenSave") void after_Sram_OpenSave() {
    gSaveContext.save.entrance = ENTRANCE(SOUTH_CLOCK_TOWN, 0);
    gSaveContext.save.day = 1;
    gSaveContext.save.time = CLOCK_TIME(6, 0);
    AMMO(ITEM_BOW) = 69;
    AMMO(ITEM_DEKU_STICK) = 69;
    AMMO(ITEM_DEKU_NUT) = 69;
    AMMO(ITEM_POWDER_KEG) = 69;
    AMMO(ITEM_BOMB) = 69;
    AMMO(ITEM_BOMBCHU) = 69;
}

RECOMP_HOOK("Player_UpdateCommon") void on_player_update_common_debug_input(Player* this, PlayState* play, Input* input) {
    if (this != GET_PLAYER(play)){
        return;
    }
    if (input->press.button & (D_JPAD)) {
        Vec3f spawn_pos;
        // f32 distToCam = Math3D_Dist2D(this->actor.world.pos.x, this->actor.world.pos.z,
        //                             play->mainCamera.eye.x, play->mainCamera.eye.z);
        f32 distToRedead = 100;

        Vec3f cam_pos_at_link = {play->mainCamera.eye.x, this->actor.world.pos.y, play->mainCamera.eye.z};

        Math_Vec3f_Diff(&cam_pos_at_link, &this->actor.world.pos, &spawn_pos);
        Math3D_Normalize(&spawn_pos);
        Math_Vec3f_Scale(&spawn_pos, distToRedead);
        spawn_pos.x += this->actor.world.pos.x;
        spawn_pos.y += this->actor.world.pos.y + 5;
        spawn_pos.z += this->actor.world.pos.z;
        CollisionPoly *outPoly = NULL;
        f32 floorPos = BgCheck_EntityRaycastFloor1(&play->colCtx, &outPoly, &spawn_pos);
        if (outPoly != NULL) {
            spawn_pos.y = floorPos;
            s16 redead_direction = Math_Vec3f_Yaw(&spawn_pos, &this->actor.world.pos);

            EnRd *redead = (EnRd *)Actor_Spawn(
                &play->actorCtx, play, ACTOR_EN_SKB,
                spawn_pos.x, spawn_pos.y, spawn_pos.z,
                0, redead_direction, 0, EN_RD_TYPE_REGULAR);

            if (redead != NULL) {
                // following two lines make the redead instantly scream and freeze link
                redead->actor.parent = NULL;
                redead->isMourning = true;
                cozy_sudden_redead.active = false;
            }
        }
    }
}
