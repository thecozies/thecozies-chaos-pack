#include "cozy_chaos.h"

REGISTER_CHAOS_EFFECT(cozy_top_down_zelda);

COMMON_FUNCS(top_down_zelda, cozy_top_down_zelda, "Top Down Zelda");

static bool init_cam = true;
Player* player_for_camera = NULL;
Vec3f last_player_pos = {0,0,0};
static f32 fog_near = 360;

static f32 ortho_cam_dist = 800;
static f32 g_far = 700;

bool should_store_saved_view_properties = false;
bool should_restore_saved_view_properties = false;

struct saved_properties {
    s16 fogNear;
    s16 fogFar;
};
struct saved_properties saved_view_props = {0};

void store_saved_view_properties(View* view, PlayState *play) {
    saved_view_props.fogNear = play->lightCtx.fogNear;
    saved_view_props.fogFar = play->lightCtx.zFar;
}
void restore_saved_view_properties(View* view, PlayState *play) {
    play->lightCtx.fogNear = saved_view_props.fogNear;
    play->lightCtx.zFar = saved_view_props.fogFar;
    should_restore_saved_view_properties = false;
}

void on_top_down_zelda_start(struct PlayState *play) {
    on_top_down_zelda_activate(play);
    init_cam = true;
    should_store_saved_view_properties = true;
    should_restore_saved_view_properties = true;
}

CozyChaosEffect cozy_top_down_zelda = {
    .effect = {
        .name = "Top Down Zelda",
        .duration = 20*30,
        .on_start_fun = on_top_down_zelda_start,
        .update_fun = NULL,
        .on_end_fun = on_top_down_zelda_end,
    },
    // .disturbance = CHAOS_DISTURBANCE_MEDIUM,
    .disturbance = CHAOS_DISTURBANCE_LOW,
    .active = false,
    .entity = NULL,
};

RECOMP_HOOK_RETURN("Player_Init") void on_Player_Init() {
    init_cam = true;
}

RECOMP_HOOK("Player_Update") void on_player_update_common_ortho(Actor* thisx, PlayState* play) {
    if (!cozy_top_down_zelda.active) {
        return;
    }
    GET_ACTIVE_CAM(play)->inputDir.y = 0x8000;
}
void after_player_update_common_ortho(Player* this, PlayState* play) {
    player_for_camera = this;
    last_player_pos = this->actor.world.pos;

    CollisionPoly *outPoly = NULL;
    s32 bgId;
    f32 outY;
    if (!BgCheck_EntityCheckCeiling(&play->colCtx, &outY, &this->actor.world.pos, 1500 + 5, &outPoly, &bgId, NULL)) {
        Math_ApproachF(&ortho_cam_dist, 1500, 0.25f, 100.0f);
        g_far = ortho_cam_dist * 4;
    } else {
        outY -= this->actor.world.pos.y;
        f32 temp_dist = MIN(outY - 5, 1500);
        temp_dist = MAX(temp_dist, 50);
        Math_ApproachF(&ortho_cam_dist, temp_dist, 0.25f, 100.0f);
        g_far = ortho_cam_dist * 4;
    }

}

PLAYER_UPDATE_RETURN_HOOK(top_down_zelda, after_player_update_common_ortho);

RECOMP_HOOK_RETURN("Player_Destroy") void on_player_destroy_ortho() {
    player_for_camera = NULL;
}

View* mainview = NULL;
PlayState* main_play_state = NULL;
RECOMP_HOOK("Play_DrawMain") void on_Play_DrawMain(PlayState* this) {
    if (!cozy_top_down_zelda.active) {
        if (should_restore_saved_view_properties) {
            restore_saved_view_properties(&this->view, this);
        }
        mainview = NULL;
        return;
    }
    mainview = &this->view;
    main_play_state = this;
    if (should_store_saved_view_properties) {
        store_saved_view_properties(mainview, this);
        should_store_saved_view_properties = false;
    }
}

extern void View_ViewportToVp(Vp* dest, Viewport* src);
extern void View_ApplyLetterbox(View* view);
extern s32 View_StepDistortion(View* view, Mtx* projectionMtx);
extern s32 Actor_CullingVolumeTest(PlayState* play, Actor* actor, Vec3f* projPos, f32 projW);
RECOMP_PATCH s32 Actor_CullingCheck(PlayState* play, Actor* actor) {
    s32 is_main_view = cozy_top_down_zelda.active && (&play->view == mainview);
    if (is_main_view) {
        if (actor->projectedPos.x + actor->cullingVolumeScale > gScreenWidth * -16.0f &&
            actor->projectedPos.x - actor->cullingVolumeScale < gScreenWidth * 16.0f &&
            actor->projectedPos.y + actor->cullingVolumeScale > gScreenHeight * -16.0f &&
            actor->projectedPos.y - actor->cullingVolumeScale < gScreenHeight * 16.0f
        ) {
            return true;
        }

        return false;
    }
    return Actor_CullingVolumeTest(play, actor, &actor->projectedPos, actor->projectedW);
}
RECOMP_HOOK("Room_DrawCullable") void Room_DrawCullable(PlayState* play, Room* room, u32 flags) {
    static s8 last_unk78 = 0;
    static bool was_active = false;
    if (cozy_top_down_zelda.active) {
        if (!was_active) {
            last_unk78 = play->roomCtx.unk78;
        }
        play->roomCtx.unk78 = -1;
        was_active = true;
        return;
    } 
    if (was_active) {
        play->roomCtx.unk78 = last_unk78;
        was_active = false;
    }
}


