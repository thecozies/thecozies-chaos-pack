#include "cozy_chaos.h"

REGISTER_CHAOS_EFFECT(cozy_na_aim);

COMMON_FUNCS(cozy_na_aim)

CozyChaosEffect cozy_na_aim = {
    .effect = {
        .name = "N/A Aim",
        .duration = 20*20,
        .on_start_fun = on_cozy_na_aim_activate,
        .update_fun = NULL,
        .on_end_fun = on_cozy_na_aim_end,
    },
    .disturbance = CHAOS_DISTURBANCE_HIGH,
    .active = false,
    .entity = NULL,
};

typedef enum NAArrowEffectType {
    NA_ARROW_EFFECT_TYPE_CURVE,
    NA_ARROW_EFFECT_TYPE_FLACCID,
    NA_ARROW_EFFECT_TYPE_ATTACK,
    NA_ARROW_EFFECT_TYPE_COUNT
} NAArrowEffectType;
int effect_type_likelihood[NA_ARROW_EFFECT_TYPE_COUNT] = {
    [NA_ARROW_EFFECT_TYPE_CURVE] = 8,
    [NA_ARROW_EFFECT_TYPE_FLACCID] = 3,
    [NA_ARROW_EFFECT_TYPE_ATTACK] = 2,
};

int get_total_likelihood() {
    int total = 0;
    for (int i = 0; i < NA_ARROW_EFFECT_TYPE_COUNT; i++) {
        total += effect_type_likelihood[i];
    }
    return total;
}

NAArrowEffectType get_random_effect_type() {
    int total_likelihood = get_total_likelihood();
    int random_value = Rand_ZeroOne() * total_likelihood;

    for (int i = 0; i < NA_ARROW_EFFECT_TYPE_COUNT; i++) {
        random_value -= effect_type_likelihood[i];
        if (random_value < 0) {
            return i;
        }
    }
    return NA_ARROW_EFFECT_TYPE_COUNT;
}

typedef struct {
    s16 timer;
    s16 pure_timer;
    u8 state;
    NAArrowEffectType effect_type;
} MoreArrowDatas;

ActorExtensionId arrow_extension_id;

RECOMP_CALLBACK("*", recomp_on_init) void handleInits() {
    arrow_extension_id = z64recomp_extend_actor(ACTOR_EN_ARROW, sizeof(MoreArrowDatas));
}

RECOMP_HOOK("EnArrow_Init") void on_en_arrow_init(Actor* thisx, PlayState* play) {
    MoreArrowDatas* arrow_data = (MoreArrowDatas*)z64recomp_get_extended_actor_data(thisx, arrow_extension_id);
    if (arrow_data) {
        arrow_data->pure_timer = 0;
        arrow_data->effect_type = get_random_effect_type();

        switch (arrow_data->effect_type) {
            case NA_ARROW_EFFECT_TYPE_CURVE: {
                arrow_data->timer = Rand_Next() & 0xFFFF;
                break;
            }
            case NA_ARROW_EFFECT_TYPE_FLACCID:
                arrow_data->timer = 0;
                break;
            case NA_ARROW_EFFECT_TYPE_ATTACK:
                arrow_data->timer = 0;
                arrow_data->state = 0;
                break;
            default:
                arrow_data->timer = 0;
                break;
        }
    }
}

