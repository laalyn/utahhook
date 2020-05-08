#include "ragebot.h"
#include "autowall.h"
#include "legitbot.h"

#include "../Utils/bonemaps.h"
#include "../Utils/entity.h"
#include "../Utils/math.h"
#include "../Utils/xorstring.h"
#include "../interfaces.h"
#include "../settings.h"

std::vector<int64_t> Ragebot::friends = {};
std::vector<long> RagebotkillTimes = { 0 }; // the Epoch time from when we kill someone

inline bool RagebotShouldAim = false, EnemyPresent = false;
int prevWeaponIndex = 0;
const int MultiVectors = 7;
static float prevSpotDamage = 0.f;

/* Fills points Vector. True if successful. False if not.  Credits for Original method - ReactiioN */
static bool HeadMultiPoint(C_BasePlayer* player, Vector points[], matrix3x4_t *boneMatrix)
{

	model_t* pModel = player->GetModel();
    if (!pModel)
		return false;

    studiohdr_t* hdr = modelInfo->GetStudioModel(pModel);
    if (!hdr)
		return false;
    
	mstudiobbox_t* bbox = hdr->pHitbox((int)Hitbox::HITBOX_HEAD, 0);
    if (!bbox)
		return false;

    Vector mins, maxs;
    Math::VectorTransform(bbox->bbmin, boneMatrix[bbox->bone], mins);
    Math::VectorTransform(bbox->bbmax, boneMatrix[bbox->bone], maxs);

    Vector center = (mins + maxs) * 0.5f;

    // new Head Points :
    // 0 - nose, 1 -  upperbackofhead, 2 - skullcap, 3 - forehead, 4 - leftear
    // 5 - rightear 6 - backofhead

	points[MultiVectors] = (center,center,center,center,center,center,center);
	/* // OLD CODE
    for (int i = 0; i < headVectors; i++) // set all points initially to center mass of head.
	points[i] = center;
	*/
    points[0].z += bbox->radius * 0.60f;
    points[1].y -= bbox->radius * 0.80f;
    points[1].z += bbox->radius * 0.90f;
    points[2].z += bbox->radius * 1.25f;
    points[3].y += bbox->radius * 0.80f;
    points[4].x += bbox->radius * 0.80f;
    points[5].x -= bbox->radius * 0.80f;
    points[6].y -= bbox->radius * 0.80f;

    return true;
}

static bool UpperChestMultiPoint(C_BasePlayer* player, Vector points[], matrix3x4_t *boneMatrix)
{
    
	model_t* pModel = player->GetModel();
    if (!pModel)
		return false;

    studiohdr_t* upChst = modelInfo->GetStudioModel(pModel);
    if (!upChst)
		return false;
    
	mstudiobbox_t* bbox = upChst->pHitbox((int)BONE_UPPER_SPINAL_COLUMN, 0);
    if (!bbox)
		return false;

    Vector mins, maxs;
    Math::VectorTransform(bbox->bbmin, boneMatrix[bbox->bone], mins);
    Math::VectorTransform(bbox->bbmax, boneMatrix[bbox->bone], maxs);

    Vector center = (mins + maxs) * 0.5f;

    // new Head Points :
    // 0 - nose, 1 -  upperbackofhead, 2 - skullcap, 3 - forehead, 4 - leftear
    // 5 - rightear 6 - backofhead
    for (int i = 0; i < MultiVectors; i++) // set all points initially to center mass of head.
		points[i] = center;
    points[1].y -= bbox->radius * 0.80f;
    points[1].z += bbox->radius * 0.90f;
    points[2].z += bbox->radius * 1.25f;
    points[3].y += bbox->radius * 0.80f;
    points[4].x += bbox->radius * 0.80f;
    points[5].x -= bbox->radius * 0.80f;
    points[6].y -= bbox->radius * 0.80f;

    return true;
}
 
static bool ChestMultiPoint(C_BasePlayer* player, Vector points[], matrix3x4_t *boneMatrix)
{

    model_t* pModel = player->GetModel();
    if (!pModel)
		return false;

    studiohdr_t* hdr = modelInfo->GetStudioModel(pModel);
    if (!hdr)
		return false;
    
	mstudiobbox_t* bbox = hdr->pHitbox((int)BONE_MIDDLE_SPINAL_COLUMN, 0);
    if (!bbox)
		return false;

    Vector mins, maxs;
    Math::VectorTransform(bbox->bbmin, boneMatrix[bbox->bone], mins);
    Math::VectorTransform(bbox->bbmax, boneMatrix[bbox->bone], maxs);

    Vector center = (mins + maxs) * 0.5f;

    // new Head Points :
    // 0 - nose, 1 -  upperbackofhead, 2 - skullcap, 3 - forehead, 4 - leftear
    // 5 - rightear 6 - backofhead

	/*
	* To redunce time we directly implement the values rather than ... using for loop
	*/
	points[MultiVectors] = (center,center,center,center,center,center,center);
    // OLD CODE
	// for (int i = 0; i < MultiVectors; i++) // set all points initially to center mass of head.
	// 	points[i] = center;
	// OLD CODE BYE
    // points[0].z += bbox->radius * 0.60f;
    points[1].y -= bbox->radius * 0.80f;
    points[1].z += bbox->radius * 0.90f;
    points[2].z += bbox->radius * 1.25f;
    points[3].y += bbox->radius * 0.80f;
    points[4].x += bbox->radius * 0.80f;
    points[5].x -= bbox->radius * 0.80f;
    points[6].y -= bbox->radius * 0.80f;

    return true;
}

