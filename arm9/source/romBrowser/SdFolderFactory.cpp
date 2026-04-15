#include "common.h"
#include <vector>
#include "fat/Directory.h"
#include "FileInfo.h"
#include "FileType/Folder/FolderFileType.h"
#include "SdFolderFactory.h"
#include "string.h"
#if defined(DEBUG)
namespace
{
    std::unique_ptr<SdFolder> CreateSearchTestFolder(const IFileTypeProvider* fileTypeProvider)
    {
        static const char* sFakeNames[] =
        {
            "Pokemon HeartGold.nds",
            "Pokemon SoulSilver.nds",
            "Mario Kart DS.nds",
            "New Super Mario Bros.nds",
            "Castlevania Dawn of Sorrow.nds",
            "Chrono Trigger.nds",
            "Advance Wars Dual Strike.nds",
            "Kirby Super Star Ultra.nds",
            "Final Fantasy Tactics A2.nds",
            "Golden Sun Dark Dawn.nds",
            "Metroid Prime Hunters.nds",
            "test-homebrew.nds",
            "README.txt"
        };

        const int count = sizeof(sFakeNames) / sizeof(sFakeNames[0]);
        auto fileInfos = (FileInfo**)malloc(sizeof(FileInfo*) * count);
        for (int i = 0; i < count; i++)
        {
            const char* name = sFakeNames[i];
            const FileType* fileType = fileTypeProvider->GetFileType(name);
            fileInfos[i] = new FileInfo(name, fileType, FastFileRef(), 0);
        }

        return std::make_unique<SdFolder>(fileInfos, count);
    }
}
#endif

std::unique_ptr<SdFolder> SdFolderFactory::CreateFromPath(const char* path) const
{
#if defined(DEBUG)
    if (strcmp(path, ".") == 0)
    {
        return CreateSearchTestFolder(_fileTypeProvider);
    }
#endif

    Directory directory;
    if (directory.Open(path) != FR_OK)
        return nullptr;

    int count = 0;
    int bufferSize = 8;
    auto fileInfos = (FileInfo**)malloc(sizeof(FileInfo*) * bufferSize);
    auto sdFileInfo = std::make_unique<FILINFO>();
    while (true)
    {
        if (directory.Read(sdFileInfo.get()) != FR_OK)
            return nullptr;

        if (sdFileInfo->fname[0] == 0)
            break;

        if (count >= bufferSize)
        {
            bufferSize *= 2;
            fileInfos = (FileInfo**)realloc(fileInfos, sizeof(FileInfo*) * bufferSize);
        }
        auto fileType = sdFileInfo->fattrib & AM_DIR
            ? &FolderFileType::sInstance
            : _fileTypeProvider->GetFileType(sdFileInfo->fname);
        fileInfos[count++] = new FileInfo(sdFileInfo->fname, fileType,
            FastFileRef(directory.GetFatFsDirectory(), sdFileInfo.get()), sdFileInfo->fattrib);
    }

    return std::make_unique<SdFolder>(fileInfos, count);
}
