#pragma GCC diagnostic ignored "-Wcomment"
#pragma GCC diagnostic ignored "-Warray-bounds"

#include "ragebot.h"
#include "autowall.h"

#include "../Utils/bonemaps.h"
#include "../Utils/entity.h"
#include "../Utils/math.h"
#include "../Utils/xorstring.h"
#include "backtrack.h"
#include "../interfaces.h"

#include <thread>
#include <iostream>
#include <stdlib.h>
#include <random>
#include <climits>

#define PI_F (3.14)
#define absolute(x) ( x = x < 0 ? x * -1 : x)

#define TICK_INTERVAL globalVars->interval_per_tick
#define TIME_TO_TICKS(dt) ((int)(0.5f + (float)(dt) / TICK_INTERVAL))
#define TICKS_TO_TIME(t) (TICK_INTERVAL *(t))

#ifndef NormalizeNo
#define NormalizeNo(x) (x = (x < 0) ? ( x * -1) : x )
#endif

#ifndef GetPercentVal
#define GetPercentVal(val, percent) ( val * (percent/100) )
#endif

#ifndef GetPercent
#define GetPercent(total, val) ( (val/total) * 100)
#endif

std::vector<int64_t> Ragebot::friends = {};
std::vector<long> RagebotkillTimes = { 0 }; // the Epoch time from when we kill someone

inline bool	EnemyPresent = false,
    doubleFire = false,
    DieBitch = false;

inline int DeathBoneIndex = 0, MaxEnemyRender = 2;
const int MultiVectors = 7;
static Vector DoubleTapSpot = Vector(0);

static Vector VelocityExtrapolate(C_BasePlayer* player, Vector aimPos)
{
    // cvar->ConsoleDPrintf(XORSTR("Lenth : %f\n"), player->GetVelocity().Length2D());
    if (player->GetVelocity().Length2D() < 125.f)
	return aimPos;
    return aimPos + (player->GetVelocity() * globalVars->interval_per_tick);
}

/* Fills points Vector. True if successful. False if not.  Credits for Original method - ReactiioN */
static bool HeadMultiPoint(C_BasePlayer* player, Vector points[], matrix3x4_t boneMatrix[])
{

    model_t* pModel = player->GetModel();
    if (!pModel)
	return false;

    studiohdr_t* hdr = modelInfo->GetStudioModel(pModel);
    if (!hdr)
	return false;

    mstudiobbox_t* bbox = hdr->pHitbox((int)CONST_BONE_HEAD, 0);

    if (!bbox)
	return false;

    Vector mins, maxs;
    Math::VectorTransform(bbox->bbmin, boneMatrix[bbox->bone], mins);
    Math::VectorTransform(bbox->bbmax, boneMatrix[bbox->bone], maxs);

    Vector center = (mins + maxs) * 0.5f;

    // 0 - center, 1 - forehead, 2 - skullcap,
    // 3 - leftear, 4 - rightear, 5 - nose, 6 - backofhead

    points[MultiVectors] = (center,center,center,center,center,center);
    /* // OLD CODE
for (int i = 0; i < headVectors; i++) // set all points initially to center mass of head.
    points[i] = center;
    */
    points[1].z += bbox->radius * 0.60f; // morph each point.
    points[2].z += bbox->radius * 1.35f; // ...
    points[3].y -= bbox->radius * 0.80f;
    points[3].z += bbox->radius * 0.90f;
    points[4].x += bbox->radius * 0.80f;
    points[5].x -= bbox->radius * 0.80f;
    points[6].y += bbox->radius * 0.80f;
    points[7].y -= bbox->radius * 0.80f;

    return true;
}

static bool UpperChestMultiPoint(C_BasePlayer* player, Vector points[], matrix3x4_t boneMatrix[])
{

    model_t* pModel = player->GetModel();
    if (!pModel)
	return false;

    studiohdr_t* upChst = modelInfo->GetStudioModel(pModel);
    if (!upChst)
	return false;

    mstudiobbox_t* bbox = upChst->pHitbox((int)CONST_BONE_UPPER_SPINE, 0);
    if (!bbox)
	return false;

    Vector mins, maxs;
    Math::VectorTransform(bbox->bbmin, boneMatrix[bbox->bone], mins);
    Math::VectorTransform(bbox->bbmax, boneMatrix[bbox->bone], maxs);

    Vector center = (mins + maxs) * 0.5f;

    points[MultiVectors] = (center, center, center, center, center, center,center);

    points[1].y -= bbox->radius * 0.80f;
    points[1].z += bbox->radius * 0.90f;
    points[2].z += bbox->radius * 1.25f;
    points[3].y += bbox->radius * 0.80f;
    points[4].x += bbox->radius * 0.80f;
    points[5].x -= bbox->radius * 0.80f;
    points[6].y -= bbox->radius * 0.80f;

    return true;
}

