#include "antiaim.h"

#include "ragebot.h"
#include "../settings.h"
#include "../Hooks/hooks.h"
#include "../Utils/math.h"
#include "../Utils/entity.h"
#include "../interfaces.h"

static float lbyBreak = 180;

float AntiAim::GetMaxDelta(CCSGOAnimState *animState)
{

    float speedFraction = std::max(0.0f, std::min(animState->feetShuffleSpeed, 1.0f));

    float speedFactor = std::max(0.0f, std::min(1.0f, animState->feetShuffleSpeed2));

    float unk1 = ((animState->runningAccelProgress * -0.30000001) - 0.19999999) * speedFraction;
    float unk2 = unk1 + 1.0f;
    float delta;

    if (animState->duckProgress > 0)
    {
	unk2 += ((animState->duckProgress * speedFactor) * (0.5f - unk2)); // - 1.f
    }

    delta = *(float *)((uintptr_t)animState + 0x3A4) * unk2;

    return delta - 0.5f;
}

// Pasted from space!hook, but I tried
static bool GetBestHeadAngle(QAngle &angle, float delta, bool bSend)
{
    float b, r, l;

    Vector src3D, dst3D, forward, right, up;

    trace_t tr;
    Ray_t ray, ray2, ray3;
    CTraceFilter filter;

    C_BasePlayer *localplayer = (C_BasePlayer *)entityList->GetClientEntity(engine->GetLocalPlayer());
    if (!localplayer)
	return false;

    QAngle viewAngles;
    engine->GetViewAngles(viewAngles);

    viewAngles.x = 0;

    Math::AngleVectors(viewAngles, forward, right, up);

    auto GetTargetEntity = [&](void) {
	float bestFov = FLT_MAX;
	C_BasePlayer *bestTarget = NULL;

	for (int i = 0; i < engine->GetMaxClients(); ++i)
	{
	    C_BasePlayer *player = (C_BasePlayer *)entityList->GetClientEntity(i);

	    if (!player || player == localplayer || player->GetDormant() || !player->GetAlive() || player->GetImmune() || player->GetTeam() == localplayer->GetTeam())
		continue;

	    float fov = Math::GetFov(viewAngles, Math::CalcAngle(localplayer->GetEyePosition(), player->GetEyePosition()));

	    if (fov < bestFov)
	    {
		bestFov = fov;
		bestTarget = player;
	    }
	}

	return bestTarget;
    };

    auto target = GetTargetEntity();
    filter.pSkip = localplayer;
    src3D = localplayer->GetEyePosition();
    dst3D = src3D + (forward * 384);

    if (!target)
	return false;

    ray.Init(src3D, dst3D);
    trace->TraceRay(ray, MASK_SHOT, &filter, &tr);
    b = (tr.endpos - tr.startpos).Length();

    ray2.Init(src3D + right * 35, dst3D + right * 35);
    trace->TraceRay(ray2, MASK_SHOT, &filter, &tr);
    r = (tr.endpos - tr.startpos).Length();

    ray3.Init(src3D - right * 35, dst3D - right * 35);
    trace->TraceRay(ray3, MASK_SHOT, &filter, &tr);
    l = (tr.endpos - tr.startpos).Length();

    if (b < r && b < l && l == r) // Left = Right > Back
	return true;

    if (b > r && b > l) // Back
	angle.y -= bSend ? 180 + delta : 180;
    else if (r > l && r > b) // Right
    {
	angle.y += bSend ? 90 + delta : 90;
	lbyBreak = 90 + delta;
    }
    else if (r > l && r == b) // Right = Back
    {
	angle.y += bSend ? 135 + delta : 135;
	lbyBreak = 135 + delta;
    }
    else if (l > r && l > b) // Left
    {
	angle.y -= bSend ? 90 + delta : 90;
	lbyBreak = 90 - delta;
    }
    else if (l > r && l == b) // Left = Back
    {
	angle.y -= bSend ? 135 + delta : 135;
	lbyBreak = 135 - delta;
    }
    else
	return false;

    return true;
}

static void DoAntiAim(AntiAimType type, float delta, QAngle &angle, bool bSend, CCSGOAnimState *animState, bool directionSwitch, C_BasePlayer *localplayer)
{
    if (Settings::AntiAim::States::enabled)
    {
	if (localplayer->GetVelocity().Length() <= 0.0f)
	    type = Settings::AntiAim::States::Stand::type;
	else if (!(localplayer->GetFlags() & FL_ONGROUND))
	    type = Settings::AntiAim::States::Air::type;
	else
	    type = Settings::AntiAim::States::Run::type;
    }
    else
	type = Settings::AntiAim::type;

    switch (type)
    {
    case AntiAimType::RAGE:
    {
	static bool yFlip = false;

	angle.x = 89.0f;
	if (yFlip)
	    angle.y += directionSwitch ? delta / 4 : -delta / 4;
	else
	    angle.y += directionSwitch ? -delta / 4 + 180.0f : delta / 4 + 180.0f;

	if (!bSend)
	{
	    if (yFlip)
		angle.y += directionSwitch ? -delta : delta;
	    else
		angle.y += directionSwitch ? delta : -delta;
	}
	else
	    yFlip = !yFlip;
    }
	break;

    case AntiAimType::LEGIT:
    {
	if (!bSend)
	    angle.y += directionSwitch ? delta : -delta;
    }
	break;

    case AntiAimType::CUSTOM:
    {
	angle.x = 89.0f;

	if (Settings::AntiAim::States::enabled)
	{
	    if (localplayer->GetVelocity().Length() <= 0.0f)
		angle.y += Settings::AntiAim::States::Stand::angle;
	    else if (!(localplayer->GetFlags() & FL_ONGROUND))
		angle.y += Settings::AntiAim::States::Air::angle;
	    else
		angle.y += Settings::AntiAim::States::Run::angle;
	}
	else
	    angle.y += Settings::AntiAim::yaw;

	if (!bSend)
	    angle.y += directionSwitch ? delta : -delta;
    }
	break;

    case AntiAimType::FREESTAND:
    {
	angle.x = 89.0f;

	QAngle edge_angle = angle;
	if (GetBestHeadAngle(edge_angle, delta, bSend))
	    angle.y = edge_angle.y;
	else
	    angle.y += bSend ? 180 + delta : 180;
    }

    default:
	break;
    }
}

