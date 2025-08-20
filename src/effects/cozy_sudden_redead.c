#include "cozy_chaos.h"

REGISTER_CHAOS_EFFECT(cozy_sudden_redead);

COMMON_ACTIVATE_FUNC(sudden_redead, cozy_sudden_redead, "Sudden Redead");

CozyChaosEffect cozy_sudden_redead = {
    .effect = {
        .name = "Sudden Redead",
        .duration = 1,
        .on_start_fun = on_sudden_redead_activate,
        .update_fun = NULL,
        .on_end_fun = NULL,
    },
    .disturbance = CHAOS_DISTURBANCE_MEDIUM,
    .active = false,
    .entity = NULL,
};

void on_try_spawn_redead(Player* this, PlayState* play) {
    if (!cozy_sudden_redead.active) {
        return;
    }

    Vec3f spawn_pos;
    f32 distToCam = Math3D_Dist2D(this->actor.world.pos.x, this->actor.world.pos.z,
                                   play->mainCamera.eye.x, play->mainCamera.eye.z);
    f32 distToRedead = 50 + distToCam;

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
            &play->actorCtx, play, ACTOR_EN_RD,
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

PLAYER_UPDATE_RETURN_HOOK(sudden_redead, on_try_spawn_redead);
