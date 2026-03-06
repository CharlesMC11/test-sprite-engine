import Cocoa
import MetalKit

@_cdecl("render")
public func start_metal_app(sprite: UnsafeMutablePointer<Sprite>) {
    let app = NSApplication.shared

    let window = NSWindow(
        contentRect: NSRect(x: 0, y: 0, width: 480, height: 320),
        styleMask: [.titled, .closable, .miniaturizable],
        backing: .buffered, defer: false)

    let view = MTKView(frame: window.contentView!.bounds)
    view.device = MTLCreateSystemDefaultDevice()
    // FIX: Disable framebuffer-only to allow compute write access
    view.framebufferOnly = false
    view.layer?.isOpaque = false
    // Optional: Set a specific clear color to verify it's working (e.g., black)
    view.clearColor = MTLClearColor(red: 0, green: 0, blue: 0, alpha: 1)

    let renderer = SpriteRenderer(metalView: view, pointer: sprite)
    view.delegate = renderer

    window.contentView?.addSubview(view)
    window.makeKeyAndOrderFront(nil)

    app.run()
}
