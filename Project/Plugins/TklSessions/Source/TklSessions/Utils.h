#pragma once

#if WITH_EDITOR

#define LogScreen(Color, Duration, Order, Format, ...)                         \
  {                                                                            \
    if (GEngine)                                                               \
      GEngine->AddOnScreenDebugMessage(                                        \
          Order, Duration, FColor::##Color,                                    \
          FString::Printf(TEXT(Format), ##__VA_ARGS__));                       \
  }

#else

#define LogScreen(Color, Duration, Order, Format, ...)

#endif