RECOMP_PATCH s32 View_ApplyPerspective(View* view) {
    static Vec3f camPos;
    static bool init_cam = true;

    f32 aspect;
    s32 width;
    s32 height;
    Vp* vp;
    Mtx* projection;
    Mtx* viewing;
    GraphicsContext* gfxCtx = view->gfxCtx;

    s32 is_main_view = (view == mainview);

    OPEN_DISPS(gfxCtx);

    // Viewport
    vp = GRAPH_ALLOC(gfxCtx, sizeof(Vp));
    View_ViewportToVp(vp, &view->viewport);
    view->vp = *vp;

    View_ApplyLetterbox(view);

    gSPViewport(POLY_OPA_DISP++, vp);
    gSPViewport(POLY_XLU_DISP++, vp);

    // Perspective projection
    projection = GRAPH_ALLOC(gfxCtx, sizeof(Mtx));
    view->projectionPtr = projection;

    width = view->viewport.rightX - view->viewport.leftX;
    height = view->viewport.bottomY - view->viewport.topY;
    aspect = (f32)width / (f32)height;

    if (is_main_view && cozy_top_down_zelda.active && player_for_camera != NULL) {
        main_play_state->lightCtx.fogNear = fog_near;
        main_play_state->lightCtx.zFar = g_far;

        gEXSetNearClipping(POLY_OPA_DISP++, true);
        gEXSetNearClipping(POLY_XLU_DISP++, true);
        gEXSetNearClipping(OVERLAY_DISP++, false);

        guOrtho(projection, gScreenWidth * -1.0f, gScreenWidth * 1.0f, gScreenHeight * -1.0f, gScreenHeight * 1.0f,
            0, g_far, 4096);
            // view->zNear, view->zFar, 4096);

        gSPPerspNormalize(POLY_OPA_DISP++, 0xFFFF);
        gSPPerspNormalize(POLY_XLU_DISP++, 0xFFFF);
        if (player_for_camera != NULL) {
            Vec3f regionPosition = { last_player_pos.x, last_player_pos.y, last_player_pos.z };
            // snap region to multiples of ortho width/height
            s32 regionWidth = gScreenWidth * 2;
            s32 regionHeight = gScreenHeight * 2;

            regionPosition.x = Math_FFloorF(regionPosition.x / regionWidth) * regionWidth;
            regionPosition.x += gScreenWidth;
            regionPosition.z = Math_FFloorF(regionPosition.z / regionHeight) * regionHeight;
            regionPosition.z += gScreenHeight;

            if (init_cam) {
                camPos = regionPosition;
                init_cam = false;
            } else {
                s32 xRate = regionWidth / 10;
                s32 zRate = regionHeight / 10;
                if (camPos.x < regionPosition.x) {
                    camPos.x += xRate;
                    if (camPos.x > regionPosition.x) {
                        camPos.x = regionPosition.x;
                    }
                }
                if (camPos.x > regionPosition.x) {
                    camPos.x -= xRate;
                    if (camPos.x < regionPosition.x) {
                        camPos.x = regionPosition.x;
                    }
                }

                camPos.y = regionPosition.y;

                if (camPos.z < regionPosition.z) {
                    camPos.z += zRate;
                    if (camPos.z > regionPosition.z) {
                        camPos.z = regionPosition.z;
                    }
                }
                if (camPos.z > regionPosition.z) {
                    camPos.z -= zRate;
                    if (camPos.z < regionPosition.z) {
                        camPos.z = regionPosition.z;
                    }
                }
            }

            view->eye.x = camPos.x;
            view->eye.z = camPos.z;
            view->eye.y = camPos.y + ortho_cam_dist;
            view->at.x = camPos.x;
            view->at.z = camPos.z;
            view->at.y = camPos.y;
            view->up.x = 0;
            view->up.y = 0;
            view->up.z = -1;
        }
    } else {
        gEXSetNearClipping(POLY_OPA_DISP++, false);
        gEXSetNearClipping(POLY_XLU_DISP++, false);
        gEXSetNearClipping(OVERLAY_DISP++, false);
        guPerspective(projection, &view->perspNorm, view->fovy, aspect, view->zNear, view->zFar, view->scale);
        gSPPerspNormalize(POLY_OPA_DISP++, view->perspNorm);
        gSPPerspNormalize(POLY_XLU_DISP++, view->perspNorm);
        View_StepDistortion(view, projection);
    }
    view->projection = *projection;

    gSPMatrix(POLY_OPA_DISP++, projection, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);
    gSPMatrix(POLY_XLU_DISP++, projection, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);

    // View matrix (look-at)
    viewing = GRAPH_ALLOC(gfxCtx, sizeof(Mtx));
    view->viewingPtr = viewing;

    if ((view->eye.x == view->at.x) && (view->eye.y == view->at.y) && (view->eye.z == view->at.z)) {
        view->eye.z += 2.0f;
    }

    guLookAt(viewing, view->eye.x, view->eye.y, view->eye.z, view->at.x, view->at.y, view->at.z, view->up.x, view->up.y,
             view->up.z);

    view->viewing = *viewing;

    gSPMatrix(POLY_OPA_DISP++, viewing, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);
    gSPMatrix(POLY_XLU_DISP++, viewing, G_MTX_NOPUSH | G_MTX_MUL | G_MTX_PROJECTION);

    CLOSE_DISPS(gfxCtx);

    return 1;
}

RECOMP_HOOK_RETURN("Play_DrawMain") void after_Play_DrawMain() {
    mainview = NULL;
}
