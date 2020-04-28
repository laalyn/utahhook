#include "ragdollgravity.h"


void RagdollGravity::CreateMove(ICvar* cvar)
{
    if (!Settings::RagdollGravity::enabled)
        return;
    static auto ragdollGravity = cvar->FindVar("cl_ragdoll_gravity");
    //findVar("cl_ragdoll_gravity");
    ragdollGravity->SetValue(-600);
    //ragdollGravity->setValue(config->visuals.inverseRagdollGravity ? -600 : 600);
}