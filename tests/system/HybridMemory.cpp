/**************************************************************\

 ██╗  ██╗ █████╗ ██████╗ ████████╗ █████╗ ███╗   ██╗██╗ █████╗
 ╚██╗██╔╝██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗████╗  ██║██║██╔══██╗
  ╚███╔╝ ███████║██████╔╝   ██║   ███████║██╔██╗ ██║██║███████║
  ██╔██╗ ██╔══██║██╔══██╗   ██║   ██╔══██║██║╚██╗██║██║██╔══██║
 ██╔╝ ██╗██║  ██║██║  ██║   ██║   ██║  ██║██║ ╚████║██║██║  ██║
 ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝

Edition:
##  @date 19/10/2026 by @author Tsukini

File Name:
##  @file HybridMemory.cpp

File Description:
##  Unit tests of the HybridMemory (ram, custom gpu handler, real gpu backends when present)
\**************************************************************/

#include "utils.hpp"
#include <gtest/gtest.h>
#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>
#include <string>

// Run fn and expect the exception code
template<typename Fn>
static void expectCode(Fn fn, const utils::exception::InternalCode code)
{
    try {
        fn();
        FAIL() << "Expected an exception";
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), code);
    }
}

// Gpu handler backed by the host memory, records the calls
class FakeGpu: public utils::system::IGpu {
    public:
        bool available = true;
        bool failAllocation = false;
        bool failMemcpy = false;
        std::size_t allocations = 0;
        std::size_t frees = 0;
        std::size_t stores = 0;
        std::size_t memsets = 0;
        std::size_t memcpys = 0;
        std::size_t joins = 0;
        std::vector<std::vector<std::byte>*> live;

        bool isAvailable(void) const override {return this->available;};
        utils::system::MemoryMode getMode(void) const override {return utils::system::MemoryMode::Custom;};
        std::string getName(void) const override {return "FakeGpu";};
        std::size_t getMemory(void) const override {return 1 << 30;};

        utils::system::GpuBuffer allocate(const std::size_t bytes) override
        {
            if (this->failAllocation) throw utils::exception::ErrorException(utils::exception::InternalCode::GpuAllocation, "fake allocation failure");
            this->allocations++;
            this->live.push_back(new std::vector<std::byte>(bytes, std::byte{0xCD}));
            return reinterpret_cast<utils::system::GpuBuffer>(this->live.back());
        };

        void free(const utils::system::GpuBuffer buffer) noexcept override
        {
            if (!buffer) return;
            std::vector<std::byte>* memory = reinterpret_cast<std::vector<std::byte>*>(buffer);
            this->live.erase(std::find(this->live.begin(), this->live.end(), memory));
            delete memory;
            this->frees++;
        };

        void store(const utils::system::GpuBuffer dst, const void* src, const std::size_t bytes, const std::size_t offset = 0) override
        {
            std::vector<std::byte>& memory = *reinterpret_cast<std::vector<std::byte>*>(dst);
            if (offset + bytes > memory.size()) throw utils::exception::ErrorException(utils::exception::InternalCode::GpuTransfer, "store outside the buffer");
            std::copy_n(static_cast<const std::byte*>(src), bytes, memory.begin() + offset);
            this->stores++;
        };

        void fetch(void* dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t offset = 0) const override
        {
            const std::vector<std::byte>& memory = *reinterpret_cast<const std::vector<std::byte>*>(src);
            if (offset + bytes > memory.size()) throw utils::exception::ErrorException(utils::exception::InternalCode::GpuTransfer, "fetch outside the buffer");
            std::copy_n(memory.begin() + offset, bytes, static_cast<std::byte*>(dst));
        };

        void memset(const utils::system::GpuBuffer buffer, const std::byte value, const std::size_t bytes, const std::size_t offset = 0) override
        {
            std::vector<std::byte>& memory = *reinterpret_cast<std::vector<std::byte>*>(buffer);
            if (offset + bytes > memory.size()) throw utils::exception::ErrorException(utils::exception::InternalCode::GpuTransfer, "memset outside the buffer");
            std::fill_n(memory.begin() + offset, bytes, value);
            this->memsets++;
        };