static bool LowerChestMultiPoint(C_BasePlayer* player, Vector points[], matrix3x4_t *boneMatrix)
{
    model_t* pModel = player->GetModel();
    if (!pModel)
		return false;

    studiohdr_t* hdr = modelInfo->GetStudioModel(pModel);
    if (!hdr)
		return false;
    
	mstudiobbox_t* bbox = hdr->pHitbox((int)BONE_LOWER_SPINAL_COLUMN, 0);
    if (!bbox)
		return false;

    Vector mins, maxs;
    Math::VectorTransform(bbox->bbmin, boneMatrix[bbox->bone], mins);
    Math::VectorTransform(bbox->bbmax, boneMatrix[bbox->bone], maxs);

    Vector center = (mins + maxs) * 0.5f;

    // new Head Points :
    // 0 - nose, 1 -  upperbackofhead, 2 - skullcap, 3 - forehead, 4 - leftear
    // 5 - rightear 6 - backofhead

	/*
	* To redunce time we directly implement the values rather than ... using for loop
	*/
	points[MultiVectors] = (center,center,center,center,center,center,center);
    // OLD CODE
	// for (int i = 0; i < MultiVectors; i++) // set all points initially to center mass of head.
	// 	points[i] = center;
	// OLD CODE BYE
    // points[0].z += bbox->radius * 0.60f;
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
static void safetyPrediction(C_BasePlayer* player, Vector& wallbangspot, float& wallbangdamage, Vector &visibleSpot, float& VisibleDamage)
{

	static float minDamage = Settings::Ragebot::AutoWall::value;
    static float minDamageVisible = Settings::Ragebot::visibleDamage;
    const std::unordered_map<int, int>* modelType = BoneMaps::GetModelTypeBoneMap(player);	
	
	static int len = sizeof(Settings::Ragebot::AutoAim::desiredBones) / sizeof(Settings::Ragebot::AutoAim::desiredBones[0]);

    float FOV = Settings::Ragebot::AutoAim::fov;

	matrix3x4_t BoneMatrix[128];

	if (!player->SetupBones(BoneMatrix, 128, 0x100, 0.f))
		return;

	for (int i = 0; i < len; i++)
    {
		if (!Settings::Ragebot::AutoAim::desiredBones[i])
	    	continue;

		int boneID = (*modelType).at(i);

		if (boneID == BONE_INVALID) // bone not available on this modeltype.
	   		continue;

		bool VisiblityCheck = Entity::IsVisible(player, boneID, FOV, false);
		float playerHelth = player->GetHealth();

	// If we found head here
	if (i == BONE_HEAD) // head multipoint
	{
	    Vector headPoints[MultiVectors];
	    if (!HeadMultiPoint(player, headPoints, BoneMatrix))
			continue;

	    // cheaking for all head vectors
	    for (int j = 0; j < MultiVectors; j++)
	    {
			Autowall::FireBulletData data;
			float spotDamage = Autowall::GetDamage(headPoints[j], !Settings::Ragebot::friendly, data);

			// if ( !( spotDamage >= minDamageVisible >= minDamage) )
			// 	continue;

			if ( spotDamage > 0.f && !EnemyPresent)
		    	EnemyPresent = true;

			if (VisiblityCheck)
			{
				if (spotDamage >= playerHelth)
				{
		    		VisibleDamage = spotDamage;
		    		visibleSpot = headPoints[j];
					prevSpotDamage = 0.f;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamageVisible && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = VisibleDamage = spotDamage;
		    		visibleSpot = headPoints[j];
				}

			}
			else 
			{
				if (spotDamage >= playerHelth)
				{
		    		wallbangspot = headPoints[j];
		    		wallbangdamage = spotDamage;
					prevSpotDamage = 0.f;
		    		return;
				}

				if (spotDamage > 0.f && spotDamage >= minDamage)
				{
		    		prevSpotDamage = wallbangdamage = spotDamage;
		    		wallbangspot = headPoints[j];
					return;
				}
			}
			
			
			
	    }
	}
	
	else if (i == BONE_UPPER_SPINAL_COLUMN) // upper chest MultiPoint
	{
	    Vector upperChest[MultiVectors];
	    if (!UpperChestMultiPoint(player, upperChest, BoneMatrix))
			continue;

	    // cheaking for all upper Chest vectors
	    for (int j = 0; j < MultiVectors; j++)
	    {
			Autowall::FireBulletData data;
			float spotDamage = Autowall::GetDamage(upperChest[j], !Settings::Ragebot::friendly, data);

			// if ( !( spotDamage >= minDamageVisible >= minDamage) )
			// 	continue;

			if ( spotDamage > 0.f && !EnemyPresent)
		    	EnemyPresent = true;
				
			if (VisiblityCheck)
			{
				if (spotDamage >= playerHelth)
				{
		    		VisibleDamage = spotDamage;
		    		visibleSpot = upperChest[j];
					prevSpotDamage = 0.f;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamageVisible && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = VisibleDamage = spotDamage;
		    		visibleSpot = upperChest[j];
				}

			}
			else 
			{
				if (spotDamage >= playerHelth)
				{
		    		wallbangspot = upperChest[j];
		    		wallbangdamage = spotDamage;
					prevSpotDamage = 0.f;
		    		return;
				}

				if (spotDamage > 0.f && spotDamage >= minDamage)
				{
		    		prevSpotDamage = wallbangdamage = spotDamage;
		    		wallbangspot = upperChest[j];
					return;
				}
			}
			
	    }
	}
	
	else if (i == BONE_MIDDLE_SPINAL_COLUMN) // Chest Multipoint
	{
	    Vector MiddleChest[MultiVectors];
	    if (!ChestMultiPoint(player, MiddleChest, BoneMatrix))
		continue;

	    // cheaking for all head vectors
	    for (int j = 0; j < MultiVectors; j++)
	    {
			Autowall::FireBulletData data;
			float spotDamage = Autowall::GetDamage(MiddleChest[j], !Settings::Ragebot::friendly, data);

			// if ( !( spotDamage >= minDamageVisible >= minDamage) )
			// 	continue;
			
			if ( spotDamage > 0.f && !EnemyPresent)
		    	EnemyPresent = true;

			if (VisiblityCheck)
			{
				if (spotDamage >= playerHelth)
				{
		    		VisibleDamage = spotDamage;
		    		visibleSpot = MiddleChest[j];
					prevSpotDamage = 0.f;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamageVisible && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = VisibleDamage = spotDamage;
		    		visibleSpot = MiddleChest[j];
				}

			}
			else 
			{
				if (spotDamage >= playerHelth && !VisiblityCheck)
				{
		    		wallbangspot = MiddleChest[j];
		    		wallbangdamage = spotDamage;
					prevSpotDamage = 0.f;
		    		return;
				}

				if (spotDamage > 0.f && spotDamage >= minDamage)
				{
		    		prevSpotDamage = wallbangdamage = spotDamage;
		    		wallbangspot = MiddleChest[j];
					return;
				}
			}
	    }
	}
	
	else if (i == BONE_LOWER_SPINAL_COLUMN) // Lower multipoint
	{
	    Vector LowerChest[MultiVectors];
	    if (!LowerChestMultiPoint(player, LowerChest, BoneMatrix))
			continue;

	    // cheaking for all head vectors
	    for (int j = 0; j < MultiVectors; j++)
	    {
			Autowall::FireBulletData data;
			float spotDamage = Autowall::GetDamage(LowerChest[j], !Settings::Ragebot::friendly, data);

			// if ( !( spotDamage >= minDamageVisible >= minDamage) )
			// 	continue;

			if ( spotDamage > 0.f && !EnemyPresent)
		    	EnemyPresent = true;

			if (VisiblityCheck)
			{
				if (spotDamage >= playerHelth)
				{
		    		VisibleDamage = spotDamage;
		    		visibleSpot = LowerChest[j];
					prevSpotDamage = 0.f;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamageVisible  && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = VisibleDamage = spotDamage;
		    		visibleSpot = LowerChest[j];
				}

			}
			else 
			{
				if (spotDamage >= playerHelth && !VisiblityCheck)
				{
		    		wallbangspot = LowerChest[j];
		    		wallbangdamage = spotDamage;
					prevSpotDamage = 0.f;
		    		return;
				}

				if (spotDamage > 0.f && spotDamage >= minDamage)
				{
		    		prevSpotDamage = wallbangdamage = spotDamage;
		    		wallbangspot = LowerChest[j];
					return;
				}
			}
			
	    }
	}
	
	else 
	{
		Vector bone3D = player->GetBonePosition(boneID);

		Autowall::FireBulletData data;
		float boneDamage = Autowall::GetDamage(bone3D, !Settings::Ragebot::friendly, data);

		// if ( !( boneDamage >= minDamageVisible >= minDamage) )
		// 		continue;

		if ( boneDamage > 0.f && !EnemyPresent)
		    	EnemyPresent = true;

		if (VisiblityCheck)
		{
			if (boneDamage >= playerHelth)
			{
				visibleSpot = bone3D;
				VisibleDamage = boneDamage;
				prevSpotDamage = 0.f;
				return;
			}
			if ( boneDamage >= minDamageVisible && boneDamage > prevSpotDamage && boneDamage > prevSpotDamage)
	    	{
				visibleSpot = bone3D;
				prevSpotDamage = VisibleDamage = boneDamage;
	    	}
		}
		else 
		{
			if (boneDamage >= playerHelth)
			{
				wallbangspot = bone3D;
				wallbangdamage = boneDamage;
				prevSpotDamage = 0.f;
				return;
	    	}
			if ( boneDamage >= minDamage)
			{
	    		wallbangspot = bone3D;
	    		prevSpotDamage = wallbangdamage = boneDamage;
				return;
			}
	    	
		}
	
      }
	}

	return;

}

/*
** Method to calculate the best damge
** To kill enemy instantly
*/
static void BestDamagePrediction(C_BasePlayer* player, Vector& wallbangspot, float& wallbangdamage, Vector &visibleSpot, float& VisibleDamage)
{

	static float minDamage = Settings::Ragebot::AutoWall::value;
    static float minDamageVisible = Settings::Ragebot::visibleDamage;
    const std::unordered_map<int, int>* modelType = BoneMaps::GetModelTypeBoneMap(player);	
	
	static int len = sizeof(Settings::Ragebot::AutoAim::desiredBones) / sizeof(Settings::Ragebot::AutoAim::desiredBones[0]);

    float FOV = Settings::Ragebot::AutoAim::fov;

	matrix3x4_t BoneMatrix[128];

	if (!player->SetupBones(BoneMatrix, 128, 0x100, 0.f))
		return;

	for (int i = 0; i < len; i++)
    {
		if (!Settings::Ragebot::AutoAim::desiredBones[i])
	   		continue;

		int boneID = (*modelType).at(i);

		if (boneID == BONE_INVALID) // bone not available on this modeltype.
	    	continue;

		bool VisiblityCheck = Entity::IsVisible(player, boneID, FOV, false);
		float playerHelth = player->GetHealth();

	// If we found head here
	if (i == BONE_HEAD) // head multipoint
	{
	    Vector headPoints[MultiVectors];
	    if (!HeadMultiPoint(player, headPoints, BoneMatrix))
			continue;

	    // cheaking for all head vectors
	    for (int j = 0; j < MultiVectors; j++)
	    {
			Autowall::FireBulletData data;
			float spotDamage = Autowall::GetDamage(headPoints[j], !Settings::Ragebot::friendly, data);
/*
			if (spotDamage < minDamage && spotDamage < minDamageVisible)
				continue;

			if (spotDamage <= prevSpotDamage)
				continue;
*/
			if ( spotDamage > 0.f && !EnemyPresent ) 
		    	EnemyPresent = true;

			if ( VisiblityCheck ) // cheking if the enemy  is visible
			{
				if (spotDamage >= playerHelth)
				{
		    		VisibleDamage = spotDamage;
		    		visibleSpot = headPoints[j];
		    		prevSpotDamage = 0;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamageVisible && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = wallbangdamage = spotDamage;
		    		wallbangspot = headPoints[j];
				}
			}
			else
			{
				if (spotDamage >= playerHelth)
				{
		    		wallbangspot = headPoints[j];
		    		wallbangdamage = spotDamage;
		    		prevSpotDamage = 0;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamage && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = VisibleDamage = spotDamage;
		    		visibleSpot = headPoints[j];
				}
			}
	    }
	}
	
	else if (i == BONE_UPPER_SPINAL_COLUMN) // head multipoint
	{
	    Vector upperChest[MultiVectors];
	    if (!UpperChestMultiPoint(player, upperChest, BoneMatrix))
		continue;

	    // cheaking for all head vectors
	    for (int j = 0; j < MultiVectors; j++)
	    {
			Autowall::FireBulletData data;
			float spotDamage = Autowall::GetDamage(upperChest[j], !Settings::Ragebot::friendly, data);

/*
			if (spotDamage < minDamage && spotDamage < minDamageVisible)
				continue;

			if (spotDamage <= prevSpotDamage)
				continue;
*/
			if ( spotDamage > 0.f && !EnemyPresent ) 
		    	EnemyPresent = true;

			if ( VisiblityCheck ) // cheking if the enemy  is visible
			{
				if (spotDamage >= playerHelth)
				{
		    		VisibleDamage = spotDamage;
		    		visibleSpot = upperChest[j];
		    		prevSpotDamage = 0;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamageVisible && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = wallbangdamage = spotDamage;
		    		wallbangspot = upperChest[j];
				}
			}
			else
			{
				if (spotDamage >= playerHelth)
				{
		    		wallbangspot = upperChest[j];
		    		wallbangdamage = spotDamage;
		    		prevSpotDamage = 0;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamage && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = VisibleDamage = spotDamage;
		    		visibleSpot = upperChest[j];
				}
			}
	    }
	}
	
	else if (i == BONE_MIDDLE_SPINAL_COLUMN) // head multipoint
	{
	    Vector MiddleChest[MultiVectors];
	    if (!ChestMultiPoint(player, MiddleChest, BoneMatrix))
			continue;

		for (int j = 0; j < MultiVectors; j++)
	    {
			Autowall::FireBulletData data;
			float spotDamage = Autowall::GetDamage(MiddleChest[j], !Settings::Ragebot::friendly, data);
/*
			if (spotDamage < minDamage && spotDamage < minDamageVisible)
				continue;

			if (spotDamage <= prevSpotDamage)
				continue;
*/
			if ( spotDamage > 0.f && !EnemyPresent ) 
		    	EnemyPresent = true;

			if ( VisiblityCheck ) // cheking if the enemy  is visible
			{
				if (spotDamage >= playerHelth)
				{
		    		VisibleDamage = spotDamage;
		    		visibleSpot = MiddleChest[j];
		    		prevSpotDamage = 0;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamageVisible && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = wallbangdamage = spotDamage;
		    		wallbangspot = MiddleChest[j];
				}
			}
			else
			{
				if (spotDamage >= playerHelth)
				{
		    		wallbangspot = MiddleChest[j];
		    		wallbangdamage = spotDamage;
		    		prevSpotDamage = 0;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamage && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = VisibleDamage = spotDamage;
		    		visibleSpot = MiddleChest[j];
				}
			}
	    }
	}
	
	else if (i == BONE_UPPER_SPINAL_COLUMN) // head multipoint
	{
	    Vector LowerChest[MultiVectors];
	    if (!LowerChestMultiPoint(player, LowerChest, BoneMatrix))
			continue;
	    
		for (int j = 0; j < MultiVectors; j++)
	    {
			Autowall::FireBulletData data;
			float spotDamage = Autowall::GetDamage(LowerChest[j], !Settings::Ragebot::friendly, data);
/*
			if (spotDamage < minDamage && spotDamage < minDamageVisible)
				continue;

			if (spotDamage <= prevSpotDamage)
				continue;
*/
			if ( spotDamage > 0.f && !EnemyPresent ) 
		    	EnemyPresent = true;

			if ( VisiblityCheck ) // cheking if the enemy  is visible
			{
				if (spotDamage >= playerHelth)
				{
		    		VisibleDamage = spotDamage;
		    		visibleSpot = LowerChest[j];
		    		prevSpotDamage = 0;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamageVisible && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = wallbangdamage = spotDamage;
		    		wallbangspot = LowerChest[j];
				}
			}
			else
			{
				if (spotDamage >= playerHelth)
				{
		    		wallbangspot = LowerChest[j];
		    		wallbangdamage = spotDamage;
		    		prevSpotDamage = 0;
		    		return;
				}
				if (spotDamage > 0.f && spotDamage >= minDamage && spotDamage > prevSpotDamage)
				{
		    		prevSpotDamage = VisibleDamage = spotDamage;
		    		visibleSpot = LowerChest[j];
				}
			}
	    }
	}
	
	else 
	{
		Vector bone3D = player->GetBonePosition(boneID);

		Autowall::FireBulletData data;
		float boneDamage = Autowall::GetDamage(bone3D, !Settings::Ragebot::friendly, data);
		
/*
		if (boneDamage < minDamage && boneDamage < minDamageVisible)
				continue;

		if ( boneDamage > 0.f && boneDamage <= prevSpotDamage)
			continue;
		*/
		if ( boneDamage > 0.f && !EnemyPresent ) 
		    	EnemyPresent = true;

		if (VisiblityCheck)
		{
			if (boneDamage >= playerHelth)
			{
				visibleSpot = bone3D;
				VisibleDamage = boneDamage;
				prevSpotDamage = 0.f;
				return;
			}
			if (boneDamage > prevSpotDamage && boneDamage >= minDamageVisible && boneDamage > 0.f)
	    	{
				visibleSpot = bone3D;
				prevSpotDamage = VisibleDamage = boneDamage;
	    	}
		}
		else
		{
			if (boneDamage >= playerHelth)
			{
				wallbangspot = bone3D;
				wallbangdamage = boneDamage;
				prevSpotDamage = 0.f;
				return;
			}
			if (boneDamage > prevSpotDamage && boneDamage >= minDamage && boneDamage > 0.f)
			{
	    		wallbangspot = bone3D;
	    		prevSpotDamage = wallbangdamage = boneDamage;
			}
		}
		
     }
	}

}


/*
** Get best Damage from the enemy and the spot
*/
static void GetBestSpotAndDamage(C_BasePlayer* player, Vector& wallBangSpot, float& WallbangDamage, Vector& visibleSPot, float& VisibleDamage)
{

	if (Settings::Ragebot::damagePrediction == DamagePrediction::safety)
	{
		safetyPrediction(player, wallBangSpot, WallbangDamage, visibleSPot, VisibleDamage);
		return;
	}
	else if (Settings::Ragebot::damagePrediction == DamagePrediction::damage)
	{
		BestDamagePrediction(player, wallBangSpot, WallbangDamage, visibleSPot, VisibleDamage);
		return;
	}    
	return;
}


static void RagebotNoShoot(C_BaseCombatWeapon* activeWeapon, C_BasePlayer* player, CUserCmd* cmd)
{
	if (player && Settings::Legitbot::NoShoot::enabled)
	{
		if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_C4)
			return;

		if (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER)
			cmd->buttons &= ~IN_ATTACK2;
		else
			cmd->buttons &= ~IN_ATTACK;
	}
}

