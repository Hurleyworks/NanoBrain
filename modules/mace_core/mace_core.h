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
#include <numbers>
#include <variant>

#ifdef __clang__
#include <experimental/coroutine>
#define COROUTINE_NAMESPACE std::experimental
#else
#include <coroutine>
#define COROUTINE_NAMESPACE std
#endif

// eigen math
#include <linalg/eigen34/Eigen/Dense>

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

// moody camel lock free queue
#include <concurrent/concurrentqueue.h>

// nano signal and slots
#include <nano_signal/nano_signal_slot.hpp>
#include <nano_signal/nano_mutex.hpp>
using Observer = Nano::Observer<>;

//BS_thread_pool
#include <BSthread/BS_thread_pool.h>
#include <BSthread/BS_thread_pool_light.h>

// json
#include <json/json.hpp>
using nlohmann::json;

// some useful tools and defines outside mace namespace
#include "excludeFromBuild/basics/Defaults.h"
#include "excludeFromBuild/basics/Util.h"

namespace mace
{
// basics
#include "excludeFromBuild/basics/StringUtil.h"
#include "excludeFromBuild/basics/InputEvent.h"

// imaging
#include "excludeFromBuild/imaging/CacheHandler.h"

} // namespace mace