// kinda update kinda init
EnArrow* this_arrow;
void func_8088ACE0(EnArrow* this, PlayState* play);
RECOMP_HOOK("func_8088A594") void on_func_8088A594(EnArrow* this, PlayState* play) {
    this_arrow = this;
}
RECOMP_HOOK_RETURN("func_8088A594") void after_func_8088A594() {
    if (!cozy_na_aim.active) {
        return;
    }

    EnArrow* this = this_arrow;
    if (!ARROW_IS_ARROW(this->actor.params) || this->actionFunc != func_8088ACE0) {
        return;
    }
    this->unk_260 = 120;
    
    MoreArrowDatas* arrow_data = (MoreArrowDatas*)z64recomp_get_extended_actor_data(&this->actor, arrow_extension_id);
    switch (arrow_data->effect_type) {
        case NA_ARROW_EFFECT_TYPE_CURVE: {
            break;
        }
        case NA_ARROW_EFFECT_TYPE_FLACCID: {
            this->actor.world.rot.x = CLAMP(this->actor.world.rot.x, -0x3000, 0);
            this->actor.world.rot.x -= 0x1000;
            this->actor.world.rot.x = CLAMP(this->actor.world.rot.x, -0x4000, -0x1000);

            Actor_SetSpeeds(&this->actor, 10);
            this->actor.shape.rot.x = Math_Atan2S_XY(this->actor.speed, -this->actor.velocity.y);

            this->actor.shape.rot.y += Rand_Centered() * 0x800;
            this->actor.world.rot.y = this->actor.shape.rot.y;
            Actor_UpdateVelocityWithGravity(&this->actor);
            break;
        }
        case NA_ARROW_EFFECT_TYPE_ATTACK: {
            break;
        }
        default:
            break;
    }
}

enum ARROW_ATTACK_STATE {
    ARROW_ATTACK_STATE_WAIT,
    ARROW_ATTACK_STATE_SLOW_DOWN,
    ARROW_ATTACK_STATE_TURN_TO_PLAYER,
    ARROW_ATTACK_STATE_RELEASE
};

static s16 arrowPrevPitch;
static s16 arrowPrevYaw;

static u8 get_arrow_attack_type(EnArrow* this) {
    switch (this->actor.params) {
        case ARROW_TYPE_NORMAL_LIT:
            return HIT_EFFECT_FIRE;
        case ARROW_TYPE_NORMAL_HORSE:
            return HIT_EFFECT_UNK_0;
        case ARROW_TYPE_NORMAL:
            return HIT_EFFECT_UNK_0;
        case ARROW_TYPE_FIRE:
            return HIT_EFFECT_KNOCKBACK_BURN;
        case ARROW_TYPE_ICE:
            return HIT_EFFECT_FREEZE;
        case ARROW_TYPE_LIGHT:
            return HIT_EFFECT_KNOCKBACK_SHOCK;
    }
    return 0;
}

// 16 is a heart
static u8 get_arrow_damage(EnArrow* this) {
    switch (this->actor.params) {
        case ARROW_TYPE_NORMAL_LIT:
            return 8;
        case ARROW_TYPE_NORMAL_HORSE:
            return 8;
        case ARROW_TYPE_NORMAL:
            return 8;
        case ARROW_TYPE_FIRE:
            return 16;
        case ARROW_TYPE_ICE:
            return 16;
        case ARROW_TYPE_LIGHT:
            return 32;
    }
    return 8;
}

