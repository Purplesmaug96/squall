#pragma once

#include "storm/String.hpp"
#include "storm/Memory.hpp"
#include "storm/Error.hpp"
#include "storm/Thread.hpp"
#include "storm/List.hpp"
#include "storm/Hash.hpp"
#include "storm/Event.hpp"
#include "storm/File.hpp"
#include "storm/Handle.hpp"
#include "storm/Crypto.hpp"
#include "storm/Core.hpp"
#include "storm/Queue.hpp"
#include "storm/Transparency.hpp"
#include "storm/Unicode.hpp"
#include "storm/Array.hpp"
#include "storm/Atomic.hpp"
#include "storm/Registry.hpp"

#include <string.h>

#define SFileGetFileName(hFile, buffer, length) strncpy(buffer, "Unknown", length)