static bool ChestMultiPoint(C_BasePlayer* player, Vector points[], matrix3x4_t boneMatrix[])
{

    model_t* pModel = player->GetModel();
    if (!pModel)
	return false;

    studiohdr_t* hdr = modelInfo->GetStudioModel(pModel);
    if (!hdr)
	return false;

    mstudiobbox_t* bbox = hdr->pHitbox((int)CONST_BONE_MIDDLE_SPINE, 0);
    if (!bbox)
	return false;

    Vector mins, maxs;
    Math::VectorTransform(bbox->bbmin, boneMatrix[bbox->bone], mins);
    Math::VectorTransform(bbox->bbmax, boneMatrix[bbox->bone], maxs);

    Vector center = (mins + maxs) * 0.5f;
    /*
    * To redunce time we directly implement the values rather than ... using for loop
    */
    points[MultiVectors] = (center,center,center,center,center,center,center);

    points[1].y -= bbox->radius * 0.80f;
    points[1].z += bbox->radius * 0.90f;
    points[2].z += bbox->radius * 1.25f;
    points[3].y += bbox->radius * 0.80f;
    points[4].x += bbox->radius * 0.80f;
    points[5].x -= bbox->radius * 0.80f;
    points[6].y -= bbox->radius * 0.80f;

    return true;
}

static bool LowerChestMultiPoint(C_BasePlayer* player, Vector points[], matrix3x4_t boneMatrix[])
{
    model_t* pModel = player->GetModel();
    if (!pModel)
	return false;

    studiohdr_t* hdr = modelInfo->GetStudioModel(pModel);
    if (!hdr)
	return false;

    mstudiobbox_t* bbox = hdr->pHitbox((int)CONST_BONE_LOWER_SPINE, 0);
    if (!bbox)
	return false;

    Vector mins, maxs;
    Math::VectorTransform(bbox->bbmin, boneMatrix[bbox->bone], mins);
    Math::VectorTransform(bbox->bbmax, boneMatrix[bbox->bone], maxs);

    Vector center = (mins + maxs) * 0.5f;

    /*
    * To redunce time we directly implement the values rather than ... using for loop
    */
    points[MultiVectors] = (center,center,center,center,center,center,center);

    points[1].y -= bbox->radius * 0.80f;
    points[1].z += bbox->radius * 0.90f;
    points[2].z += bbox->radius * 1.25f;
    points[3].y += bbox->radius * 0.80f;
    points[4].x += bbox->radius * 0.80f;
    points[5].x -= bbox->radius * 0.80f;
    points[6].y -= bbox->radius * 0.80f;

    return true;
}

