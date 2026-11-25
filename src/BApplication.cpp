#include "Display/Display.h"
#include "BApplication.h"
#include <time.h>

BApplication::BApplication() {
  SeedRandom(time(ENull));
  gDisplay.Init();

};

BApplication::~BApplication() {}

static void Exit(TInt aExitCode) {
  exit(aExitCode);
}
