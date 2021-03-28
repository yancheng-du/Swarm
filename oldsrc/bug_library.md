# Bug Library
## Instructions
Use an unordered list with entries in the following format
to add any bugs you encountered, and how you fixed them.
Please be descriptive and add a line break after each entry.

* Description: Description of Bug
* Solution: how you fixed it, or if it's still open label <OPEN>

================================================================================
Description: pictures do not show up, only a black background is visible when
rendering with OpenGL
Solution:    The black background is usually due to the texture not being
properly read in. In this case, the graphics_module.cpp - GraphicsModule
constructor calls loadPNG and passes a file path. This file path
must be relative to whatever is calling the GraphicsModule contructor, not to
graphics_module.cpp. This goes for many of the file paths in the graphics code.