/*
** Method for safety damage prediction where
** It will just look for required Damage Not for the best damage
*/
static void GetDamageAndSpots(C_BasePlayer* player, Vector &spot, float& damage, float& playerHelth, int& i)
{
    using namespace Settings;

    const std::unordered_map<int, int>* modelType = BoneMaps::GetModelTypeBoneMap(player);

    // float FOV = Settings::Ragebot::AutoAim::fov;

    damage = 0.f;
    spot = Vector(0);

    matrix3x4_t boneMatrix[128];

    if ( !player->SetupBones(boneMatrix, 128, 0x100, 0) )
	return;

    if (!Settings::Ragebot::AutoAim::desiredBones[i])
	return;

    int boneID = (*modelType).at(i);

    if (boneID == BONE_INVALID) // bone not available on this modeltype.
	return;

    // If we found head here
    if (boneID == BONE_HEAD) // head multipoint
    {
	Vector headPoints[MultiVectors] = {Vector(0), Vector(0), Vector(0), Vector(0), Vector(0), Vector(0), Vector(0)};

	if (!HeadMultiPoint(player, headPoints, boneMatrix))
	    return;

	float prevDamage = 0;
	// cvar->ConsoleDPrintf(XORSTR("Found the head\n"));
	for (int j = 0; j < MultiVectors; j++)
	{
	    Autowall::FireBulletData data;
	    float spotDamage = Autowall::GetDamage(headPoints[j], true, data);

	    if (spotDamage >= playerHelth + 9 )
	    {
		damage = spotDamage;
		spot = headPoints[j];
		DieBitch = true;
		return;
	    }
	    // if ( (spotDamage >= playerHelth / 2) && Settings::Ragebot::DoubleFire)
	    // {
	    // 	spot = DoubleTapSpot = headPoints[j];
	    // 	doubleFire = true;
	    // 	return;
	    // }

	    if (spotDamage > prevDamage)
	    {
		damage = prevDamage = spotDamage;
		spot = headPoints[j];
	    }

	    return;
	}
    }

    else if (boneID == BONE_UPPER_SPINAL_COLUMN) // upper chest MultiPoint
    {
	Vector upperChest[MultiVectors] = {Vector(0), Vector(0), Vector(0), Vector(0), Vector(0), Vector(0), Vector(0)};
	if (!UpperChestMultiPoint(player, upperChest, boneMatrix))
	    return;

	//cvar->ConsoleDPrintf(XORSTR("Found the upperchest\n"));
	for (int j = 0; j < MultiVectors; j++)
	{
	    Autowall::FireBulletData data;
	    float spotDamage = Autowall::GetDamage(upperChest[j], true, data);
	    float prevDamage = 0;

	    if (spotDamage >= playerHelth + 9 )
	    {
		damage = spotDamage;
		spot = upperChest[j];
		DieBitch = true;
		return;
	    }
	    // else if ( (spotDamage >= playerHelth / 2) && Settings::Ragebot::DoubleFire)
	    // {
	    // 	spot = DoubleTapSpot = upperChest[j];
	    // 	doubleFire = true;
	    // 	return;
	    // }

	    if (spotDamage > prevDamage )
	    {
		damage = prevDamage = spotDamage;
		spot = upperChest[j];
	    }
	    return;
	}
    }

    else if (boneID == BONE_MIDDLE_SPINAL_COLUMN) // Chest Multipoint
    {
	Vector MiddleChest[MultiVectors] = {Vector(0), Vector(0), Vector(0), Vector(0), Vector(0), Vector(0), Vector(0)};
	if (!ChestMultiPoint(player, MiddleChest, boneMatrix))
	    return;

	//cvar->ConsoleDPrintf(XORSTR("Found the middle chest\n"));
	for (int j = 0; j < MultiVectors; j++)
	{
	    Autowall::FireBulletData data;
	    float spotDamage = Autowall::GetDamage(MiddleChest[j], true, data);
	    float prevDamage = 0;

	    if (spotDamage >= playerHelth + 9 )
	    {
		damage = spotDamage;
		spot = MiddleChest[j];
		DieBitch = true;
		return;
	    }
	    // else if ( (spotDamage >= playerHelth / 2) && Settings::Ragebot::DoubleFire)
	    // {
	    // 	DoubleTapSpot = MiddleChest[j];
	    // 	doubleFire = true;
	    // 	return;
	    // }

	    if (spotDamage > prevDamage)
	    {
		damage = prevDamage = spotDamage;
		spot = MiddleChest[j];
	    }
	    return;
	}
    }

    else if (boneID == BONE_LOWER_SPINAL_COLUMN) // Lower multipoint
    {
	Vector LowerChestVec[MultiVectors] = {Vector(0), Vector(0), Vector(0), Vector(0), Vector(0), Vector(0), Vector(0)};

	if (!LowerChestMultiPoint(player, LowerChestVec, boneMatrix))
	    return;

	//cvar->ConsoleDPrintf(XORSTR("Found the lowerChest\n"));
	for (int j = 0; j < MultiVectors; j++)
	{
	    Autowall::FireBulletData data;
	    float spotDamage = Autowall::GetDamage(LowerChestVec[j], true, data);
	    float prevDamage = 0;

	    if (spotDamage >= playerHelth + 9 )
	    {
		damage = spotDamage;
		spot = LowerChestVec[j];
		DieBitch = true;
		return;
	    }
	    // else if ( (spotDamage >= playerHelth / 2) && Settings::Ragebot::DoubleFire)
	    // {
	    // 	spot = DoubleTapSpot = LowerChestVec[j];
	    // 	doubleFire = true;
	    // 	return;
	    // }

	    if (spotDamage > prevDamage)
	    {
		damage = prevDamage = spotDamage;
		spot = LowerChestVec[j];
	    }
	    return;
	}
    }

    else
    {
	Vector bone3D = player->GetBonePosition(boneID);

	//cvar->ConsoleDPrintf(XORSTR("bone ID : %d \n"), boneID);
	Autowall::FireBulletData data;
	float spotDamage = Autowall::GetDamage(bone3D, true, data);

	if (spotDamage >= playerHelth + 9 )
	{
	    damage = spotDamage;
	    spot = bone3D;
	    DieBitch = true;
	    return;
	}
	// else if ( (spotDamage >= playerHelth / 2) && Settings::Ragebot::DoubleFire)
	// {
	//     DoubleTapSpot = bone3D;
	// 	doubleFire = true;
	// 	return;
	// }

	if (spotDamage)
	{
	    damage = spotDamage;
	    spot = bone3D;
	}
	return;
    }

}


/*
** Get best Damage from the enemy and the spot
*/
static void GetBestSpotAndDamage(C_BasePlayer* player, Vector& Spot, float& Damage)
{

    int len = sizeof(Settings::Ragebot::AutoAim::desiredBones) / sizeof(Settings::Ragebot::AutoAim::desiredBones[0]);

    Vector spot;
    float damage;

    float playerHelth = player->GetHealth();


    // For safety mesurements
    if (Settings::Ragebot::damagePrediction == DamagePrediction::justDamage)
    {
	for (int i = 0; i < len; i++)
	{
	    GetDamageAndSpots(player, spot, damage, playerHelth, i);
	    if (DieBitch)
	    {
		Damage = damage;
		Spot = spot;
		return;
	    }
	    if (damage > Settings::Ragebot::visibleDamage && damage > 0 && Entity::IsSpotVisible(player,spot))
	    {
		Spot = spot;
		Damage = damage;
		return;
	    }
	    else if (damage > Settings::Ragebot::AutoWall::value && damage > 0)
	    {
		Spot = spot;
		Damage = damage;
		return;
	    }

	}

    }

    else if (Settings::Ragebot::damagePrediction == DamagePrediction::damage)
    {
	float prevdamage = 0;
	for (int i = 0; i < len; i++)
	{
	    GetDamageAndSpots(player, spot, damage, playerHelth, i);
	    if (DieBitch)
	    {
		Damage = damage;
		Spot = spot;
		return;
	    }
	    if (damage > Settings::Ragebot::visibleDamage && damage > prevdamage && Entity::IsSpotVisible(player,spot))
	    {
		Spot = spot;
		prevdamage = Damage = damage;
	    }
	    else if (damage > Settings::Ragebot::AutoWall::value && damage > prevdamage)
	    {
		Spot = spot;
		prevdamage = Damage = damage;
	    }

	}
	return;
    }

}