        void memcpy(const utils::system::GpuBuffer dst, const utils::system::GpuBuffer src, const std::size_t bytes, const std::size_t dstOffset = 0, const std::size_t srcOffset = 0) override
        {
            if (this->failMemcpy) throw utils::exception::ErrorException(utils::exception::InternalCode::GpuTransfer, "fake copy failure");
            std::vector<std::byte>& to = *reinterpret_cast<std::vector<std::byte>*>(dst);
            const std::vector<std::byte>& from = *reinterpret_cast<const std::vector<std::byte>*>(src);
            if (dstOffset + bytes > to.size() || srcOffset + bytes > from.size()) throw utils::exception::ErrorException(utils::exception::InternalCode::GpuTransfer, "memcpy outside the buffer");
            std::copy_n(from.begin() + srcOffset, bytes, to.begin() + dstOffset);
            this->memcpys++;
        };

        void join(void) override {this->joins++;};

        ~FakeGpu() override {for (std::vector<std::byte>* memory: this->live) delete memory;};
};

struct Particle {
    float x;
    std::int32_t id;
};

/* ------------------------------- Ram ------------------------------- */
TEST(HybridMemory, RamDefaultState) {
    utils::system::HybridMemory<float> memory(utils::system::MemoryMode::Ram);

    EXPECT_TRUE(memory.empty());
    EXPECT_EQ(memory.size(), 0u);
    EXPECT_EQ(memory.bytes(), 0u);
    EXPECT_FALSE(memory.isOnGpu());
    EXPECT_EQ(memory.getMode(), utils::system::MemoryMode::Ram);
    EXPECT_EQ(memory.getRequestedMode(), utils::system::MemoryMode::Ram);
    EXPECT_EQ(memory.getGpu(), nullptr);
    EXPECT_EQ(memory.native(), 0u);
    EXPECT_EQ(memory.data(), nullptr);
}

TEST(HybridMemory, RamAllocate) {
    utils::system::HybridMemory<double> memory(16, utils::system::MemoryMode::Ram);

    EXPECT_FALSE(memory.empty());
    EXPECT_EQ(memory.size(), 16u);
    EXPECT_EQ(memory.bytes(), 16 * sizeof(double));
    EXPECT_NE(memory.data(), nullptr);
    EXPECT_EQ(memory.native(), 0u);
}

TEST(HybridMemory, RamStoreFetch) {
    utils::system::HybridMemory<int> memory(5, utils::system::MemoryMode::Ram);
    const std::vector<int> values = {1, 2, 3, 4, 5};
    std::vector<int> out(5);

    memory.store(values);
    memory.fetch(out);
    EXPECT_EQ(out, values);
    EXPECT_EQ(memory.toVector(), values);
}

TEST(HybridMemory, RamOffsets) {
    utils::system::HybridMemory<int> memory(6, utils::system::MemoryMode::Ram);
    const std::vector<int> values = {7, 8, 9};
    std::vector<int> out(2);

    memory.clear();
    memory.store(values, 2);
    memory.fetch(out, 3);
    EXPECT_EQ(memory.toVector(), (std::vector<int>{0, 0, 7, 8, 9, 0}));
    EXPECT_EQ(out, (std::vector<int>{8, 9}));
}

TEST(HybridMemory, RamGetSetOperator) {
    utils::system::HybridMemory<float> memory(3, utils::system::MemoryMode::Ram);

    memory.clear();
    memory.set(1, 2.5f);
    EXPECT_FLOAT_EQ(memory.get(1), 2.5f);
    EXPECT_FLOAT_EQ(memory[1], 2.5f);
    EXPECT_FLOAT_EQ(memory[0], 0.0f);
}

