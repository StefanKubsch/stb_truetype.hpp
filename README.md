# stb_truetype.hpp

stb_truetype.hpp - under development - public domain

authored/modified in 2019 by Stefan Kubsch

a stripped down version of stb_truetype.h

WHY AND WHAT?
-------------

Since I only use the "old" 3D API in my project, I removed all unrelated stuff and tried to "upgrade" the original code to C++.

This lib is perfect if you just need following functions to generate a simple atlas-bitmap for further use with e.g. OpenGL:

  - stbtt_InitFont()
  - stbtt_GetCodepointBitmapBox()
  - stbtt_BakeFontBitmap()
  - stbtt_GetBakedQuad()

CONVERSION TO C++
-----------------

WHAT I DID SO FAR:
------------------

  - fixed all warnings reported by PVS-Studio and clang-tidy (use brackets etc.)
  - replaced all "typedef" with "using"
  - replaced "malloc/free" with "new/delete"
  - replaced all C-style casts with modern-style C++ cast (e.g. static_cast)
  - initialised all variables and narrowed their scopes if possible
  - fixed all implicit float-to-double-conversions by consequent use of the float literal (e.g. 0.0F)
  - made everything "const", what can and should be const
  - replaced C headers with the corresponding C++ headers
  - used STL functions like "std::max" or "std::swap" where possible, replacing the old implementations
  - replaced C-style arrays with std::vector (but, of course, this is still in work)
  - tagged all functions as "inline"
  - replaced "NULL" with "nullptr"
  - removed all implicit casts by being more expressive/use of castings
  - used "enum class" instead of plain enums
  - replaced "int" / "unsigned int" with fixed width integer type "std::int_fast32_t"
  - replaced "unsigned short" with fixed width integer type "std::int_fast32_t"

FURTHER NOTES:
--------------

  - removed Rasterizer version 1
  - removed new 3D API
  - removed all assert()
  - no support for stb_rect_pack.h
  - no implementation with other RAD tools
  - removed font name matching API
  - removed most of the "the macro-preprocessor-magic"

TO DO:
------

  - get rid of all these raw pointers
  - work on replacing arrays with std::vector