/*
* To find the closesnt enemy to reduce the calculation time and increase performace
* Original Credits to: https://github.com/goldenguy00 ( study! study! study! :^) )
*/
static C_BasePlayer* GetClosestEnemy (C_BasePlayer *localplayer, CUserCmd* cmd)
{
    C_BasePlayer* closestPlayer = nullptr;
    Vector pVecTarget = localplayer->GetEyePosition();
    QAngle viewAngles;
    engine->GetViewAngles(viewAngles);
    float prevFOV = 0.f;

    for (int i = engine->GetMaxClients(); i > 1; i--)
    {
	C_BasePlayer* player = (C_BasePlayer*)entityList->GetClientEntity(i);

	if (!player
	    || player == localplayer
	    || player->GetDormant()
	    || !player->GetAlive()
	    || player->GetImmune())
	    continue;

	if (!Settings::Ragebot::friendly && Entity::IsTeamMate(player, localplayer))
	    continue;

	Vector cbVecTarget = player->GetAbsOrigin();


	float cbFov = Math::GetFov( viewAngles, Math::CalcAngle(pVecTarget, cbVecTarget) );

	if (prevFOV == 0.f)
	{
	    prevFOV = cbFov;
	    closestPlayer = player;
	}
	else if ( cbFov < prevFOV )
	{
	    return player;
	}
	else
	    break;
    }
    return closestPlayer;
}

// get the clossest player from crosshair
static C_BasePlayer* GetClosestPlayerAndSpot(CUserCmd* cmd,C_BasePlayer* localplayer, Vector& bestSpot, float& bestDamage)
{
    if (!localplayer->GetAlive())
	return nullptr;
    /*
if ( doubleFire)
    {
	    bestSpot = DoubleTapSpot;
	    DoubleTapSpot = Vector(0);
	    doubleFire = false;
	    return localplayer;
    }
    */

    C_BasePlayer* player = GetClosestEnemy(localplayer, cmd);

    if ( !player )
	return nullptr;

    GetBestSpotAndDamage(player, bestSpot, bestDamage);

    if (bestSpot.IsZero())
	return nullptr;

    return player;
}

// Get the best damage and the player
static C_BasePlayer* GetBestEnemyAndSpot(CUserCmd* cmd,C_BasePlayer* localplayer, Vector& BestSpot, float& BestDamage)
{

    C_BasePlayer* closestEntity = nullptr;

    if (!localplayer->GetAlive())
	return nullptr;
    /*
	if (doubleFire && !DoubleTapSpot.IsZero())
	{
		BestSpot = DoubleTapSpot;
		DoubleTapSpot = Vector(0);
		doubleFire = false;
		return localplayer;
	}
	*/
    Vector bestSpot = Vector(0);
    float bestDamage = 0.f;
    float prevDamage = 0.f;

    for (int i = engine->GetMaxClients(); i  > 1; i--)
    {
	C_BasePlayer* player = (C_BasePlayer*) entityList->GetClientEntity(i);

	if (!player
	    || player == localplayer
	    || player->GetDormant()
	    || !player->GetAlive()
	    || player->GetImmune())
	    continue;

	if (Entity::IsTeamMate(player, localplayer)) // Checking for Friend If any it will continue to next player
	    continue;


	GetBestSpotAndDamage(player, bestSpot, bestDamage);

	if (DieBitch)
	{
	    DieBitch = false;
	    BestSpot = bestSpot;
	    closestEntity = player;
	}
	else if (bestDamage > prevDamage)
	{
	    BestDamage = bestDamage;
	    BestSpot = bestSpot;
	    closestEntity = player;
	}
    }

    if (BestSpot.IsZero())
	return nullptr;

    return closestEntity;
}

//Hitchance source from nanoscence
// static float Ragebothitchance(C_BasePlayer* localplayer, C_BaseCombatWeapon* activeWeapon)
// {
//     float hitchance = 10;
//     activeWeapon->UpdateAccuracyPenalty();
//     if (activeWeapon)
//     {
// 	float inaccuracy = activeWeapon->GetInaccuracy();
// 	if (inaccuracy == 0) inaccuracy = 0.0000001;
// 	hitchance = 1 / inaccuracy;
//
// 	return hitchance;
//     }
//     return hitchance;
// }
static void NormalizeAngles( Vector& angles )
{
    for ( auto i = 0; i < 3; i++ ) {
	while ( angles [ i ] < -180.0f ) angles [ i ] += 360.0f;
	while ( angles [ i ] > 180.0f ) angles [ i ] -= 360.0f;
    }
}

