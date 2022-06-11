#pragma once
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h>


namespace NSignatureHashing {
    class TFileActions {
    public:
        TFileActions(size_t pageSize);

        void OpenFileDescriptors(const std::string& fileInPath, const std::string& fileOutPath);

        char* GetNextMMapFileIn(const uint32_t& bytesPerIteration);

        char* GetNextMMapFileOut(const uint32_t& bytersPerIteration);

        bool IsLastBlock() const;

        void ReadingIterationFinished();

        void WritingIterationFinished();

        size_t GetReadingIteration();

        size_t GetWritingIteration();

        void UndoMMaping(char* mmaping, uint64_t mmapingSize);

        void CloseFiles();
    
    private:
        void OpenFileHelper(const std::string& filePath, struct stat* fileStat);

    private:
        bool LastBlock = false;
        uint32_t Iteration_ = 0, WriterIteration_ = 0;
        int FileInDesc_, FileOutDesc_;
        struct stat FileInStat_, FileOutStat_;
        uint64_t MaxIterationSize_ = 4ull * 1024ull * 1024ull * 1024ull;
        size_t PageSize_;
    };
} // namespace NSignatureCoding