TEST(HybridMemory, RamFillClear) {
    utils::system::HybridMemory<float> memory(4, utils::system::MemoryMode::Ram);

    memory.fill(1.5f);
    EXPECT_EQ(memory.toVector(), (std::vector<float>(4, 1.5f)));
    memory.clear();
    EXPECT_EQ(memory.toVector(), (std::vector<float>(4, 0.0f)));
}

TEST(HybridMemory, RamResize) {
    utils::system::HybridMemory<int> memory(4, utils::system::MemoryMode::Ram);
    const std::vector<int> values = {1, 2, 3, 4};

    memory.store(values);
    memory.resize(6);
    EXPECT_EQ(memory.size(), 6u);
    const std::vector<int> grown = memory.toVector();
    EXPECT_EQ(std::vector<int>(grown.begin(), grown.begin() + 4), values);
    memory.resize(2);
    EXPECT_EQ(memory.toVector(), (std::vector<int>{1, 2}));
    memory.resize(0);
    EXPECT_TRUE(memory.empty());
    EXPECT_EQ(memory.data(), nullptr);
}

TEST(HybridMemory, RamFreeAndReallocate) {
    utils::system::HybridMemory<int> memory(4, utils::system::MemoryMode::Ram);

    memory.free();
    EXPECT_TRUE(memory.empty());
    EXPECT_EQ(memory.getMode(), utils::system::MemoryMode::Ram);
    memory.allocate(8);
    EXPECT_EQ(memory.size(), 8u);
}

TEST(HybridMemory, RamEmptyOperations) {
    utils::system::HybridMemory<int> memory(utils::system::MemoryMode::Ram);
    std::vector<int> none;

    memory.clear();
    memory.fill(3);
    memory.store(none);
    memory.fetch(none);
    memory.join();
    EXPECT_TRUE(memory.toVector().empty());
}

TEST(HybridMemory, RamOutOfBounds) {
    utils::system::HybridMemory<int> memory(4, utils::system::MemoryMode::Ram);
    std::vector<int> values(3);

    expectCode([&](void) {(void)memory.get(4);}, utils::exception::InternalCode::OutOfBounds);
    expectCode([&](void) {memory.set(4, 1);}, utils::exception::InternalCode::OutOfBounds);
    expectCode([&](void) {memory.store(values, 2);}, utils::exception::InternalCode::OutOfBounds);
    expectCode([&](void) {memory.fetch(values, 2);}, utils::exception::InternalCode::OutOfBounds);
    expectCode([&](void) {memory.store(values, static_cast<std::size_t>(-1));}, utils::exception::InternalCode::OutOfBounds);
}

TEST(HybridMemory, RamOverflow) {
    expectCode([](void) {utils::system::HybridMemory<double> memory(std::numeric_limits<std::size_t>::max() / 2, utils::system::MemoryMode::Ram);}, utils::exception::InternalCode::OutOfMemory);
}

TEST(HybridMemory, RamMove) {
    utils::system::HybridMemory<int> source(3, utils::system::MemoryMode::Ram);
    source.fill(9);

    utils::system::HybridMemory<int> moved(std::move(source));
    EXPECT_EQ(moved.toVector(), (std::vector<int>(3, 9)));
    EXPECT_TRUE(source.empty());

    utils::system::HybridMemory<int> assigned(2, utils::system::MemoryMode::Ram);
    assigned = std::move(moved);
    EXPECT_EQ(assigned.toVector(), (std::vector<int>(3, 9)));
    EXPECT_TRUE(moved.empty());
}

TEST(HybridMemory, RamStructure) {
    utils::system::HybridMemory<Particle> memory(2, utils::system::MemoryMode::Ram);

    memory.set(1, Particle{1.5f, 42});
    EXPECT_FLOAT_EQ(memory.get(1).x, 1.5f);
    EXPECT_EQ(memory.get(1).id, 42);
}