static Vector CrossProduct(const Vector& a, const Vector& b)
{
    return Vector( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
}

static void VectorAngles( Vector& forward, Vector& up, Vector& angles )
{
    Vector left = CrossProduct(up, forward );
    left.NormalizeInPlace( );

    float forwardDist = forward.Length2D( );

    if ( forwardDist > 0.001f )
    {
	angles.x = atan2f( -forward.z, forwardDist ) * 180 / PI_F;
	angles.y = atan2f( forward.y, forward.x ) * 180 / PI_F;

	float upZ = (left.y * forward.x) - (left.x * forward.y);
	angles.z = atan2f( left.z, upZ ) * 180 / PI_F;
    }
    else
    {
	angles.x = atan2f( -forward.z, forwardDist ) * 180 / PI_F;
	angles.y = atan2f( -left.x, left.y ) * 180 / PI_F;
	angles.z = 0;
    }
}

static float RandomFloat(float min, float max)
{
    static std::default_random_engine e;
    static std::uniform_real_distribution<> dis(min, max);
    return dis(e);

}

// hitchance from versas
// FIXME: what I think is happening: aa causes hitchance to go straight down
// FIXME: instead work with localplayer's onshot
static bool Ragebothitchance(C_BasePlayer* localplayer, C_BaseCombatWeapon* weapon, CUserCmd* cmd)
{
    if ( !weapon )
	return false;

    cvar->ConsoleDPrintf(XORSTR("<<<< target hitchance: %f >>>"), Settings::Ragebot::HitChance::value);

    Vector forward, right, up;
    Vector src = localplayer->GetEyePosition();
    QAngle qangles = cmd->viewangles;
    Vector angles;
    angles.x = qangles.x;
    angles.y = qangles.y;
    angles.z = qangles.z;

    cvar->ConsoleDPrintf(XORSTR("angles: {%d, %d, %d}\n"), angles.x, angles.y, up.z);
    cvar->ConsoleDPrintf(XORSTR("forward: {%d, %d, %d}\n"), forward.x, forward.y, forward.z);
    cvar->ConsoleDPrintf(XORSTR("right: {%d, %d, %d}\n"), right.x, right.y, right.z);
    cvar->ConsoleDPrintf(XORSTR("up: {%d, %d, %d}\n"), up.x, up.y, up.z);

    Math::AngleVectors( angles, forward, right, up );

    int cHits = 0;
    int cNeededHits = static_cast< int >(150.f * (Settings::Ragebot::HitChance::value / 100.f));

    weapon->UpdateAccuracyPenalty( );
    float weap_spread = weapon->GetSpread( );
    float weap_inaccuracy = weapon->GetInaccuracy( );

    cvar->ConsoleDPrintf(XORSTR("hitsTarget: %d "), cNeededHits);


    for ( int i = 0; i < 150; i++ )
    {

	float a = RandomFloat( 0.f, 1.f );
	float b = RandomFloat( 0.f, 2.f * PI_F );
	float c = RandomFloat( 0.f, 1.f );
	float d = RandomFloat( 0.f, 2.f * PI_F );

	float inaccuracy = a * weap_inaccuracy;
	float spread = c * weap_spread;

	if (*weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER)
	{
	    a = 1.f - a * a;
	    a = 1.f - c * c;
	}

	Vector spreadView( (cos( b ) * inaccuracy) + (cos( d ) * spread), (sin( b ) * inaccuracy) + (sin( d ) * spread), 0 ), direction;

	direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
	direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
	direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
	direction.Normalize();

	Vector viewAnglesSpread;
	VectorAngles( direction, up, viewAnglesSpread );
	NormalizeAngles( viewAnglesSpread );

	Vector viewForward;
	Math::AngleVectors( viewAnglesSpread, viewForward );
	viewForward.NormalizeInPlace( );

	viewForward = src + (viewForward * weapon->GetCSWpnData( )->GetRange());

	trace_t tr;
	Ray_t ray;


	ray.Init( src, viewForward );

	trace->ClipRayToEntity( ray, MASK_SHOT | CONTENTS_GRATE, localplayer, &tr );

	if ( tr.m_pEntityHit == localplayer)
	    ++cHits;

	int sc = static_cast< int >((static_cast< float >(cHits) / 150.f) * 100.f);
	if (sc > 0)
	{
	    cvar->ConsoleDPrintf(XORSTR("=========== hitchance: %d ==========="), sc);
	}
	cvar->ConsoleDPrintf(XORSTR("hits: %d "), cHits);
	if ( static_cast< int >((static_cast< float >(cHits) / 150.f) * 100.f) >= Settings::Ragebot::HitChance::value )
	    return true;

	if ( (150 - i + cHits) < cNeededHits )
	    return false;
    }
    return false;
}

static void RagebotNoRecoil(QAngle& angle, CUserCmd* cmd, C_BasePlayer* localplayer, C_BaseCombatWeapon* activeWeapon)
{

    if (!(cmd->buttons & IN_ATTACK))
	return;

    QAngle CurrentPunch = *localplayer->GetAimPunchAngle();

    angle.x -= CurrentPunch.x * 2.f;
    angle.y -= CurrentPunch.y * 2.f;

}

// AutoCroutch is a bad idea in hvh instant death if you miss
static void RagebotAutoCrouch(C_BasePlayer* player, CUserCmd* cmd, C_BaseCombatWeapon* activeWeapon)
{
    // dont ever use autocrouch just crouch urself im too lazy to fix settings
    // if (!player || !Settings::Ragebot::AutoCrouch::enable)
	return;

    if (activeWeapon->GetNextPrimaryAttack() > globalVars->curtime)
	return;

    cmd->buttons |= IN_DUCK;
}

static void RagebotAutoSlow(C_BasePlayer* player,C_BasePlayer* localplayer, C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd, float& forrwordMove, float& sideMove)
{
    return;

    if (!Settings::Ragebot::AutoSlow::enabled)
	return;

    if (!localplayer || !player)
	return;

    if (activeWeapon->GetNextPrimaryAttack() > globalVars->curtime || !activeWeapon || activeWeapon->GetAmmo() == 0)
	return;
    bool hc = Ragebothitchance(localplayer, activeWeapon, cmd);
//  if (Settings::Ragebot::HitChanceOverwrride::enable)
//  	hc -= GetPercentVal(hc, Settings::Ragebot::HitChanceOverwrride::value);

    if (!hc)
    {
	float curTime = globalVars->curtime;
	QAngle ViewAngle = cmd->viewangles;
	Math::ClampAngles(ViewAngle);
	static auto oldorigin = localplayer->GetAbsOrigin();
	Vector unpredictedVal = ( localplayer->GetAbsOrigin() - oldorigin ) * ( 1.0 / globalVars->interval_per_tick );
	Vector velocity = localplayer->GetVelocity();
	float speed  = velocity.Length();

	if(speed > 15.f)
	{
	    QAngle dir;
	    Math::VectorAngles(velocity, dir);
	    dir.y = cmd->viewangles.y - dir.x;

	    Vector NewMove = Vector(0);
	    Math::AngleVectors(dir, NewMove);
	    float max = std::max( std::fabs( forrwordMove ), std::fabs( sideMove ) );
	    float mult = 450.f / max;

	    forrwordMove = NewMove.x;
	    sideMove = NewMove.y;
	}
    }
    else
    {
	cmd->buttons = IN_WALK;
	forrwordMove = 0;
	sideMove = 0;
    }

}

static void RagebotAutoPistol(C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
    if (!Settings::Ragebot::AutoPistol::enabled)
	return;

    if (!activeWeapon || activeWeapon->GetCSWpnData()->GetWeaponType() != CSWeaponType::WEAPONTYPE_PISTOL)
	return;

    if (activeWeapon->GetNextPrimaryAttack() < globalVars->curtime)
	return;

    if (*activeWeapon->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_REVOLVER)
	cmd->buttons &= ~IN_ATTACK;
}

static void RagebotAutoShoot(C_BasePlayer* player, C_BasePlayer* localplayer, C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd,const Vector& bestspot)
{
    if (!Settings::Ragebot::AutoShoot::enabled)
	return;

    if (!activeWeapon || activeWeapon->GetAmmo() == 0)
	return;

    CSWeaponType weaponType = activeWeapon->GetCSWpnData()->GetWeaponType();
    if (weaponType == CSWeaponType::WEAPONTYPE_KNIFE || weaponType == CSWeaponType::WEAPONTYPE_C4 || weaponType == CSWeaponType::WEAPONTYPE_GRENADE)
	return;

    if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER)
    {
	cmd->buttons |= IN_ATTACK;

	float postponeFireReadyTime = activeWeapon->GetPostPoneReadyTime();
	if (postponeFireReadyTime > 0 && postponeFireReadyTime < globalVars->curtime && !player)
	    cmd->buttons &= ~IN_ATTACK;

	return;
    }

    if (!player)
	return;

    if (Settings::Ragebot::AutoShoot::autoscope && Util::Items::IsScopeable(*activeWeapon->GetItemDefinitionIndex()) && !localplayer->IsScoped() && !(cmd->buttons & IN_ATTACK2))
    {
	cmd->buttons |= IN_ATTACK2;
	return; // continue next tick
    }

    bool hc = Ragebothitchance(localplayer, activeWeapon, cmd);
//     if (Settings::Ragebot::HitChanceOverwrride::enable)
// 	hc -= GetPercentVal(hc, Settings::Ragebot::HitChanceOverwrride::value);
    if (!hc)
	return;

    if (activeWeapon->GetNextPrimaryAttack() > globalVars->curtime)
	cmd->buttons &= ~IN_ATTACK;
    else
	cmd->buttons |= IN_ATTACK;
}

