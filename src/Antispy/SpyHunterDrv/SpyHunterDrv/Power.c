/* 
 * Copyright (c) [2010-2019] zhenfei.mzf@gmail.com rights reserved.
 * 
 * AntiSpy is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *
 *     http://license.coscl.org.cn/MulanPSL
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR
 * FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v1 for more details.
*/
#include "Power.h"
#include "InitWindows.h"
#include "..\\..\\Common\\Common.h"
#include "CommonFunction.h"

NTSTATUS PowerOffOrReboot(POWER_TYPE PowerType)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	pfnNtShutdownSystem MzfNtShutdownSystem = (pfnNtShutdownSystem)GetGlobalVeriable(enumNtShutdownSystem);
	pfnPsGetCurrentThread MzfPsGetCurrentThread = (pfnPsGetCurrentThread)GetGlobalVeriable(enumPsGetCurrentThread);

	if (MzfNtShutdownSystem && MzfPsGetCurrentThread)
	{
		PETHREAD pThread = MzfPsGetCurrentThread();
		CHAR PreMode = ChangePreMode(pThread);

		if (PowerType == ePower_Reboot)
		{
			status = MzfNtShutdownSystem(ShutdownReboot);
		}
		else
		{
			status = MzfNtShutdownSystem(ShutdownPowerOff);
		}

		RecoverPreMode(pThread, PreMode);
	}

	return status;
}

void ForceReboot()
{
	__asm mov dx, 0x64
	__asm mov al, 0xFE
	__asm out dx, al
}

NTSTATUS Powers(PVOID pInBuffer, ULONG uInSize, PVOID pOutBuffer, ULONG nOutLen, ULONG* dwRet)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_POWER pCf = (PCOMMUNICATE_POWER)pInBuffer;

	if (uInSize != sizeof(COMMUNICATE_POWER))
	{
		return status;
	}

	switch (pCf->PowerType)
	{
	case ePower_Reboot:
		status = PowerOffOrReboot(ePower_Reboot);
		break;

	case ePower_Force_Reboot:
		ForceReboot();
		break;

	case ePower_PowerOff:
		status = PowerOffOrReboot(ePower_PowerOff);
		break;

	default:
		status = STATUS_UNSUCCESSFUL;
		break;
	}

	return status;
}