static Vector VelocityExtrapolate(C_BasePlayer* player, Vector aimPos)
{
    return aimPos + (player->GetVelocity() * globalVars->interval_per_tick);
}

static C_BasePlayer* GetClosestPlayerAndSpot(CUserCmd* cmd, Vector* bestSpot, float* bestDamage, AimTargetType aimTargetType = AimTargetType::FOV)
{
	
    C_BasePlayer* localplayer = (C_BasePlayer*)entityList->GetClientEntity(engine->GetLocalPlayer());
    C_BasePlayer* closestEntity = nullptr;

    float WallBangdamage = NULL, VisibleDamage = NULL;
    float bestFov = Settings::Ragebot::AutoAim::fov;

    for (int i = 1; i < engine->GetMaxClients(); ++i)
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

	if (!Ragebot::friends.empty()) // check for friends, if any
	{
	    IEngineClient::player_info_t entityInformation;
	    engine->GetPlayerInfo(i, &entityInformation);

	    if (std::find(Ragebot::friends.begin(), Ragebot::friends.end(), entityInformation.xuid) != Ragebot::friends.end())
		continue;
	}

	Vector wallBangSpot = { NULL, NULL, NULL },
	       VisibleSpot = { NULL, NULL, NULL };

	GetBestSpotAndDamage(player, wallBangSpot, WallBangdamage, VisibleSpot, VisibleDamage);

	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*)entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());

	float playerHelth = player->GetHealth();

	if (WallBangdamage > VisibleDamage && WallBangdamage > 0.f)
	{
	    if ( !wallBangSpot.IsZero() )
	    {
			if (WallBangdamage >= playerHelth)
			{
		    	//cvar->ConsoleDPrintf(XORSTR("damage in going to kill the enemy\n"));
		    	*bestDamage = WallBangdamage;
		    	*bestSpot = wallBangSpot;
		    	closestEntity = player;
		    	lastRayEnd = wallBangSpot;
				prevSpotDamage = 0.f;
		    	return closestEntity;
			}
		   	//cvar->ConsoleDPrintf(XORSTR("in wall bang not enmply \n"));
		    *bestDamage = WallBangdamage;
		    *bestSpot = wallBangSpot;
		    closestEntity = player;
		    lastRayEnd = wallBangSpot;

	    }
	}
	else if ( VisibleDamage >= WallBangdamage && VisibleDamage > 0.f)
	{
		if ( !VisibleSpot.IsZero() )
		{
	    	if (VisibleDamage >= playerHelth)
	    	{
				*bestDamage = VisibleDamage;
				*bestSpot = VisibleSpot;
				closestEntity = player;
				lastRayEnd = VisibleSpot;
				prevSpotDamage = 0.f;
				return closestEntity;
	    	}

			*bestDamage = VisibleDamage;
			*bestSpot = VisibleSpot;
			closestEntity = player;
			lastRayEnd = VisibleSpot;
	   	}
	}
    }
    if (bestSpot->IsZero() || *bestDamage < 1.f)
    {
		return nullptr;
    }

    return closestEntity;
}

