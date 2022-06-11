#include "parser.h"
#include "scheduler.h"

// these constants could be parsed from config
namespace {
    static constexpr size_t Threads = 8;
    static constexpr size_t Md5HashSize = 32;
} // anonymous namespace

int main(int argc, const char** argv) {
    uint32_t blockSizeInBytes = 1024 * 1024;

    if (argc < 3 || argc > 4) {
        throw std::runtime_error("wrong number of arguments");
    }
    const std::string fileInPath = argv[1];
    const std::string fileOutPath = argv[2];
    if (argc == 4) {
        blockSizeInBytes = atoi(argv[3]);
    }
    size_t pageSize = sysconf(_SC_PAGESIZE);
    NSignatureHashing::TFileActions fileReader(pageSize);

    assert(pageSize % (Threads * Md5HashSize) == 0);
    try {
        fileReader.OpenFileDescriptors(fileInPath, fileOutPath);
    } catch (std::exception& error) {
        std::cerr << error.what() << std::endl;
        throw;
    }

    size_t outputOffset = 0;
    while (!fileReader.IsLastBlock()) {
        char* mmapingInput;
        char* mmapingOutput;
        try {
            mmapingInput = fileReader.GetNextMMapFileIn(blockSizeInBytes);
            if (outputOffset == 0) {
                mmapingOutput = fileReader.GetNextMMapFileOut(Md5HashSize * Threads);
            }
        } catch (std::exception& error) {
            std::cerr << error.what() << std::endl;
            throw;
        }

        NSignatureHashing::TScheduleHashing hashingScheduler(
            Threads,
            mmapingInput,
            blockSizeInBytes,
            mmapingOutput + outputOffset,
            Md5HashSize
        );

        hashingScheduler.Schedule();
        outputOffset = (outputOffset + Md5HashSize * Threads) % pageSize;
        try {
            fileReader.UndoMMaping(mmapingInput, blockSizeInBytes);
            if (outputOffset == 0 || fileReader.IsLastBlock()) {
                fileReader.UndoMMaping(mmapingOutput, Md5HashSize * Threads);
                fileReader.WritingIterationFinished();
            }
        } catch (std::exception& error) {
            std::cerr << error.what() << std::endl;
        }

        fileReader.ReadingIterationFinished();
    }


    fileReader.CloseFiles();

    return 0;
}