static void FixMouseDeltas(CUserCmd* cmd,C_BasePlayer* player, const QAngle& angle, const QAngle& oldAngle)
{
    if (!player)
	return;

    QAngle delta = angle - oldAngle;
    float sens = cvar->FindVar(XORSTR("sensitivity"))->GetFloat();
    float m_pitch = cvar->FindVar(XORSTR("m_pitch"))->GetFloat();
    float m_yaw = cvar->FindVar(XORSTR("m_yaw"))->GetFloat();
    float zoomMultiplier = cvar->FindVar("zoom_sensitivity_ratio_mouse")->GetFloat();

    Math::NormalizeAngles(delta);

    cmd->mousedx = -delta.y / (m_yaw * sens * zoomMultiplier);
    cmd->mousedy = delta.x / (m_pitch * sens * zoomMultiplier);
}
/* DT ==== */

static bool charged_dt = false;
static float last_doubletap;
static float tickBaseShift = 0;

static float GetWeaponFireRate(C_BaseCombatWeapon* weapon) {
    if (*weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_GLOCK)
	return 0.15f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_HKP2000 )
	return 0.169f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_P250 ) //the cz and p250 have the same name idky same with other guns
	return 0.15f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_TEC9 )
	return 0.12f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_ELITE )
	return 0.12f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_FIVESEVEN )
	return 0.15f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_DEAGLE )
	return 0.224f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_NOVA )
	return 0.882f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SAWEDOFF )
	return 0.845f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_MAG7 )
	return 0.845f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_XM1014 )
	return 0.35f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_MAC10 )
	return 0.075f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_UMP45 )
	return 0.089f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_MP9 )
	return 0.070f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_BIZON )
	return 0.08f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_MP7 )
	return 0.08f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_P90 )
	return 0.070f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_GALILAR )
	return 0.089f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_AK47 )
	return 0.1f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SG556 )
	return 0.089f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_M4A1 )
	return 0.089f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_AUG )
	return 0.089f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_M249 )
	return 0.08f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_NEGEV )
	return 0.0008f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SSG08 )
	return 1.25f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_AWP )
	return 1.463f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_G3SG1 )
	return 0.25f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SCAR20 )
	return 0.25f;
    else if ( *weapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_MP5 )
	return 0.08f;
    else
	return .0f;
}

