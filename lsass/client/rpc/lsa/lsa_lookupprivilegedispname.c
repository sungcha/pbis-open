/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4 -*-
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * Editor Settings: expandtabs and use 4 spaces for indentation */

/*
 * Copyright Likewise Software    2004-2011
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

/*
 * Copyright (C) Likewise Software. All rights reserved.
 *
 * Module Name:
 *
 *        lsa_lookupprivilegedispname.c
 *
 * Abstract:
 *
 *        Remote Procedure Call (RPC) Client Interface
 *
 *        LsaLookupPrivilegeDisplayName function
 *
 * Authors: Rafal Szczesniak (rafal@likewise.com)
 */

#include "includes.h"


NTSTATUS
LsaLookupPrivilegeDisplayName(
    IN LSA_BINDING      hBinding,
    IN POLICY_HANDLE    hPolicy,
    IN PWSTR            pwszName,
    IN SHORT            ClientLanguage,
    IN SHORT            ClientSystemLanguage,
    OUT PWSTR          *ppwszDisplayName,
    OUT PUSHORT          pLanguage
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    DWORD err = ERROR_SUCCESS;
    UNICODE_STRING Name = {0};
    PUNICODE_STRING pDisplayName = NULL;
    PWSTR pwszDisplayName = NULL;
    USHORT Language = 0;

    BAIL_ON_INVALID_PTR(hBinding, ntStatus);
    BAIL_ON_INVALID_PTR(hPolicy, ntStatus);
    BAIL_ON_INVALID_PTR(pwszName, ntStatus);
    BAIL_ON_INVALID_PTR(ppwszDisplayName, ntStatus);
    BAIL_ON_INVALID_PTR(pLanguage, ntStatus);

    err = LwAllocateUnicodeStringFromWc16String(
                     &Name,
                     pwszName);
    BAIL_ON_LSA_ERROR(err);

    DCERPC_CALL(ntStatus, cli_LsaLookupPrivilegeDisplayName(
                              (handle_t)hBinding,
                              hPolicy,
                              &Name,
                              ClientLanguage,
                              ClientSystemLanguage,
                              &pDisplayName,
                              &Language));
    BAIL_ON_NT_STATUS(ntStatus);

    ntStatus = LsaRpcAllocateMemory(
                     OUT_PPVOID(&pwszDisplayName),
                     pDisplayName->Length + sizeof(pDisplayName->Buffer[0]));
    BAIL_ON_NT_STATUS(ntStatus);

    memcpy(pwszDisplayName, pDisplayName->Buffer, pDisplayName->Length);

    *ppwszDisplayName = pwszDisplayName;
    *pLanguage        = Language;

error:
    if (ntStatus)
    {
        if (ppwszDisplayName)
        {
            *ppwszDisplayName = NULL;
        }

        if (pLanguage)
        {
            *pLanguage = 0;
        }
    }

    LwFreeUnicodeString(&Name);

    if (pDisplayName)
    {
        LsaFreeStubUnicodeString(pDisplayName);
    }

    return ntStatus;
}