//Hitchance
static bool Ragebothitchance(C_BasePlayer* localplayer, C_BaseCombatWeapon* activeWeapon)
{
    float hitchance = 101;

    if (activeWeapon)
    {
	activeWeapon->UpdateAccuracyPenalty();
	float AuccuracyPenalty = activeWeapon->GetAccuracyPenalty();
	float inaccuracy = activeWeapon->GetInaccuracy();
	float weaponspread = activeWeapon->GetSpread();

	if (inaccuracy == 0)
	    inaccuracy = 0.0000001;
	if (AuccuracyPenalty == 0)
	    AuccuracyPenalty = 0.0000001;

	hitchance = 1 / (inaccuracy + weaponspread);

	if (Settings::Ragebot::HitChanceOverwrride::enable)
	{
	    return (hitchance >= Settings::Ragebot::HitChance::value * Settings::Ragebot::HitChanceOverwrride::value);
	}

	return hitchance >= Settings::Ragebot::HitChance::value * 1.2f;
    }
}

static void RagebotRCS(QAngle& angle, C_BasePlayer* player, CUserCmd* cmd, C_BasePlayer* localplayer, C_BaseCombatWeapon* activeWeapon)
{

    if (!(cmd->buttons & IN_ATTACK))
	return;

    static QAngle RagebotRCSLastPunch = { 0, 0, 0 };
    bool hasTarget = RagebotShouldAim && player;

    float aimpunch = cvar->FindVar("weapon_recoil_scale")->GetFloat();
    QAngle CurrentPunch = *localplayer->GetAimPunchAngle();

    if (Settings::Ragebot::silent || hasTarget)
    {
	angle.x -= CurrentPunch.x * 2.0f;
	angle.y -= CurrentPunch.y * 2.0f;
    }
    else if (aimpunch)
    {
	QAngle NewPunch = { CurrentPunch.x - RagebotRCSLastPunch.x, CurrentPunch.y - RagebotRCSLastPunch.y, 0 };

	angle.x -= NewPunch.x * 2.0f;
	angle.y -= NewPunch.y * 2.0f;
    }

    RagebotRCSLastPunch = CurrentPunch;
}