static bool CanShift(C_BasePlayer* localPlayer, C_BaseCombatWeapon* activeWeapon)
{

    if ( !activeWeapon)
	return false;

    float m_flPlayerTime = (localPlayer->GetTickBase() - ((tickBaseShift > 0) ? 1 + tickBaseShift : 0)) * globalVars->interval_per_tick;

    if ( m_flPlayerTime <= activeWeapon->GetNextPrimaryAttack() )
	return false; // no need to shift ticks

    return true;
};

static bool should_restore(C_BasePlayer* localPlayer, C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
    static int count = 20;
    static bool start;

    if ( cmd->tick_count == last_doubletap + TIME_TO_TICKS(GetWeaponFireRate(activeWeapon) * 1) )
	start = true;

    if ( count == 0 )
    {
	start = false;
	count = 20;
    }
    while ( count != 0 && start )
    {
	count--;
	return true;
    }

    return false;
};
bool shoot_again(C_BasePlayer* localplayer, C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
    if ( cmd->tick_count > last_doubletap + TIME_TO_TICKS( GetWeaponFireRate(activeWeapon) * 2 ) )
	return true;
    else
	return false;

    return false;
};

// auto shoot_2 = [ ] ( ) -> bool
// {
//
//     auto weapon = g_LocalPlayer->m_hActiveWeapon( );
//     if ( !weapon )
// 	return false;
//     float m_flPlayerTime = (g_LocalPlayer->m_nTickBase( ) - ((nTickBaseShift > 0) ? 1 + nTickBaseShift : 0)) * g_pGlobalVars->interval_per_tick;
//
//     if ( m_flPlayerTime <= weapon->m_flNextPrimaryAttack( ) )
// 	return false; // no need to shift ticks
//
//     return true;
// };

static void DoubleTap(C_BasePlayer* localplayer, C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd) {
    static bool chokePack;
    if ( !localplayer || !localplayer->GetAlive())
	return;

    if (!activeWeapon || activeWeapon->GetInReload())
	return;

    // if ( !activeWeapon || activeWeapon->m_iClip1( ) == 0 ) return;
    if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_TASER ) return;
    if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_HEGRENADE || *activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_INCGRENADE || *activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_FLASHBANG || *activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_SMOKEGRENADE || *activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_MOLOTOV || *activeWeapon->GetItemDefinitionIndex( ) == ItemDefinitionIndex::WEAPON_DECOY || *activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_KNIFE || Util::Items::IsKnife(*activeWeapon->GetItemDefinitionIndex())) return;

    float flServerTime = localplayer->GetTickBase() * globalVars->interval_per_tick;
    bool canShoot = (activeWeapon->GetNextPrimaryAttack() <= flServerTime);

    static bool charge_dt = false;

    // ALWAYS DTTTT
    // TODO: add dt settings
    // if ( GetKeyState( Variables.rageaimbot.fastshoot ) )
	charge_dt = true;

    if ( true )// Variables.rageaimbot.doubletab == 1 )
    {
	if ( charge_dt && shoot_again(localplayer, activeWeapon, cmd) )
	    charged_dt = true;
	else
	    charged_dt = false;
    }
    else
    {
	if ( shoot_again(localplayer, activeWeapon, cmd) )
	    charged_dt = true;
	else
	    charged_dt = false;
    }

    if ( should_restore( localplayer, activeWeapon, cmd ) )
    {
	// doubletap_delta = 0; NOT USED ??
	last_doubletap = 0;
	cmd->tick_count = INT_MAX;
	cmd->buttons &= ~IN_ATTACK;
    }

//     if (charge_dt && !CanShift())
//     {
// 	    if (cmd->buttons & IN_ATTACK)
// 		    cmd->viewangles = angle - localplayer->m_aimPunchAngle() * g_CVar->FindVar("weapon_recoil_scale")->GetFloat();
//     }


    if ( false )// Variables.rageaimbot.doubletab == 1 )
    {
	if ( charge_dt && CanShift(localplayer, activeWeapon) && shoot_again( localplayer, activeWeapon, cmd ) )
	{
	    // TODO: do smt with chokePack
	    // globals::chockepack = 1;
	    if ( cmd->buttons & IN_ATTACK )
	    {
		last_doubletap = cmd->tick_count;
		tickBaseShift = TIME_TO_TICKS(GetWeaponFireRate(activeWeapon));
	    }
	    charge_dt = false;
	}

    }
    else // else if ( Variables.rageaimbot.doubletab == 2 )
    {
	if ( CanShift( localplayer, activeWeapon ) && shoot_again( localplayer, activeWeapon, cmd ) )
	{
	    // globals::chockepack = 1;
	    if ( cmd->buttons & IN_ATTACK )
	    {
		last_doubletap = cmd->tick_count;
		tickBaseShift = TIME_TO_TICKS(GetWeaponFireRate(activeWeapon));
	    }
	}

    }

}

