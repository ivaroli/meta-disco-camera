/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        MutexGuard.h

  Description: Definition of a mutex helper class for locking and unlocking.
               Intended for use in the implementation of VmbCPP.

-------------------------------------------------------------------------------

  THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF TITLE,
  NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR  PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, 
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED  
  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================*/

#ifndef VMBCPP_MUTEXGUARD
#define VMBCPP_MUTEXGUARD

/**
* \file        MutexGuard.h
*
* \brief       Definition of a mutex helper class for locking and unlocking.
*              Intended for use in the implementation of VmbCPP.
*/

#include <VmbCPP/BasicLockable.h>


namespace VmbCPP
{
class Mutex;

class MutexGuard
{
  public:
    MutexGuard() noexcept;
    MutexGuard( MutexPtr &pMutex );
    MutexGuard( const BasicLockablePtr& pLockable );
    MutexGuard( const BasicLockable &rLockable );
    ~MutexGuard();

    MutexGuard(const MutexGuard&) = delete;
    MutexGuard& operator=(const MutexGuard&) = delete;

    void Protect();
    bool Release();

  protected:
    Mutex  *m_pMutex;
};

} //namespace VmbCPP


#endif //VMBCPP_MUTEXGUARD
