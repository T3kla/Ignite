#include "TklSessions.h"

#define LOCTEXT_NAMESPACE "FTklSessionsModule"

DEFINE_LOG_CATEGORY(LogTklSessions);

void FTklSessionsModule::StartupModule() {}

void FTklSessionsModule::ShutdownModule() {}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FTklSessionsModule, TklSessions)