/* ------------------------------- Modes ------------------------------- */
TEST(HybridMemory, CustomWithoutHandler) {
    expectCode([](void) {utils::system::HybridMemory<int> memory(utils::system::MemoryMode::Custom);}, utils::exception::InternalCode::InvalidArgument);
}

TEST(HybridMemory, MakeGpuRam) {
    EXPECT_EQ(utils::system::make_raw_gpu(utils::system::MemoryMode::Ram), nullptr);
    EXPECT_EQ(utils::system::make_gpu(utils::system::MemoryMode::Ram), nullptr);
}

TEST(HybridMemory, AutoWorksWhateverTheMachine) {
    utils::system::HybridMemory<int> memory(4, utils::system::MemoryMode::Auto);
    const std::vector<int> values = {4, 3, 2, 1};

    EXPECT_EQ(memory.getRequestedMode(), utils::system::MemoryMode::Auto);
    EXPECT_EQ(memory.isOnGpu(), memory.getMode() != utils::system::MemoryMode::Ram);
    memory.store(values);
    memory.join();
    EXPECT_EQ(memory.toVector(), values);
}

/* ------------------------------- Custom handler ------------------------------- */
TEST(HybridMemory, CustomState) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    utils::system::HybridMemory<float> memory(8, gpu);

    EXPECT_TRUE(memory.isOnGpu());
    EXPECT_EQ(memory.getMode(), utils::system::MemoryMode::Custom);
    EXPECT_EQ(memory.getRequestedMode(), utils::system::MemoryMode::Custom);
    EXPECT_EQ(memory.getGpu(), gpu);
    EXPECT_NE(memory.native(), 0u);
    EXPECT_EQ(memory.data(), nullptr);
    EXPECT_EQ(memory.size(), 8u);
    EXPECT_EQ(gpu->allocations, 1u);
}

TEST(HybridMemory, CustomInvalidHandler) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();

    gpu->available = false;
    expectCode([&](void) {utils::system::HybridMemory<int> memory(4, std::shared_ptr<utils::system::IGpu>(gpu));}, utils::exception::InternalCode::InvalidArgument);
    expectCode([](void) {utils::system::HybridMemory<int> memory(4, std::shared_ptr<utils::system::IGpu>());}, utils::exception::InternalCode::InvalidArgument);
}

TEST(HybridMemory, CustomStoreFetch) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    utils::system::HybridMemory<int> memory(5, gpu);
    const std::vector<int> values = {1, 2, 3, 4, 5};
    std::vector<int> out(3);

    memory.store(values);
    EXPECT_EQ(memory.toVector(), values);
    memory.fetch(out, 1);
    EXPECT_EQ(out, (std::vector<int>{2, 3, 4}));
    EXPECT_EQ(gpu->stores, 1u);
}

TEST(HybridMemory, CustomGetSet) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    utils::system::HybridMemory<Particle> memory(3, gpu);

    memory.set(2, Particle{3.5f, 7});
    EXPECT_FLOAT_EQ(memory.get(2).x, 3.5f);
    EXPECT_EQ(memory[2].id, 7);
}

TEST(HybridMemory, CustomClearUsesMemset) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    utils::system::HybridMemory<int> memory(4, gpu);

    memory.clear();
    EXPECT_EQ(gpu->memsets, 1u);
    EXPECT_EQ(memory.toVector(), (std::vector<int>(4, 0)));
}

TEST(HybridMemory, CustomFillSameByteUsesMemset) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    utils::system::HybridMemory<std::uint32_t> memory(4, gpu);

    memory.fill(0xABABABABu);
    EXPECT_EQ(gpu->memsets, 1u);
    EXPECT_EQ(gpu->stores, 0u);
    EXPECT_EQ(memory.toVector(), (std::vector<std::uint32_t>(4, 0xABABABABu)));
}

