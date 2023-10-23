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

// eigen math
#include <linalg/eigen34/Eigen/Dense>

using Eigen::Matrix;
using Pose = Eigen::Affine3f;
using Scale = Eigen::Vector3f;

using Float = float;
using MatrixXc = Eigen::Matrix<uint8_t, Eigen::Dynamic, Eigen::Dynamic>;
using MatrixXf = Eigen::Matrix<Float, Eigen::Dynamic, Eigen::Dynamic>;
using MatrixXu = Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic>;
using Vector3u = Eigen::Matrix<uint32_t, 3, 1>;
using Vector4u = Eigen::Matrix<uint32_t, 4, 1>;
using Vector2u = Eigen::Matrix<uint32_t, 2, 1>;
// using Matrix3f = Eigen::Matrix<Float, 3, 3>;
using Matrix43f = Eigen::Matrix<Float, 4, 3>;
using MatrixRowMajor34f = Eigen::Matrix<Float, 3, 4, Eigen::RowMajor>;
using Matrix4f = Eigen::Matrix<Float, 4, 4>;
using MatrixXu16 = Eigen::Matrix<uint16_t, Eigen::Dynamic, Eigen::Dynamic>;
using VectorXu = Eigen::Matrix<uint32_t, Eigen::Dynamic, 1>;
using VectorXb = Eigen::Matrix<bool, Eigen::Dynamic, 1>;
using MatrixXi = Eigen::Matrix<int32_t, Eigen::Dynamic, Eigen::Dynamic>;

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

// nano signal and slots
#include <nano_signal/nano_signal_slot.hpp>
#include <nano_signal/nano_mutex.hpp>
using Observer = Nano::Observer<>;

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
	#include "excludeFromBuild/basics/InputEvent.h"

} // namespace mace
