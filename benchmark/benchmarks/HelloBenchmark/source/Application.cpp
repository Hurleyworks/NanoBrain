#include "Jahley.h"
#include <benchmark/benchmark.h>
//#include <nanothread/nanothread.h>

const std::string APP_NAME = "HelloBenchmark";

// from https://github.com/CoffeeBeforeArch/misc_code/blob/master/inc_bench/inc_bench.cpp


// Function to incrememnt atomic int
void inc_atomic (std::atomic_int64_t& val)
{
    for (int i = 0; i < 100000; i++)
        val++;
}

// Function to increment using a lock
void inc_mutex (std::mutex& m, long long int& val)
{
    for (int i = 0; i < 100000; i++)
    {
        std::lock_guard<std::mutex> g (m);
        val++;
    }
}

// Atomic bench
static void atomic_bench (benchmark::State& s)
{
    // Number of thread
    auto num_threads = s.range (0) - 1;

    // Create an atomic integer (for increments)
    std::atomic_int64_t val{0};

    // Reserve space for threads
    std::vector<std::thread> threads;
    threads.reserve (num_threads);

    // Timing loop
    for (auto _ : s)
    {
        // Spawn threads
        for (auto i = 0u; i < num_threads; i++)
            threads.emplace_back ([&]
                                  { inc_atomic (val); });
        inc_atomic (val);

        // Join threads
        for (auto& thread : threads)
            thread.join();

        // Clear to prevent joining twice
        threads.clear();
    }
}
BENCHMARK (atomic_bench)
    ->DenseRange (1, std::thread::hardware_concurrency())
    ->Unit (benchmark::kMillisecond)
    ->UseRealTime();

// Lock guard bench
static void lock_guard_bench (benchmark::State& s)
{
    // Number of thread
    auto num_threads = s.range (0) - 1;

    // Create an atomic integer (for increments)
    long long int val{0};

    // Reserve space for threads
    std::vector<std::thread> threads;
    threads.reserve (num_threads);

    // Create a mutex
    std::mutex m;

    // Timing loop
    for (auto _ : s)
    {
        // Spawn threads
        for (auto i = 0u; i < num_threads; i++)
            threads.emplace_back ([&]
                                  { inc_mutex (m, val); });
        inc_mutex (m, val);

        // Join threads
        for (auto& thread : threads)
            thread.join();

        // Clear to prevent joining twice
        threads.clear();
    }
}
BENCHMARK (lock_guard_bench)
    ->DenseRange (1, std::thread::hardware_concurrency())
    ->Unit (benchmark::kMillisecond)
    ->UseRealTime();

// Function for generating argument pairs
static void custom_args (benchmark::internal::Benchmark* b)
{
    for (int i = 1 << 4; i <= 1 << 10; i <<= 2)
    {
        // Collect stats at 1/8, 1/2, and 7/8
        for (int j : {32, 128, 224})
        {
            b = b->ArgPair (i, j);
        }
    }
}


// Baseline benchmark
static void baseModRandom (benchmark::State& s)
{
    // Number of elements in the vectors
    int N = s.range (0);

    // Max for mod operator
    int ceil = s.range (1);

    // Vector for input and output of modulo
    std::vector<int> input;
    std::vector<int> output;
    input.resize (N);
    output.resize (N);

    // Generate random inputs (uniform random dist. between 0 & 255)
    std::mt19937 rng;
    rng.seed (std::random_device()());
    std::uniform_int_distribution<int> dist (0, 255);
    for (int& i : input)
    {
        i = dist (rng);
    }

    // Main benchmark loop
    while (s.KeepRunning())
    {
        s.PauseTiming();
        for (int& i : input)
        {
            i = dist (rng);
        }
        s.ResumeTiming();

        // Compute the modulo for each element
        for (int i = 0; i < N; i++)
        {
            output[i] = input[i] % ceil;
        }
    }
}
// Register the benchmark
BENCHMARK (baseModRandom)->Apply (custom_args);

// Baseline benchmark
static void baseMod (benchmark::State& s)
{
    // Number of elements in the vectors
    int N = s.range (0);

    // Max for mod operator
    int ceil = s.range (1);

    // Vector for input and output of modulo
    std::vector<int> input;
    std::vector<int> output;
    input.resize (N);
    output.resize (N);

    // Generate random inputs (uniform random dist. between 0 & 255)
    std::mt19937 rng;
    rng.seed (std::random_device()());
    std::uniform_int_distribution<int> dist (0, 255);
    for (int& i : input)
    {
        i = dist (rng);
    }

    // Main benchmark loop
    while (s.KeepRunning())
    {
        // Compute the modulo for each element
        for (int i = 0; i < N; i++)
        {
            output[i] = input[i] % ceil;
        }
    }
}
// Register the benchmark
BENCHMARK (baseMod)->Apply (custom_args);

// from https://github.com/CoffeeBeforeArch/misc_code/tree/master/simple_bench

static void accumulate_bench (benchmark::State& s)
{
    // Number of elements (2^10)
    auto N = 1 << s.range (0);

    // Create a vector of random numbers
    std::vector<int> v (N);
    std::generate (begin (v), end (v), []
                   { return rand() % 100; });

    // Variable for our results
    int result = 0;

    // Main timing loop
    for (auto _ : s)
    {
        benchmark::DoNotOptimize (result = std::accumulate (begin (v), end (v), 0));
    }
}

BENCHMARK (accumulate_bench)->DenseRange (20, 22)->Unit (benchmark::kMicrosecond);


class Application : public Jahley::App
{
 public:
    Application() :
        Jahley::App()
    {
        int argc = 1;

        // https://stackoverflow.com/questions/2392308/c-vector-of-char-array
        // You cannot store arrays in vectors (or in any other standard library container).
        // The things that standard library containers store must be copyable and assignable,
        // and arrays are neither of these
        std::vector<char*> argv;
        char test[] = "HelloBenchmark";
        argv.push_back (test);

        benchmark::Initialize (&argc, argv.data());
        benchmark::TimeUnit::kMillisecond; //  how to set this???
        benchmark::RunSpecifiedBenchmarks();
    }

 private:
};

Jahley::App* Jahley::CreateApplication()
{
    return new Application();
}
