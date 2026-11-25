#ifndef GENUS_DESKTOPSTORE_H
#define GENUS_DESKTOPSTORE_H

#include <dirent.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include "BStoreBase.h"

class DesktopStore : public BStoreBase {
public:
  explicit DesktopStore(const char *aStoreName) : BStoreBase(aStoreName) {
    Initialize();
  }

public:
  char mTargetDir[4096];

public:
  void Initialize() OVERRIDE {
#ifdef _WIN32
    const char *homeDir = getenv("USERPROFILE");
#else
    const char *homeDir = getenv("HOME");
#endif
    snprintf(mTargetDir, sizeof(mTargetDir), "%s/.modus", homeDir ? homeDir : ".");

    DIR *dir = opendir(mTargetDir);
    if (dir) {
      closedir(dir);
    }
    else {
#ifdef _WIN32
      _mkdir(mTargetDir);
#else
      mkdir(mTargetDir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH);
#endif
    }

    mInitialized = ETrue;
  }

  TUint32 Size(const char *aKey) {
    char name[4096];
    sprintf(name, "%s/%s.%s.store", mTargetDir, mStoreName, aKey);
    struct stat stat_buf;
    if (stat(name, &stat_buf) != 0) {
      return 0;
    }
    return TUint32(stat_buf.st_size);
  }

  TBool Get(const char *aKey, void *aValue, TUint32 aSize) OVERRIDE {
    char name[4096];
    sprintf(name, "%s/%s.%s.store", mTargetDir, mStoreName, aKey);

    FILE *fp = fopen(name, "rb");
    if (!fp) {
      return EFalse;
    }
    if (fread(aValue, 1, aSize, fp) != aSize) {
      fclose(fp);
      return EFalse;
    }
    fclose(fp);
    return ETrue;
  }

  TBool Set(const char *aKey, void *aValue, TUint32 aSize) OVERRIDE {
    char name[4096];
    sprintf(name, "%s/%s.%s.store", mTargetDir, mStoreName, aKey);

    FILE *fp = fopen(name, "wb");
    if (!fp) {
      return EFalse;
    }
    if (fwrite(aValue, 1, aSize, fp) != aSize) {
      fclose(fp);
      return EFalse;
    }
    fclose(fp);
    return ETrue;
  }

  TBool Remove(const char *aKey) OVERRIDE {
    char name[4096];
    sprintf(name, "%s/%s.%s.store", mTargetDir, mStoreName, aKey);
    return unlink(name) == 0;
  }
};

#endif //GENUS_DESKTOPSTORE_H
