/*
MIT License

Copyright (c) 2023 Steve Hurley

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