TEST(HybridMemory, CustomFillUploadsChunks) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    utils::system::HybridMemory<float> memory(3, gpu);

    memory.fill(1.5f);
    EXPECT_EQ(gpu->memsets, 0u);
    EXPECT_GE(gpu->stores, 1u);
    EXPECT_EQ(memory.toVector(), (std::vector<float>(3, 1.5f)));
}

TEST(HybridMemory, CustomFillBiggerThanAChunk) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    const std::size_t size = (1 << 20) / sizeof(float) * 2 + 17;
    utils::system::HybridMemory<float> memory(size, gpu);

    memory.fill(0.25f);
    EXPECT_GT(gpu->stores, 1u);
    EXPECT_EQ(memory.toVector(), (std::vector<float>(size, 0.25f)));
}

TEST(HybridMemory, CustomResizeKeepsTheData) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    utils::system::HybridMemory<int> memory(4, gpu);

    memory.store(std::vector<int>{1, 2, 3, 4});
    memory.resize(6);
    EXPECT_EQ(memory.size(), 6u);
    EXPECT_EQ(gpu->memcpys, 1u);
    EXPECT_EQ(gpu->live.size(), 1u);
    memory.resize(2);
    EXPECT_EQ(memory.toVector(), (std::vector<int>{1, 2}));
    memory.resize(0);
    EXPECT_TRUE(memory.empty());
    EXPECT_EQ(memory.native(), 0u);
    EXPECT_TRUE(gpu->live.empty());
}

TEST(HybridMemory, CustomResizeFailureKeepsTheOldMemory) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    utils::system::HybridMemory<int> memory(3, gpu);

    memory.store(std::vector<int>{5, 6, 7});
    gpu->failMemcpy = true;
    expectCode([&](void) {memory.resize(9);}, utils::exception::InternalCode::GpuTransfer);
    EXPECT_EQ(memory.size(), 3u);
    EXPECT_EQ(gpu->live.size(), 1u);
    gpu->failMemcpy = false;
    EXPECT_EQ(memory.toVector(), (std::vector<int>{5, 6, 7}));
}

TEST(HybridMemory, CustomAllocationFailure) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();

    gpu->failAllocation = true;
    expectCode([&](void) {utils::system::HybridMemory<int> memory(4, gpu);}, utils::exception::InternalCode::GpuAllocation);
}

TEST(HybridMemory, CustomOutOfBounds) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    utils::system::HybridMemory<int> memory(2, gpu);
    std::vector<int> values(3);

    expectCode([&](void) {memory.store(values);}, utils::exception::InternalCode::OutOfBounds);
    expectCode([&](void) {(void)memory.get(2);}, utils::exception::InternalCode::OutOfBounds);
}

TEST(HybridMemory, CustomOverflow) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();

    expectCode([&](void) {utils::system::HybridMemory<double> memory(std::numeric_limits<std::size_t>::max() / 2, gpu);}, utils::exception::InternalCode::OutOfMemory);
    EXPECT_EQ(gpu->allocations, 0u);
}

TEST(HybridMemory, CustomJoin) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    utils::system::HybridMemory<int> memory(1, gpu);

    memory.join();
    EXPECT_EQ(gpu->joins, 1u);
}

TEST(HybridMemory, CustomFreeAndDestruction) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();

    {
        utils::system::HybridMemory<int> memory(4, gpu);
        memory.free();
        EXPECT_EQ(gpu->frees, 1u);
        EXPECT_TRUE(memory.empty());
        EXPECT_EQ(memory.getMode(), utils::system::MemoryMode::Custom);
        memory.allocate(2);
        EXPECT_EQ(gpu->live.size(), 1u);
    }
    EXPECT_TRUE(gpu->live.empty());
    EXPECT_EQ(gpu->allocations, gpu->frees);
}

