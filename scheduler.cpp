#include "scheduler.h"

#include <string>
#include <functional>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

namespace {
    using boost::uuids::detail::md5;

    std::string HashToString(const md5::digest_type &digest) {
        const auto charDigest = reinterpret_cast<const char *>(&digest);
        std::string result;
        boost::algorithm::hex(charDigest, charDigest + sizeof(md5::digest_type), std::back_inserter(result));
        return result;
    }

    std::string GetHash(const std::string_view& s) {
        md5 hash;
        md5::digest_type digest;
        hash.process_bytes(s.data(), s.size());
        hash.get_digest(digest);
        return HashToString(digest);
    }
} // anonymous namespace


namespace NSignatureHashing {
    TScheduleHashing::TScheduleHashing(int threads, char* mapping, uint64_t mappingSize, char* writerMapping, int md5HashSize) 
        : Threads_(threads)
        , Mapping_(mapping)
        , MappingSize_(mappingSize)
        , WriterMapping_(writerMapping)
        , BytesPerThread_(mappingSize / threads)
        , Md5HashSize_(md5HashSize)
    {
    }

    void TScheduleHashing::Schedule() {
        std::vector<std::thread> hashingTasks;
        for (size_t i = 0; i < Threads_; ++i) {
            for (size_t i = 0; i < Threads_; ++i) {
                char* partStart = Mapping_ + i * BytesPerThread_;
                size_t size = BytesPerThread_;
                char* writeStart = WriterMapping_ + i * Md5HashSize_;
                size_t writeSize = Md5HashSize_;
                if (i * BytesPerThread_ > MappingSize_) {
                    continue;
                }
                hashingTasks.emplace_back(
                    [partStart, size, writeStart, writeSize] {
                        std::string currentHash = GetHash(std::string_view(partStart, size));
                        for (int i = 0; i < currentHash.size(); ++i) {
                            *(writeStart + i) = currentHash[i];
                        }
                    }
                );
            }
        }
        for (size_t i = 0; i < hashingTasks.size(); ++i) {
            hashingTasks[i].join();
        }
    }
} // namespace NSignatureCoding