void AntiAim::CreateMove(CUserCmd *cmd)
{
    if (!Settings::AntiAim::enabled)
	return;

    QAngle oldAngle = cmd->viewangles;
    float oldForward = cmd->forwardmove;
    float oldSideMove = cmd->sidemove;

    QAngle angle = cmd->viewangles;

    C_BasePlayer *localplayer = (C_BasePlayer *)entityList->GetClientEntity(engine->GetLocalPlayer());
    if (!localplayer || !localplayer->GetAlive())
	return;

    C_BaseCombatWeapon *activeWeapon = (C_BaseCombatWeapon *)entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
    if (!activeWeapon)
	return;

    if (activeWeapon->GetCSWpnData()->GetWeaponType() == CSWeaponType::WEAPONTYPE_GRENADE)
    {
	C_BaseCSGrenade *csGrenade = (C_BaseCSGrenade *)activeWeapon;

	if (csGrenade->GetThrowTime() > 0.f)
	    return;
    }

    if (cmd->buttons & IN_USE || cmd->buttons & IN_ATTACK || (cmd->buttons & IN_ATTACK2 && *activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER))
	return;

    if (localplayer->GetMoveType() == MOVETYPE_LADDER || localplayer->GetMoveType() == MOVETYPE_NOCLIP)
	return;

    if (Settings::AntiAim::AutoDisable::knifeHeld && localplayer->GetAlive() && activeWeapon->GetCSWpnData()->GetWeaponType() == CSWeaponType::WEAPONTYPE_KNIFE)
	return;

    static bool bSend = true;
    bSend = (inputSystem->IsButtonDown(Settings::FakeDuck::key) && Settings::FakeDuck::enabled) || Settings::FakeLag::enabled ? CreateMove::sendPacket : !bSend;

    static bool directionSwitch = false;

    if (inputSystem->IsButtonDown(Settings::AntiAim::left))
	directionSwitch = true;
    else if (inputSystem->IsButtonDown(Settings::AntiAim::right))
	directionSwitch = false;

    CCSGOAnimState *animState = localplayer->GetAnimState();

    bool needToFlick = false;

    static float lastCheck;
    static bool lbyBreak = false;
    static float nextUpdate = FLT_MAX;

    if (localplayer->GetVelocity().Length2D() >= 0.1f || !(localplayer->GetFlags() & FL_ONGROUND) || localplayer->GetFlags() & FL_FROZEN)
    {
	lbyBreak = false;
	lastCheck = globalVars->curtime;
	nextUpdate = globalVars->curtime + 0.22;
    }
    else if ((!lbyBreak && (globalVars->curtime - lastCheck) > 0.22) || (lbyBreak && (globalVars->curtime - lastCheck) > 1.1))
    {
	lbyBreak = true;
	lastCheck = globalVars->curtime;
	needToFlick = true;
	nextUpdate = globalVars->curtime + 1.1;
    }

    if ((nextUpdate - globalVars->interval_per_tick) >= globalVars->curtime && nextUpdate <= globalVars->curtime)
	CreateMove::sendPacket = false;

    if (needToFlick)
    {
	CreateMove::sendPacket = false;

	if (Settings::AntiAim::LBYBreaker::enabled)
	    angle.y += directionSwitch ? -Settings::AntiAim::LBYBreaker::offset : Settings::AntiAim::LBYBreaker::offset;
	else if (Settings::AntiAim::type == AntiAimType::FREESTAND)
	    angle.y += lbyBreak;
	else
	    angle.y += directionSwitch ? -AntiAim::GetMaxDelta(animState) : AntiAim::GetMaxDelta(animState);
    }
    else
    {
	CreateMove::sendPacket = bSend;
	static AntiAimType type;

	float doubleDelta = AntiAim::GetMaxDelta(animState) * 2;

	DoAntiAim(type, doubleDelta, angle, bSend, animState, directionSwitch, localplayer);
    }

    Math::NormalizeAngles(angle);
    Math::ClampAngles(angle);

    cmd->viewangles = angle;

    Math::CorrectMovement(oldAngle, cmd, oldForward, oldSideMove);
}
