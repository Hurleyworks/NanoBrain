#pragma once

#include <unordered_map>
#include <unordered_set>
#include <array>
#include <queue>
#include <stack>
#include <fstream>
#include <set>
#include <vector>
#include <sstream>
#include <random>
#include <chrono>
#include <thread>
#include <ctime>
#include <string>
#include <iostream>
#include <stdexcept>
#include <assert.h>
#include <limits>
#include <algorithm>
#include <functional>
#include <stdint.h>
#include <any>
#include <filesystem>
#include <mutex>
#include <memory>
#include <condition_variable>
#include <variant>
#include <future>
#include <semaphore>
#include <concepts>

using ItemID = int64_t;

#ifdef __clang__
#include <experimental/coroutine>
#define COROUTINE_NAMESPACE std::experimental
#else
#include <coroutine>
#define COROUTINE_NAMESPACE std
#endif


// openimageio
#include <OpenImageIO/thread.h>
#include <OpenImageIO/unordered_map_concurrent.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagecache.h>


// g3log
#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>


// json
#include <json/json.hpp>
using nlohmann::json;

// some useful tools and defines outside mace namespace
#include "excludeFromBuild/basics/Defaults.h"
#include "excludeFromBuild/basics/Util.h"

#include "excludeFromBuild/thread/BS_thread_pool.h"
#include "excludeFromBuild/thread/BS_thread_pool_light.h"

namespace mace
{
	#include "excludeFromBuild/basics/StringUtil.h"

} // namespace mace