static void RagebotAutoCrouch(C_BasePlayer* player, CUserCmd* cmd)
{
    if (!Settings::Ragebot::AutoCrouch::enabled)
	return;

    if (!player)
	return;

    cmd->buttons |= IN_BULLRUSH | IN_DUCK;
}

static void RagebotAutoSlow(C_BasePlayer* player, float& forward, float& sideMove, float& bestDamage, C_BaseCombatWeapon* active_weapon, CUserCmd* cmd)
{

    if (!Settings::Ragebot::AutoSlow::enabled || !player || !RagebotShouldAim)
    {
	return;
    }

    float nextPrimaryAttack = active_weapon->GetNextPrimaryAttack();

    if (nextPrimaryAttack > globalVars->curtime)
    {
	return;
    }

    C_BasePlayer* localplayer = (C_BasePlayer*)entityList->GetClientEntity(engine->GetLocalPlayer());

    C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*)entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
    if (!activeWeapon || activeWeapon->GetAmmo() == 0)
	return;

    if (Settings::Ragebot::HitChance::enabled && RagebotShouldAim)
    {
	if (!Ragebothitchance(localplayer, activeWeapon))
	{
	    cmd->buttons |= IN_WALK;
	    forward = 0;
	    sideMove = 0;
	    cmd->upmove = 0;
	    return;
	}
	else if (Ragebothitchance(localplayer, activeWeapon))
	{
	    cmd->buttons |= IN_WALK;
	}
	else
	{
	    return;
	}

	// Experimental items
	/*if (!(HitPercentage(localplayer, activeWeapon)))
		{
			cmd->buttons |= IN_WALK;
			forward = 0;
			sideMove = 0;
			cmd->upmove = 0;
			return;
		}*/
	/*if( !Ragebothitchance(localplayer, activeWeapon) && !(cmd->buttons & IN_WALK))
			{
				cmd->buttons |= IN_WALK;
				forward = -forward;
				sideMove = -sideMove;
				cmd->upmove = 0;
				return;
			}
			else if( Ragebothitchance(localplayer, activeWeapon) && !(cmd->buttons & IN_WALK)) {
				cmd->buttons |= IN_WALK;
				forward = 0;
				sideMove = 0;
				cmd->upmove = 0;
				return;
			}
			else if( !Ragebothitchance(localplayer, activeWeapon) && (cmd->buttons & IN_WALK))
			{
				forward = -forward;
				sideMove = -sideMove;
				cmd->upmove = 0;
				return;
			}
			else if( Ragebothitchance(localplayer, activeWeapon) && (cmd->buttons & IN_WALK))
			{
				forward = 0;
				sideMove = 0;
				cmd->upmove = 0;
				return;
			}
			else
			{
				return;
			}*/

	/*else if (cmd->buttons & IN_ATTACK) 
        {
            cmd->buttons |= IN_WALK;
			return;
        }*/
    }

    else if ((active_weapon->GetSpread() + active_weapon->GetInaccuracy()) > (activeWeapon->GetCSWpnData()->GetMaxPlayerSpeed() / 3.0f)) // https://youtu.be/ZgjYxBRuagA
    {
	cmd->buttons |= IN_WALK;
	forward = -forward;
	sideMove = -sideMove;
	cmd->upmove = 0;
	return;
    }
    else if ((active_weapon->GetSpread() + active_weapon->GetInaccuracy()) == (activeWeapon->GetCSWpnData()->GetMaxPlayerSpeed() / 3.0f))
    {
	cmd->buttons |= IN_WALK;
	forward = 0;
	sideMove = 0;
	cmd->upmove = 0;
	return;
    }
}

