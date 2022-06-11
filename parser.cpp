#include "parser.h"

#include <stdexcept>


namespace NSignatureHashing {

    TFileActions::TFileActions(size_t pageSize) 
        : PageSize_(pageSize)
    {
    }

    void TFileActions::OpenFileDescriptors(const std::string& fileInPath, const std::string& fileOutPath) {
        try {
            OpenFileHelper(fileInPath, &FileInStat_);
        } catch (const std::exception&) {
            throw std::runtime_error("can't open input file");
        }
        FileInDesc_ = open(&fileInPath[0], O_RDWR, static_cast<mode_t>(0600));
        if (FileInDesc_ == -1) {
            throw std::runtime_error("can't open input file");
        }
        try {
            OpenFileHelper(fileOutPath, &FileOutStat_);
        } catch (const std::exception&) {
            throw std::runtime_error("can't open input file");
        }
        FileOutDesc_ = open(&fileOutPath[0], O_RDWR, static_cast<mode_t>(0600));
        if (FileOutDesc_ == -1) {
            close(FileInDesc_);
            throw std::runtime_error("cant open output file");
        }
    }

    void TFileActions::OpenFileHelper(const std::string& filePath, struct stat* fileStat) {
        if (lstat(&filePath[0], fileStat) == -1) {
            throw std::exception();
        }
        if (!S_ISREG(fileStat->st_mode)) {
            throw std::exception();
        }
    }


    char* TFileActions::GetNextMMapFileIn(const uint32_t& bytesPerIteration) {
        uint64_t comfortableSize = bytesPerIteration * (GetReadingIteration() + 1);
        uint64_t previousSize = FileInStat_.st_size;
        if (comfortableSize >= previousSize) {
            LastBlock = true;
        }
        if (comfortableSize > FileInStat_.st_size) {
            int error = ftruncate(FileInDesc_, comfortableSize);
            if (error == -1) {
                close(FileInDesc_);
                close(FileOutDesc_);
                throw std::runtime_error("truncating failed");
            }
        }
        char *mmapping = static_cast<char*>(mmap(NULL, bytesPerIteration, PROT_WRITE | PROT_READ, MAP_SHARED, FileInDesc_, bytesPerIteration * GetReadingIteration()));
        if (mmapping == MAP_FAILED) {
            close(FileInDesc_);
            close(FileOutDesc_);
            throw std::runtime_error("mmapping failed");
        }
        return mmapping;
    }

    char* TFileActions::GetNextMMapFileOut(const uint32_t& bytesPerIteration) {
        int error = ftruncate(FileOutDesc_, PageSize_ * (GetWritingIteration() + 1));
        if (error == -1) {
            close(FileInDesc_);
            close(FileOutDesc_);
            throw std::runtime_error("truncating failed");
        }
        char *mmapping = static_cast<char*>(mmap(NULL, PageSize_, PROT_WRITE | PROT_READ, MAP_SHARED, FileOutDesc_, PageSize_ * GetWritingIteration()));
        if (mmapping == MAP_FAILED) {
            close(FileInDesc_);
            close(FileOutDesc_);
            throw std::runtime_error("mmapping failed");
        }
        return mmapping;
    }

    void TFileActions::ReadingIterationFinished() {
        ++Iteration_;
    }

    void TFileActions::WritingIterationFinished() {
        ++WriterIteration_;
    }

    size_t TFileActions::GetReadingIteration() {
        return Iteration_;
    }

    size_t TFileActions::GetWritingIteration() {
        return WriterIteration_;
    }

    bool TFileActions::IsLastBlock() const {
        return LastBlock;
    }

    void TFileActions::UndoMMaping(char* mmaping, uint64_t mmapingSize) {
        msync(mmaping, mmapingSize, MS_SYNC);
        if (munmap(mmaping, mmapingSize) == -1) {
            throw std::runtime_error("munmap failed");
        }
    }

    void TFileActions::CloseFiles() {
        if (close(FileInDesc_) == -1) {
            throw std::runtime_error("closing input file failed");
        }
        if (close(FileOutDesc_) == -1) {
            throw std::runtime_error("closing output file failed");
        }
    }

} // namespace NSignatureCoding
