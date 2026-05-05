# Custom AArch64-Metal Sprite Engine

A low-level 2D sprite simulation engine built from scratch to explore modern
Single Instruction, Multiple Data (SIMD) architectures, unified memory, and
high-performance graphics pipelines.

Inspired by the hardware architecture of the Game Boy Advance (GBA) and the
execution constraints of legacy Game Pak buses, this project bridges classic
sprite management with techniques tailored for **Apple Silicon (M2 Max /
AArch64)**.

---

## Core Architecture

### 1. Unified Structure of Arrays (SoA) Pool Allocator

To maximize cache-line efficiency, entities are managed inside a unified
`channel_pool`. Rather than allocating individual entity objects, fields are
partitioned into contiguous, linear memory channels (`x_pos`, `y_vel`,
`z_new_pos`, etc.).

- **Zero-Copy GPU Sharing**: Backed by `MTL::ResourceStorageModeShared`, the
  CPU-side physics subsystem writes directly to unified memory space, making
  transforms immediately visible to the GPU.
- **Strict Cache Alignment**: Memory reallocations automatically round up each
  channel’s internal capacity to 128-byte alignment, ensuring its NEON vector
  field fit into a cache lane boundaries.

### 2. Vectorized Update Pipeline (ARM NEON)

- The engine loads transformations via 128-bit vector lanes (`vld1q_f32`) and
  updates states using hardware fused multiply-accumulate (`vfmaq_f32`)
  operations.

---

## Technical Stack & Hardware Focus

- **Language Standard**: C++23 working features (leveraging `<ranges>`,
  concepts, and compile-time evaluation constraints).
- **Compiler**: Native Toolchain (`Apple Clang`) configured with
  high-optimization overrides (`-O3 -flto`).
- **Graphics API**: Pure C++ Metal bindings via `metal-cpp` (zero Objective-C
  overhead during hot paths).
- **Assembly / Intrinsic Targets**: AArch64 Vector Architecture
  (`<arm_neon.h>`).

---

## Key Implementation Highlights

- **Bit-Packed System Blobs**: Custom metadata masking limits allocation
  footprints, mirroring vintage hardware registries like OAM.
- **Fixed-Timestep Simulation**: Disconnects rendering update cadences from
  physical tick integrations to ensure deterministic collision stability, backed
  by strict clamp protections to prevent framerate "cascades."
- **Spatial Hashing**: Accelerates proximity queries by mapping dense grid
  arrays to linear memory lookups, narrowing the collision search space.