TEST(HybridMemory, CustomMove) {
    std::shared_ptr<FakeGpu> gpu = std::make_shared<FakeGpu>();
    utils::system::HybridMemory<int> source(3, gpu);
    source.fill(0x01010101);

    utils::system::HybridMemory<int> moved(std::move(source));
    EXPECT_EQ(moved.toVector(), (std::vector<int>(3, 0x01010101)));
    EXPECT_TRUE(source.empty());
    EXPECT_EQ(source.native(), 0u);
    EXPECT_FALSE(source.isOnGpu());

    utils::system::HybridMemory<int> assigned(2, gpu);
    assigned = std::move(moved);
    EXPECT_EQ(gpu->live.size(), 1u);
    EXPECT_EQ(assigned.size(), 3u);
}

/* ------------------------------- Real backends ------------------------------- */
class HybridMemoryBackend: public ::testing::TestWithParam<utils::system::MemoryMode> {};

TEST_P(HybridMemoryBackend, RoundTrip) {
    std::unique_ptr<utils::system::HybridMemory<float>> memory;
    std::vector<float> values(1000);

    try {
        memory = std::make_unique<utils::system::HybridMemory<float>>(values.size(), GetParam());
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::GpuUnavailable);
        GTEST_SKIP() << "backend not available on this machine";
    }
    std::iota(values.begin(), values.end(), 0.0f);

    EXPECT_TRUE(memory->isOnGpu());
    EXPECT_EQ(memory->getMode(), GetParam());
    EXPECT_NE(memory->native(), 0u);
    memory->store(values);
    memory->join();
    EXPECT_EQ(memory->toVector(), values);
    EXPECT_FLOAT_EQ(memory->get(999), 999.0f);
    memory->set(3, -1.0f);
    EXPECT_FLOAT_EQ(memory->get(3), -1.0f);
}

TEST_P(HybridMemoryBackend, FillClearResize) {
    std::unique_ptr<utils::system::HybridMemory<std::uint32_t>> memory;

    try {
        memory = std::make_unique<utils::system::HybridMemory<std::uint32_t>>(10, GetParam());
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::GpuUnavailable);
        GTEST_SKIP() << "backend not available on this machine";
    }

    memory->fill(0xABABABABu);
    EXPECT_EQ(memory->toVector(), (std::vector<std::uint32_t>(10, 0xABABABABu)));
    memory->resize(20);
    EXPECT_EQ(memory->get(9), 0xABABABABu);
    memory->clear();
    EXPECT_EQ(memory->get(19), 0u);
    memory->fill(7u);
    EXPECT_EQ(memory->get(5), 7u);
}

TEST_P(HybridMemoryBackend, UnalignedBytes) {
    std::unique_ptr<utils::system::HybridMemory<std::uint8_t>> memory;

    try {
        memory = std::make_unique<utils::system::HybridMemory<std::uint8_t>>(7, GetParam());
    } catch (const utils::exception::IException& e) {
        EXPECT_EQ(e.getCode(), utils::exception::InternalCode::GpuUnavailable);
        GTEST_SKIP() << "backend not available on this machine";
    }

    memory->fill(0x5A);
    EXPECT_EQ(memory->toVector(), (std::vector<std::uint8_t>(7, 0x5A)));
    memory->store(std::vector<std::uint8_t>{1, 2, 3}, 3);
    EXPECT_EQ(memory->toVector(), (std::vector<std::uint8_t>{0x5A, 0x5A, 0x5A, 1, 2, 3, 0x5A}));
}

static std::string backendName(const ::testing::TestParamInfo<utils::system::MemoryMode>& info)
{
    switch (info.param) {
        case utils::system::MemoryMode::Cuda: return "Cuda";
        case utils::system::MemoryMode::OpenCL: return "OpenCL";
        default: return "Vulkan";
    }
}

INSTANTIATE_TEST_SUITE_P(Backends, HybridMemoryBackend, ::testing::Values(utils::system::MemoryMode::Cuda, utils::system::MemoryMode::OpenCL, utils::system::MemoryMode::Vulkan), backendName);
