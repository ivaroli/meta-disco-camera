/*=============================================================================
  Copyright (C) 2012 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        Mutex.cpp

  Description: Implementation of class VmbCPP::Mutex.

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

#include <new>

#include <VmbCPP/Mutex.h>
#include <VmbCPP/LoggerDefines.h>


namespace VmbCPP {

Mutex::Mutex( bool bInitLock )
#ifdef _WIN32
    :   m_hMutex( nullptr )
#endif
{
#ifdef _WIN32
    m_hMutex = CreateMutex( nullptr, FALSE, nullptr );
    if( nullptr == m_hMutex )
    {
        LOG_FREE_TEXT( "Could not create mutex." );
        throw std::bad_alloc();
    }
#else
    pthread_mutex_init(&m_Mutex, nullptr);
#endif

    if( true == bInitLock )
    {
        Lock();
    }
}

Mutex::~Mutex()
{  
#ifdef _WIN32
    CloseHandle( m_hMutex );
#else
    pthread_mutex_destroy(&m_Mutex);
#endif
}

void Mutex::Lock()
{
#ifdef _WIN32
    WaitForSingleObject( m_hMutex, INFINITE );
#else
    pthread_mutex_lock( &m_Mutex );
#endif
}

void Mutex::Unlock()
{  
#ifdef _WIN32
    ReleaseMutex( m_hMutex );
#else
    pthread_mutex_unlock( &m_Mutex );
#endif
}

}  // namespace VmbCPP