static void RagebotAutoPistol(C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
    // if (!Settings::Ragebot::AutoPistol::enabled)
    // 	return;

    if (!activeWeapon || activeWeapon->GetCSWpnData()->GetWeaponType() != CSWeaponType::WEAPONTYPE_PISTOL)
	return;

    if (activeWeapon->GetNextPrimaryAttack() < globalVars->curtime)
	return;

    if (*activeWeapon->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_REVOLVER)
	cmd->buttons &= ~IN_ATTACK;
}

static void AutoCock(C_BasePlayer* player, C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
    if (!Settings::Ragebot::AutoShoot::enabled)
	return;

    if (*activeWeapon->GetItemDefinitionIndex() != ItemDefinitionIndex::WEAPON_REVOLVER)
	return;

    if (activeWeapon->GetAmmo() == 0)
	return;
    if (cmd->buttons & IN_USE)
	return;

    cmd->buttons |= IN_ATTACK;
    float postponeFireReadyTime = activeWeapon->GetPostPoneReadyTime();
    if (postponeFireReadyTime > 0)
    {
	if (postponeFireReadyTime < globalVars->curtime)
	{
	    if (player)
	    {
		Ragebot::coacking = false;
		return;
	    }

	    Ragebot::coacking = true;
	    cmd->buttons &= ~IN_ATTACK;
	}
    }
}

