# aseprite-rgba-converter
Tiny project to convert a .ase to a RGBA Byte array

Please checkout http://www.aseprite.org/, or https://github.com/aseprite/aseprite for more infromation about Aseprite!

---

This project is intended as a intermediary library for converting ".ase" files directly into SFML::Sprite.

Project implements parser for the [ase file format](https://github.com/aseprite/aseprite/blob/master/docs/files/ase.txt) and converts it to doc-lite.h objects which is based on the aseprite doc module.

Currently a bit of a work in progress, but it's functional 'enough' to use.

|                     | Is Supported? ( X is yes ) |
|:-------------------:|:--------------------------:|
| Frame-Tags          | X                          |
| Animations          | X                          |
| Color Mode: RGBA    | X                          |
| Color Mode: Grey    |                            |
| Color Mode: Indexed |                            |
| Pallets             |                            |
| Transparent BG      | X                          |
| Layers              |                            |
| Layer-Blending      |                            |
| Colored BG          |                            |


Example usage:
```
#include "loader.h"

aseprite::Sprite s = aseprite::load_sprite_from_file("AwesomeSprite.ase");
// You can do stuff with each frame's pixels with: s.frames[0].pixels, this is a 32-bit RGBA vector.
```


### Descriptions:

integration_test: Tests with version 1.1 ase files.

doc-lite.h: A lite copy of aespirte's doc module.

loader.h: Meat of the project, provides `load_sprite(<buffer>)` and `load_sprite_from_file(<filename>)`.
