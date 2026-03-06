import MetalKit

class SpriteRenderer: NSObject, MTKViewDelegate {
    var commandQueue: MTLCommandQueue!
    var pipelineState: MTLComputePipelineState!
    var sprite: UnsafeMutablePointer<Sprite>

    init(metalView: MTKView, pointer: UnsafeMutablePointer<Sprite>) {
        self.sprite = pointer
        let device = metalView.device!
        self.commandQueue = device.makeCommandQueue()

        let libraryPath = URL(fileURLWithPath: "build/shader.metallib")

    do {
        let library = try device.makeLibrary(URL: libraryPath)
        guard let kernel = library.makeFunction(name: "renderSprite") else {
            fatalError("Could not find function 'renderSprite' in library")
        }
        self.pipelineState = try device.makeComputePipelineState(function: kernel)
    } catch {
        fatalError("Failed to load Metal library: \(error)")
    }

    super.init()
    }

    func draw(in view: MTKView) {
        guard let drawable = view.currentDrawable,
              let commandBuffer = commandQueue.makeCommandBuffer(),
              let encoder = commandBuffer.makeComputeCommandEncoder() else { return }

        encoder.setComputePipelineState(pipelineState)

        encoder.setBytes(sprite, length: MemoryLayout<Sprite>.size, index: 0)
        encoder.setTexture(drawable.texture, index: 0)

        let threads = MTLSize(width: Int(WIDTH), height: Int(HEIGHT), depth: 1)
        encoder.dispatchThreads(threads, threadsPerThreadgroup: threads)

        encoder.endEncoding()
        commandBuffer.present(drawable)
        commandBuffer.commit()
    }

    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {}
}