static void RagebotAutoShoot(C_BasePlayer* player, C_BaseCombatWeapon* activeWeapon, CUserCmd* cmd)
{
    //cvar->ConsoleDPrintf("I ma in auto shoot method \n");
    if (!Settings::Ragebot::AutoShoot::enabled)
	return;

    if (!player || activeWeapon->GetAmmo() == 0)
	return;

    //C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());

    CSWeaponType weaponType = activeWeapon->GetCSWpnData()->GetWeaponType();
    if (weaponType == CSWeaponType::WEAPONTYPE_KNIFE || weaponType == CSWeaponType::WEAPONTYPE_C4 || weaponType == CSWeaponType::WEAPONTYPE_GRENADE)
	return;

    if (cmd->buttons & IN_USE)
	return;

    C_BasePlayer* localplayer = (C_BasePlayer*)entityList->GetClientEntity(engine->GetLocalPlayer());

    if (Settings::Ragebot::AutoShoot::autoscope && Util::Items::IsScopeable(*activeWeapon->GetItemDefinitionIndex()) && !localplayer->IsScoped() && !(cmd->buttons & IN_ATTACK2))
    {
	cmd->buttons |= IN_ATTACK2;
	return; // continue next tick
    }

    if (Settings::Ragebot::HitChance::enabled && !Ragebothitchance(localplayer, activeWeapon))
    {
	return;
    }

    float nextPrimaryAttack = activeWeapon->GetNextPrimaryAttack();

    if (!(*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER))
    {
	if (nextPrimaryAttack > globalVars->curtime)
	{
	    cmd->buttons &= ~IN_ATTACK;
	    return;
	}
	else
	{
	    cmd->buttons |= IN_ATTACK;
	    return;
	}
    }
}