RECOMP_HOOK("func_8088ACE0") void on_func_8088ACE0(EnArrow* this, PlayState* play) {
    if (!cozy_na_aim.active) {
        this_arrow = NULL;
        return;
    }
    if (!ARROW_IS_ARROW(this->actor.params)) {
        this_arrow = NULL;
        return;
    }
    this_arrow = this;
    
    MoreArrowDatas* arrow_data = (MoreArrowDatas*)z64recomp_get_extended_actor_data(&this->actor, arrow_extension_id);
    arrow_data->timer++;

    switch (arrow_data->effect_type) {
        case NA_ARROW_EFFECT_TYPE_CURVE: {
            this->actor.shape.rot.y += Math_SinS(arrow_data->timer * 0x1800) * 0x1000;
            this->actor.world.rot.y = this->actor.shape.rot.y;
            this->actor.velocity.y += Math_SinS(arrow_data->timer * 0x1573) * 8.0f;

            this->actor.velocity.x = this->actor.speed * Math_SinS(this->actor.world.rot.y);
            this->actor.velocity.z = this->actor.speed * Math_CosS(this->actor.world.rot.y);
            break;
        }
        case NA_ARROW_EFFECT_TYPE_FLACCID: {
            if (arrow_data->pure_timer == 1) {
                Actor_PlaySfx(&this->actor, NA_SE_IT_DEKUNUTS_DROP_BOMB);
            }
            this->actor.gravity = -0.5f;
            break;
        }
        case NA_ARROW_EFFECT_TYPE_ATTACK: {
            Player* player = GET_PLAYER(play);
            switch (arrow_data->state) {
                case ARROW_ATTACK_STATE_WAIT: {
                    if (arrow_data->timer >= 1) {
                        arrow_data->state++;
                        arrow_data->timer = 0;
                    }
                    break;
                }
                case ARROW_ATTACK_STATE_SLOW_DOWN: {
                    this->actor.speed -= 150/6;
                    Actor_SetSpeeds(&this->actor, this->actor.speed * 0.75f);
                    if (this->actor.speed < 2) {
                        this->actor.speed = 0;
                        arrow_data->state++;
                        arrow_data->timer = 0;
                        this->actor.velocity.y = 0;
                    }
                    this->actor.gravity = 0;
                    this->actor.velocity.x = this->actor.speed * Math_SinS(this->actor.world.rot.y);
                    this->actor.velocity.z = this->actor.speed * Math_CosS(this->actor.world.rot.y);
                    break;
                }
                case ARROW_ATTACK_STATE_TURN_TO_PLAYER: {
                    this->actor.speed =
                        this->actor.velocity.x =
                            this->actor.velocity.y =
                                this->actor.velocity.z =
                                    this->actor.gravity = 0;
                    Vec3f linksHead;
                    linksHead.x = player->actor.world.pos.x;
                    linksHead.y = player->bodyPartsPos[PLAYER_BODYPART_HEAD].y + 3.0f;
                    linksHead.z = player->actor.world.pos.z;

                    s16 goalPitch = Math_Vec3f_Pitch(&this->actor.world.pos, &linksHead);
                    s16 goalYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &linksHead);

                    Math_SmoothStepToS(&this->actor.shape.rot.x, goalPitch, 2, 0x2000, 0x200);
                    Math_SmoothStepToS(&this->actor.shape.rot.y, goalYaw, 2, 0x2000, 0x200);

                    this->actor.world.rot.x = this->actor.shape.rot.x;
                    this->actor.world.rot.y = this->actor.shape.rot.y;
                    if (arrow_data->timer >= 15) {
                        arrow_data->state++;
                        arrow_data->timer = 0;
                        Actor_SetSpeeds(&this->actor, 150);
                    }
                    break;
                }
                case ARROW_ATTACK_STATE_RELEASE: {
                    this->actor.gravity = 0;;
                    this->collider.elem.atDmgInfo.effect = get_arrow_attack_type(this);
                    this->collider.elem.atDmgInfo.damage = get_arrow_damage(this);
                    this->collider.base.atFlags |= AT_ON | AT_TYPE_ALL;
                    CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
                    break;
                }
            }
            break;
        }
        default:
            break;
    }

    arrow_data->pure_timer++;
    arrowPrevPitch = this->actor.shape.rot.x;
    arrowPrevYaw = this->actor.shape.rot.y;
}

RECOMP_HOOK_RETURN("func_8088ACE0") void after_func_8088ACE0() {
    if (this_arrow == NULL) {
        return;
    }
    EnArrow* this = this_arrow;
    MoreArrowDatas* arrow_data = (MoreArrowDatas*)z64recomp_get_extended_actor_data(&this->actor, arrow_extension_id);
    if (arrow_data->effect_type == NA_ARROW_EFFECT_TYPE_ATTACK) {
        if (
            arrow_data->state == ARROW_ATTACK_STATE_SLOW_DOWN ||
            arrow_data->state == ARROW_ATTACK_STATE_TURN_TO_PLAYER
        ) {
            this->actor.shape.rot.x = arrowPrevPitch;
            this->actor.shape.rot.y = arrowPrevYaw;
        }
    }
}
