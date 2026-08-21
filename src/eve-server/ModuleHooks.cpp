#include "eve-server.h"
#include "ModuleHooks.h"
#include "services/ServiceManager.h"

extern "C" {

#if defined(__GNUC__)
__attribute__((weak))
#endif
void EvEmu_Module_OnServerReady(EVEServiceManager* svc)
{
    (void)svc;
}

#if defined(__GNUC__)
__attribute__((weak))
#endif
void EvEmu_Module_OnTick()
{
}

#if defined(__GNUC__)
__attribute__((weak))
#endif
void EvEmu_Module_OnShutdown()
{
}

}

void EvEmu_Modules_OnServerReady(EVEServiceManager* svc)
{
    EvEmu_Module_OnServerReady(svc);
}

void EvEmu_Modules_OnTick()
{
    EvEmu_Module_OnTick();
}

void EvEmu_Modules_OnShutdown()
{
    EvEmu_Module_OnShutdown();
}
