#pragma once

#define MT_ENUM_VARIANT(NAME, VALUE) NAME = (VALUE),
#define MT_ENUM(VARIANT)                                                      \
  VARIANT (MT_BROADCAST, 1 << 0)                                              \
  VARIANT (MT_EVENT, 1 << 1)                                                  \
  VARIANT (MT_WARNING, 1 << 2)                                                \
  VARIANT (MT_UI, 1 << 3)                                                     \
  VARIANT (MT_STORAGE, 1 << 4)                                                \
  VARIANT (MT_CELLULAR, 1 << 5)                                               \
  VARIANT (MT_POSTMAN, 1 << 6)                                                \
  VARIANT (MT_ASSIST, 1 << 7)                                                 \
  VARIANT (MT_SAFETY, 1 << 8)                                                 \
  VARIANT (MT_MAX, 1 << 9)                                                    \
  VARIANT (MT_IGNORE, 0)

#define PLUS_ONE(...) +1
#define MT_COUNT (-2 MT_ENUM (PLUS_ONE)) /* -1 for MT_IGNORE -1 for MT_MAX */

#define MT_SIZE MT_COUNT

#if defined(__cplusplus)
extern "C"
{
#endif

  typedef enum
  {
    MT_ENUM (MT_ENUM_VARIANT)
  } MT;

#define MT_ENUM_STRING(NAME, VALUE)                                           \
  case NAME:                                                                  \
    return #NAME;

  const char *mt_name (MT mt);

#if defined(__cplusplus)
}
#endif
