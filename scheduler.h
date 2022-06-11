#pragma once

#include <iostream>
#include <boost/uuid/detail/md5.hpp>
#include <boost/algorithm/hex.hpp>

using boost::uuids::detail::md5;


namespace NSignatureHashing {
    
    class TScheduleHashing {
    public:
        TScheduleHashing(int threads, char* mapping, uint64_t mappingSize, char* writerMapping, int md5HashSize);

        void Schedule();

    private:
        int Threads_;
        char* Mapping_;
        size_t MappingSize_;
        char* WriterMapping_;
        int Md5HashSize_;
        uint64_t BytesPerThread_;
    };
} // namespace NSignatureCoding
