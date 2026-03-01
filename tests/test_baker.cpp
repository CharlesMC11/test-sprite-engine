#include "sprite.h"

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <cstdio>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

static constexpr char SHADER_LIB[]{"build/shader.metallib"};

static constexpr std::size_t SPRITE_SIZE_BYTES{sizeof(sc::Sprite)};

void debug_sprite(sc::Sprite* sprite);

int main(const int argc, const char* argv[])
{
    if (argc < 2)
        return 1;

    const auto fd{open(argv[1], O_RDONLY)};
    if (fd < 0)
        return 1;

    const auto sprite{static_cast<sc::Sprite*>(
            mmap(nullptr, SPRITE_SIZE_BYTES, PROT_READ, MAP_SHARED, fd, 0))};
    if (sprite == MAP_FAILED)
        return 1;

    /* METAL STUFF */
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    MTL::CommandQueue* queue = device->newCommandQueue();
    NS::Error* error = nullptr;

    auto libraryPath = NS::String::string(SHADER_LIB, NS::UTF8StringEncoding);
    MTL::Library* library = device->newLibrary(libraryPath, &error);

    if (!library) {
        std::printf("Error: Could not load shader library: %s\n",
                error->localizedDescription()->utf8String());
        return 1;
    }

    // Extract the kernel function by the name used in your shader.metal
    auto functionName =
            NS::String::string("sprite_render", NS::UTF8StringEncoding);
    MTL::Function* kernelFunction = library->newFunction(functionName);

    // Create the PSO: This "bakes" the shader function for your specific
    // hardware
    MTL::ComputePipelineState* pso =
            device->newComputePipelineState(kernelFunction, &error);

    if (!pso) {
        std::printf("Error: Failed to create PSO: %s\n",
                error->localizedDescription()->utf8String());
        return 1;
    }

    auto desc = MTL::TextureDescriptor::texture2DDescriptor(
            MTL::PixelFormatRGBA32Float, WIDTH, HEIGHT, false);
    desc->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
    desc->setStorageMode(MTL::StorageModeShared);
    MTL::Texture* outTexture = device->newTexture(desc);

    // Wrap your mmap'd sprite data into a GPU-visible buffer
    // The size must be exactly 16,448 bytes to avoid out-of-bounds reads
    MTL::Buffer* spriteBuf = device->newBuffer(
            sprite, SPRITE_SIZE_BYTES, MTL::ResourceStorageModeShared);

    // 5. Execute Compute Kernel
    MTL::CommandBuffer* cmdBuf = queue->commandBuffer();
    MTL::ComputeCommandEncoder* encoder = cmdBuf->computeCommandEncoder();

    encoder->setComputePipelineState(pso);
    // Bind the Shape Node (Sprite Struct) to [[buffer(0)]]
    encoder->setBuffer(spriteBuf, 0, 0);
    // Bind the Output Texture to [[texture(0)]]
    encoder->setTexture(outTexture, 0);

    // Define the Grid for 128x128
    MTL::Size gridSize = MTL::Size(WIDTH, HEIGHT, 1);
    // 8x8 is the "sweet spot" for threadgroup occupancy on Apple Silicon GPUs
    MTL::Size threadgroupSize = MTL::Size(8, 8, 1);

    // Dispatch the threads
    encoder->dispatchThreads(gridSize, threadgroupSize);

    encoder->endEncoding();
    cmdBuf->commit();
    cmdBuf->waitUntilCompleted();

    // 1. Prepare a buffer to hold the 128x128 RGBA float data
    // 128 * 128 pixels * 4 channels (R, G, B, A)
    std::vector<float> textureData(WIDTH * HEIGHT * 4);

    // 2. Define the region to read (the entire 128x128 texture)
    MTL::Region region = MTL::Region::Make2D(0, 0, WIDTH, HEIGHT);

    // 3. Synchronize and copy data from GPU texture to CPU vector
    // bytesPerRow = WIDTH * 4 channels * sizeof(float)
    outTexture->getBytes(
            textureData.data(), WIDTH * 4 * sizeof(float), region, 0);

    // 4. Generate the PPM File
    FILE* f = std::fopen("data/preview.ppm", "w");
    if (!f) {
        std::printf("Error: Could not create PPM file.\n");
        return 1;
    }

    // PPM Header: P3 (ASCII), Width, Height, Max Color Value (255)
    std::fprintf(f, "P3\n%d %d\n255\n", WIDTH, HEIGHT);

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            // Calculate the index in our flat float array
            int pixelIndex = (y * WIDTH + x) * 4;

            // Convert normalized float (0.0 - 1.0) to integer (0 - 255)
            // We use static_cast for safety in C++
            int r = static_cast<int>(textureData[pixelIndex + 0] * 255.0f);
            int g = static_cast<int>(textureData[pixelIndex + 1] * 255.0f);
            int b = static_cast<int>(textureData[pixelIndex + 2] * 255.0f);

            // Write the RGB triplet to the file
            std::fprintf(f, "%d %d %d ", r, g, b);
        }
        // New line at the end of every row for human-readability
        std::fprintf(f, "\n");
    }

    std::fclose(f);
    std::printf("Successfully generated output_preview.ppm\n");

    debug_sprite(sprite);

    return 0;
}

void debug_sprite(sc::Sprite* sprite)
{
    std::printf("Magic: %.8s | Encoding: %d\n", sprite->magic, sprite->encoding); //
    for (std::size_t y = 0; y < HEIGHT; ++y) {
        for (std::size_t x = 0; x < WIDTH; ++x) {
            const uint_fast8_t i{sprite->pixels[y * HEIGHT + x].index};

            std::printf("%c", i > 0 ? '#' : ' ');
        }
        std::puts("");
    }
}
