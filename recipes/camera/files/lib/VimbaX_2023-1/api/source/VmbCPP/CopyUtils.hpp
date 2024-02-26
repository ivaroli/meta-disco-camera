/*=============================================================================
  Copyright (C) 2022 Allied Vision Technologies.  All Rights Reserved.

  Redistribution of this file, in original or modified form, without
  prior written consent of Allied Vision Technologies is prohibited.

-------------------------------------------------------------------------------

  File:        CopyUtils.h

  Description: Helper functionality for copying from std containers to buffers.

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

#ifndef VMBCPP_COPY_UTILS_H
#define VMBCPP_COPY_UTILS_H

#include <algorithm>
#include <cstring>

#include "VmbC/VmbCommonTypes.h"

/**
* \file     CopyUtils.h
*
* \brief    Helper functionality for copying from std containers to buffers.
*/

namespace VmbCPP {

    namespace impl
    {
        template<class BufferElementType>
        inline void FinalizeCopy(BufferElementType* buffer, VmbUint32_t copiedSize) noexcept
        {
        }

        template<>
        inline void FinalizeCopy<char>(char* buffer, VmbUint32_t copiedSize) noexcept
        {
            // write terminating 0 char
            buffer[copiedSize] = '\0';
        }
    }

    template<class Container>
    inline
    typename std::enable_if<std::is_trivially_copyable<typename Container::value_type>::value, VmbErrorType>::type
    CopyToBuffer(Container const& container,
        typename Container::value_type* buffer,
        VmbUint32_t& bufferSize) noexcept
    {
        auto const containerSize = static_cast<VmbUint32_t>(container.size());
        if (buffer == nullptr)
        {
            bufferSize = containerSize;
        }
        else
        {
            if (bufferSize < containerSize)
            {
                return VmbErrorMoreData;
            }
            else
            {
                std::memcpy(buffer, container.data(), containerSize * sizeof(typename Container::value_type));
            }
            impl::FinalizeCopy<typename Container::value_type>(buffer, containerSize);
        }
        return VmbErrorSuccess;
    }

    template<class Map>
    inline VmbErrorType CopyMapValuesToBuffer(
        Map const& map,
        typename Map::mapped_type* buffer,
        VmbUint32_t& bufferSize) noexcept
    {
        auto const containerSize = static_cast<VmbUint32_t>(map.size());
        if (buffer == nullptr)
        {
            bufferSize = containerSize;
        }
        else
        {
            if (bufferSize < containerSize)
            {
                return VmbErrorMoreData;
            }
            else
            {
                auto bufferPos = buffer;
                for (typename Map::value_type const& element : map)
                {
                    *bufferPos = element.second;
                    ++bufferPos;
                }
            }
        }
        return VmbErrorSuccess;
    }

}  // namespace VmbCPP

#endif  // VMBCPP_COPY_UTILS_H