static void FixMouseDeltas(CUserCmd* cmd, const QAngle& angle, const QAngle& oldAngle)
{
    if (!RagebotShouldAim)
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

void Ragebot::CreateMove(CUserCmd* cmd)
{

    C_BasePlayer* localplayer = (C_BasePlayer*)entityList->GetClientEntity(engine->GetLocalPlayer());

    if (!localplayer || !localplayer->GetAlive())
    {
		RagebotShouldAim = false;
		return;
    }

    Ragebot::UpdateValues();

    QAngle oldAngle;
    engine->GetViewAngles(oldAngle);
    float oldForward = cmd->forwardmove;
    float oldSideMove = cmd->sidemove;

    QAngle angle = cmd->viewangles;
    Vector localEye = localplayer->GetEyePosition();

    C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*)entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
    if (!activeWeapon || activeWeapon->GetInReload())
	return;

    CSWeaponType weaponType = activeWeapon->GetCSWpnData()->GetWeaponType();
    if (weaponType == CSWeaponType::WEAPONTYPE_C4 || weaponType == CSWeaponType::WEAPONTYPE_GRENADE || weaponType == CSWeaponType::WEAPONTYPE_KNIFE)
	return;

    Vector bestSpot = { NULL, NULL, NULL };
    float bestDamage = 0.f;
    C_BasePlayer* player = GetClosestPlayerAndSpot(cmd, &bestSpot, &bestDamage);

    if (player)
    {
	//Auto Scop Controll system to controll auto scoping every time
	if (Settings::Ragebot::ScopeControl::enabled)
	{
	    //cheking if the weapon scopable and not scop then it will scop and go back to the next tick
	    if (Util::Items::IsScopeable(*activeWeapon->GetItemDefinitionIndex()) && !localplayer->IsScoped() && !(cmd->buttons & IN_ATTACK2))
	    {
			cmd->buttons |= IN_ATTACK2;
			return; // will go to the next tick
	    }
	}
	if (Settings::Ragebot::AutoShoot::enabled)
	{
	    RagebotShouldAim = true;
	}
	else if (cmd->buttons & IN_ATTACK)
	{
	    RagebotShouldAim = true;
	}

	Settings::Debug::AutoAim::target = bestSpot; // For Debug showing aimspot.
	if (RagebotShouldAim)
	{
	    if (Settings::Ragebot::Prediction::enabled)
	    {
			localEye = VelocityExtrapolate(localplayer, localEye); // get eye pos next tick
			bestSpot = VelocityExtrapolate(player, bestSpot); // get target pos next tick
	    }
	    angle = Math::CalcAngle(localEye, bestSpot);
	}
    }
    else if (EnemyPresent) // Just Increase the probrability of scoping for faster shooting in some cases
    {
		EnemyPresent = !EnemyPresent;
		if (Settings::Ragebot::AutoShoot::autoscope && Util::Items::IsScopeable(*activeWeapon->GetItemDefinitionIndex()) && !localplayer->IsScoped() && !(cmd->buttons & IN_ATTACK2))
		{
	    	cmd->buttons |= IN_ATTACK2;
	    	return;
		}
	Settings::Debug::AutoAim::target = { 0, 0, 0 };
	RagebotShouldAim = false;
    }
    else // No player to Shoot
    {
	Settings::Debug::AutoAim::target = { 0, 0, 0 };
	RagebotShouldAim = false;
	EnemyPresent = false;
    }

    RagebotAutoCrouch(player, cmd);
    RagebotAutoSlow(player, oldForward, oldSideMove, bestDamage, activeWeapon, cmd);
    RagebotAutoPistol(activeWeapon, cmd);
    RagebotAutoShoot(player, activeWeapon, cmd);
    AutoCock(player, activeWeapon, cmd);
    RagebotRCS(angle, player, cmd, localplayer, activeWeapon);
	RagebotNoShoot(activeWeapon, player, cmd);

    Math::NormalizeAngles(angle);
    Math::ClampAngles(angle);

    FixMouseDeltas(cmd, angle, oldAngle);
    cmd->viewangles = angle;

    Math::CorrectMovement(oldAngle, cmd, oldForward, oldSideMove);

    if (!Settings::Ragebot::silent)
		engine->SetViewAngles(cmd->viewangles);
}

void Ragebot::FireGameEvent(IGameEvent* event)
{
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

void Ragebot::UpdateValues()
{
    C_BasePlayer* localplayer = (C_BasePlayer*)entityList->GetClientEntity(engine->GetLocalPlayer());
    C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*)entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
    if (!activeWeapon)
	return;

    ItemDefinitionIndex index = ItemDefinitionIndex::INVALID;
    if (Settings::Ragebot::weapons.find(*activeWeapon->GetItemDefinitionIndex()) != Settings::Ragebot::weapons.end())
	index = *activeWeapon->GetItemDefinitionIndex();

    const RagebotWeapon_t& currentWeaponSetting = Settings::Ragebot::weapons.at(index);

    Settings::Ragebot::silent = currentWeaponSetting.silent;
    Settings::Ragebot::friendly = currentWeaponSetting.friendly;
    Settings::Ragebot::AutoAim::fov = currentWeaponSetting.RagebotautoAimFov;
    Settings::Ragebot::AutoPistol::enabled = currentWeaponSetting.autoPistolEnabled;
    Settings::Ragebot::AutoShoot::enabled = currentWeaponSetting.autoShootEnabled;
    Settings::Ragebot::AutoShoot::autoscope = currentWeaponSetting.autoScopeEnabled;
    Settings::Ragebot::HitChance::enabled = currentWeaponSetting.HitChanceEnabled;
    Settings::Ragebot::HitChance::value = currentWeaponSetting.HitChance;
    Settings::Ragebot::HitChanceOverwrride::enable = currentWeaponSetting.HitChanceOverwrriteEnable;
    Settings::Ragebot::HitChanceOverwrride::value = currentWeaponSetting.HitchanceOverwrriteValue;
    Settings::Ragebot::AutoWall::value = currentWeaponSetting.autoWallValue;
    Settings::Ragebot::visibleDamage = currentWeaponSetting.visibleDamage;
    Settings::Ragebot::AutoSlow::enabled = currentWeaponSetting.autoSlow;
    Settings::Ragebot::ScopeControl::enabled = currentWeaponSetting.scopeControlEnabled;
	Settings::Ragebot::damagePrediction = currentWeaponSetting.DmagePredictionType;

    for (int bone = BONE_PELVIS; bone <= BONE_RIGHT_SOLE; bone++)
	Settings::Ragebot::AutoAim::desiredBones[bone] = currentWeaponSetting.desiredBones[bone];
}
