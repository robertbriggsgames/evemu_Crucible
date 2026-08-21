#ifndef EVEMU_MODULE_HOOKS_H
#define EVEMU_MODULE_HOOKS_H

class EVEServiceManager;

// Generic in-process module hooks. Core never names a specific add-on.
// Optional modules (cloned into modules/) provide strong definitions of
// EvEmu_Module_OnServerReady / OnTick / OnShutdown. Weak stubs here keep
// a vanilla build working when modules/ is empty.

void EvEmu_Modules_OnServerReady(EVEServiceManager* svc);
void EvEmu_Modules_OnTick();
void EvEmu_Modules_OnShutdown();

#endif
