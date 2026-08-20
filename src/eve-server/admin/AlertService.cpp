/*
    ------------------------------------------------------------------------------------
    LICENSE:
    ------------------------------------------------------------------------------------
    This file is part of EVEmu: EVE Online Server Emulator
    Copyright 2006 - 2021 The EVEmu Team
    For the latest information visit https://evemu.dev
    ------------------------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by the Free Software
    Foundation; either version 2 of the License, or (at your option) any later
    version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
    this program; if not, write to the Free Software Foundation, Inc., 59 Temple
    Place - Suite 330, Boston, MA 02111-1307, USA, or go to
    http://www.gnu.org/copyleft/lesser.txt.
    ------------------------------------------------------------------------------------
    Author:        Zhur, Captnoord
*/

#include "eve-server.h"

#include "EVEServerConfig.h"
#include "admin/AlertService.h"

// Stable error ID returned to Crucible clients. Returning PyNone here forces
// the client LogAlertServiceStream tasklet to stream stacks immediately; that
// path does not match 360229 packet handling and exception-loops the overlay.
static const int32 ALERT_ERROR_ID = 34135;
static const int32 ALERT_LOGGING_MODE_LOCAL = 0;

AlertService::AlertService() :
    Service("alert"),
    traceLogger(nullptr)
{
    this->Add("BeanCount", &AlertService::BeanCount);
    this->Add("BeanDelivery", &AlertService::BeanDelivery);
    this->Add("GroupBeanDelivery", &AlertService::GroupBeanDelivery);
    // Accept any argument shape from Crucible 360229; typed handlers throw on mismatch.
    this->Add("SendClientStackTraceAlert", &AlertService::SendClientStackTraceAlert);

    if (sConfig.debug.StackTrace or is_log_enabled(CLIENT__STACK_TRACE))
        traceLogger = new PyTraceLog("evemu_client_stack_trace.txt", true, true);
}

AlertService::~AlertService()
{
    SafeDelete(traceLogger);
}

/** BeanCount: client asks how to handle a Python error.
  * Always return a stable integer errorID + loggingMode=0 so Crucible batches
  * stacks via BeanDelivery instead of opening LogAlertServiceStream.
  * Do NOT return PyNone for test servers — that is what causes the overlay loop.
  */
PyResult AlertService::BeanCount(PyCallArgs &call, PyRep* ignored) {
    _log(CLIENT__WARNING, "AlertService::Handle_BeanCount(): size=%lli", call.tuple->size());

    PyTuple *result = new PyTuple(2);
    result->items[0] = new PyInt(ALERT_ERROR_ID);
    result->items[1] = new PyInt(ALERT_LOGGING_MODE_LOCAL);

    sLog.Warning("AlertService", "BeanCount -> errorID=%d loggingMode=%d (batched, no immediate stream)",
                 ALERT_ERROR_ID, ALERT_LOGGING_MODE_LOCAL);

    return (PyRep*)result;
}

/** Batched client stack traces every ~15 minutes when BeanCount returns a real errorID. */
PyResult AlertService::BeanDelivery(PyCallArgs& call)
{
    _log(CLIENT__WARNING, "AlertService::Handle_BeanDelivery(): size=%lli", call.tuple->size());
    // Intentionally no-op: we do not persist batched client stacks.
    // Always succeed so the client reporter cannot exception-loop.
    return PyStatic.NewNone();
}

PyResult AlertService::GroupBeanDelivery(PyCallArgs& call)
{
    _log(CLIENT__WARNING, "AlertService::Handle_GroupBeanDelivery(): size=%u", call.tuple->size() );
    return PyStatic.NewNone();
}

/**
 * Immediate stack dump from client when BeanCount used to return None.
 * Accept any tuple shape; never throw. Optional file logging when enabled.
 */
PyResult AlertService::SendClientStackTraceAlert(PyCallArgs &call) {
    _log(CLIENT__WARNING, "AlertService::Handle_SendClientStackTraceAlert(): size=%lli", call.tuple->size());

    try {
        if ((sConfig.debug.StackTrace or is_log_enabled(CLIENT__STACK_TRACE)) and (traceLogger != nullptr) and (call.tuple != nullptr))
            traceLogger->logTrace(*call.tuple);
        else if (call.tuple != nullptr)
            sLog.Warning("AlertService", "SendClientStackTraceAlert received %lli args (logged summary only; StackTrace disabled).",
                         call.tuple->size());
    }
    catch (...) {
        sLog.Error("AlertService", "SendClientStackTraceAlert: ignored exception while logging client stack.");
    }

    // Client assigns return value into stacktraceLogMode; None is valid.
    return PyStatic.NewNone();
}
