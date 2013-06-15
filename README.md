# GLFW 3.0 (hx-glfw3)

Fork of glfw/glfw for stable development of hx-glfw3 lib.

### Changes

All callback methods are modified to include a *user data* ```void*``` pointer argument that is passed
to the provided function as its last argument.

This is to support an easier interface in the haxe FFI for haxe callbacks.