void Ragebot::CreateMove(CUserCmd* cmd)
{
    if (!Settings::Ragebot::enabled)
	return;

    C_BasePlayer* localplayer = (C_BasePlayer*)entityList->GetClientEntity(engine->GetLocalPlayer());

    if (!localplayer || !localplayer->GetAlive())
	return;

    bool RagebotShouldAim = false;

    QAngle oldAngle;
    engine->GetViewAngles(oldAngle);
    float oldForward = cmd->forwardmove;
    float oldSideMove = cmd->sidemove;

    Vector localEye = localplayer->GetEyePosition();
    QAngle angle = cmd->viewangles;

    C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*)entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
    if (!activeWeapon || activeWeapon->GetInReload())
	return;

    CSWeaponType weaponType = activeWeapon->GetCSWpnData()->GetWeaponType();
    if (weaponType == CSWeaponType::WEAPONTYPE_C4 || weaponType == CSWeaponType::WEAPONTYPE_GRENADE || weaponType == CSWeaponType::WEAPONTYPE_KNIFE)
	return;

    if (prevWeapon != (ItemDefinitionIndex)*activeWeapon->GetItemDefinitionIndex())
    {
	prevWeapon = (ItemDefinitionIndex)*activeWeapon->GetItemDefinitionIndex();
	Ragebot::UpdateValues();
    }

    Vector bestSpot = Vector(0);
    float bestDamage = 0.f;

    C_BasePlayer* player = nullptr;
    switch (Settings::Ragebot::enemySelectionType)
    {
    case EnemySelectionType::BestDamage :
	player = GetBestEnemyAndSpot(cmd, localplayer, bestSpot, bestDamage);
	break;
    case EnemySelectionType::CLosestToCrosshair :
	player = GetClosestPlayerAndSpot(cmd, localplayer, bestSpot, bestDamage);
	break;
    default:
	break;
    }

    if (player)
    {
	//Auto Scop Controll system to controll auto scoping every time
	if (Settings::Ragebot::AutoShoot::autoscope)
	{
	    //cheking if the weapon scopable and not scop then it will scop and go back to the next tick
	    if (Util::Items::IsScopeable(*activeWeapon->GetItemDefinitionIndex()) && !localplayer->IsScoped() && !(cmd->buttons & IN_ATTACK2) )
	    {
		cmd->buttons |= IN_ATTACK2;
		return; // will go to the next tick
	    }
	}
	if (Settings::Ragebot::AutoShoot::enabled || cmd->buttons & IN_ATTACK)
	    RagebotShouldAim = true;

	Settings::Debug::AutoAim::target = bestSpot; // For Debug showing aimspot.

	if (RagebotShouldAim)
	    angle = Math::CalcAngle(localEye, bestSpot);
    }
    else // No player to Shoot
    {
	Settings::Debug::AutoAim::target = Vector(0);
	RagebotShouldAim = false;
    }

    RagebotAutoSlow(player,localplayer, activeWeapon, cmd, oldForward, oldSideMove);
    RagebotAutoCrouch(player, cmd, activeWeapon);
    RagebotAutoPistol(activeWeapon, cmd);
    // RagebotAutoShoot(player,localplayer, activeWeapon, cmd, bestSpot);
    DoubleTap(localplayer, activeWeapon, cmd);
    RagebotNoRecoil(angle, cmd, localplayer, activeWeapon);
    // RagebotNoShoot(activeWeapon, player, cmd);

    Math::NormalizeAngles(angle);
    Math::ClampAngles(angle);

    FixMouseDeltas(cmd, player, angle, oldAngle);
    cmd->viewangles = angle;

    Math::CorrectMovement(oldAngle, cmd, oldForward, oldSideMove);

    // if (!Settings::Ragebot::silent)
    // 	engine->SetViewAngles(cmd->viewangles);
}

void Ragebot::FireGameEvent(IGameEvent* event)
{
    if(!Settings::Ragebot::enabled)
	return;

    if (!event)
	return;

    if (strcmp(event->GetName(), XORSTR("player_connect_full")) == 0 || strcmp(event->GetName(), XORSTR("cs_game_disconnected")) == 0)
    {
	if (event->GetInt(XORSTR("userid")) && engine->GetPlayerForUserID(event->GetInt(XORSTR("userid"))) != engine->GetLocalPlayer())
	    return;
	Ragebot::friends.clear();
    }
    if (strcmp(event->GetName(), XORSTR("player_death")) == 0)
    {
	int attacker_id = engine->GetPlayerForUserID(event->GetInt(XORSTR("attacker")));
	int deadPlayer_id = engine->GetPlayerForUserID(event->GetInt(XORSTR("userid")));

	if (attacker_id == deadPlayer_id) // suicide
	    return;

	if (attacker_id != engine->GetLocalPlayer())
	    return;

	RagebotkillTimes.push_back(Util::GetEpochTime());
    }
}

