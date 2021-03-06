/*
 * Copyright © BeyondTrust Software 2004 - 2019
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * BEYONDTRUST MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING TERMS AS
 * WELL. IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT WITH
 * BEYONDTRUST, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE TERMS OF THAT
 * SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE APACHE LICENSE,
 * NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU HAVE QUESTIONS, OR WISH TO REQUEST
 * A COPY OF THE ALTERNATE LICENSING TERMS OFFERED BY BEYONDTRUST, PLEASE CONTACT
 * BEYONDTRUST AT beyondtrust.com/contact
 */

/*
 * Module Name:
 *
 *        affinity.h
 *
 * Abstract:
 *
 *        CPU affinity operations
 *
 * Authors: Brian Koropoff (bkoropoff@likewisesoftware.com)
 *          Evgeny Popovich (epopovich@likewise.com)
 *
 */

#ifndef __LWBASE_AFFINITY_H__
#define __LWBASE_AFFINITY_H__

#include <pthread.h>

#include <lw/types.h>
#include <lw/attrs.h>
#include <lw/ntstatus.h>

ULONG
LwRtlGetCpuCount(
    VOID
    );

NTSTATUS
LwRtlSetAffinityThreadAttribute(
    pthread_attr_t* pAttr,
    ULONG CpuNumber
    );

NTSTATUS
LwRtlResetAffinityThreadAttribute(
    pthread_attr_t* pAttr
    );

#endif  // __LWBASE_AFFINITY_H__
