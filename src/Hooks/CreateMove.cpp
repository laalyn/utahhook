#include "hooks.h"
#include <future>

#include "../interfaces.h"
#include "../settings.h"

#include "../Hacks/bhop.h"
#include "../Hacks/noduckcooldown.h"
#include "../Hacks/lagcomp.h"
#include "../Hacks/autostrafe.h"
#include "../Hacks/showranks.h"
#include "../Hacks/autodefuse.h"
#include "../Hacks/jumpthrow.h"
#include "../Hacks/clantagchanger.h"
#include "../Hacks/edgejump.h"
#include "../Hacks/esp.h"
#include "../Hacks/fakeduck.h"
#include "../Hacks/fakelag.h"
#include "../Hacks/grenadehelper.h"
#include "../Hacks/grenadeprediction.h"
#include "../Hacks/edgejump.h"
#include "../Hacks/autoblock.h"
#include "../Hacks/predictionsystem.h"
#include "../Hacks/ragebot.h"
#include "../Hacks/legitbot.h"
#include "../Hacks/triggerbot.h"
#include "../Hacks/autoknife.h"
#include "../Hacks/antiaim.h"
#include "../Hacks/fakelag.h"
#include "../Hacks/esp.h"
#include "../Hacks/tracereffect.h"
#include "../Hacks/nofall.h"
#include "../Hacks/ragdollgravity.h"

bool safeMode = false;

typedef bool (*CreateMoveFn) (void*, float, CUserCmd*);

bool Hooks::CreateMove(void* thisptr, float flInputSampleTime, CUserCmd* cmd)
{
    clientModeVMT->GetOriginalMethod<CreateMoveFn>(25)(thisptr, flInputSampleTime, cmd);

    if (cmd && cmd->command_number)
    {
	// Special thanks to Gre-- I mean Heep ( https://www.unknowncheats.me/forum/counterstrike-global-offensive/290258-updating-bsendpacket-linux.html )
        uintptr_t rbp;
        asm volatile("mov %%rbp, %0" : "=r" (rbp));
        bool *sendPacket = ((*(bool **)rbp) - 0x18);

	C_BasePlayer* localplayer = (C_BasePlayer*)entityList->GetClientEntity(engine->GetLocalPlayer());
	static bool suppressDeadNotif = false;
	if (!localplayer || !localplayer->GetAlive())
	{
	    *sendPacket = true;
	    CreateMove::chokeStack = 0;
	    CreateMove::sendPacket = false;
	    // if (!suppressDeadNotif) cvar->ConsoleDPrintf("[CreateMove] sendPacket enabled, you are dead\n");
	    suppressDeadNotif = true;
	} else
	{
	    // let it stack up, normal behavior
	    *sendPacket = safeMode == true ? true : false;
	    suppressDeadNotif = false;
	}

	// gets reset because it only chokes current packet
	CreateMove::sendPacket = true;

	/* run code that affects movement before prediction */
	BHop::CreateMove(cmd);
	FakeDuck::CreateMove(cmd);
	NoDuckCooldown::CreateMove(cmd);
	AutoStrafe::CreateMove(cmd);
	ShowRanks::CreateMove(cmd);
	AutoDefuse::CreateMove(cmd);
	JumpThrow::CreateMove(cmd);
    	GrenadeHelper::CreateMove(cmd);
	GrenadePrediction::CreateMove(cmd);
       	EdgeJump::PrePredictionCreateMove(cmd);
	Autoblock::CreateMove(cmd);
	NoFall::PrePredictionCreateMove(cmd);

	PredictionSystem::StartPrediction(cmd);
	    if (Settings::Legitbot::enabled)
		Legitbot::CreateMove(cmd);
	    else if ( Settings::Ragebot::enabled)
		Ragebot::CreateMove(cmd);
	    LagComp::CreateMove(cmd);
	    AutoKnife::CreateMove(cmd);
	    AntiAim::CreateMove(cmd);
	    FakeLag::CreateMove(cmd);
	    ESP::CreateMove(cmd);
	    TracerEffect::CreateMove(cmd);
	    RagdollGravity::CreateMove(cvar);
	PredictionSystem::EndPrediction();

    	EdgeJump::PostPredictionCreateMove(cmd);
    	NoFall::PostPredictionCreateMove(cmd);

	static bool suppressNotChokingNotif = false;
	// chocking
	if (!CreateMove::sendPacket)
	{
	    *sendPacket = safeMode == true ? true : false;
        if (*sendPacket == false) CreateMove::chokeStack++;
	    // cvar->ConsoleDPrintf("[RageBot -> CreateMove] hiding shot {tick: %d}\n", CreateMove::tickCnt);
	    cvar->ConsoleDPrintf("[CreateMove] choke meter: ");
	    for (int i = 1; i <= CreateMove::chokeStack; i++) {
		if (i > 15) {
		    cvar->ConsoleDPrintf("[!!] ", i);
		} else {
		    cvar->ConsoleDPrintf("[%d] ", i);
		}
	    }
	    cvar->ConsoleDPrintf("{tick: %d}\n", CreateMove::tickCnt);
	    suppressNotChokingNotif = false;
	}
	// sending
	else
	{
	    *sendPacket = true;
	    CreateMove::chokeStack = 0;
	    // cvar->ConsoleDPrintf("[CreateMove] sending <+> {tick: %d}\n", CreateMove::tickCnt);
	    if (!suppressNotChokingNotif) cvar->ConsoleDPrintf("[CreateMove] choke meter: {tick: %d}\n", CreateMove::tickCnt);
	    suppressNotChokingNotif = true;
	}

	if (CreateMove::sendPacket) {
            CreateMove::lastTickViewAngles = cmd->viewangles;
        }

	CreateMove::tickCnt++;
	if (CreateMove::tickCnt > 1024) {
	    CreateMove::tickCnt = 0;
	}
    }

	return false;
}
