# Source Server Query

C++ Library for querying Source game servers.
Currently only supports rules and player queries and targets VS 2010 (but I'm sure it'll work with any other version).

### Why?
The [C alternative](https://developer.valvesoftware.com/wiki/Source_Server_Query_Library) looks horrible (global variables everywhere!) and has a bunch of unneeded stuff. I want a clean, basic and elegant C++ version.

### How?
Run install.bat (this registers the LIBSSQ_PATH user env variable), then add $(LIBSSQ_PATH)\public as include and library path to your project.

### Example
See ssquery project for a completely worked out example.
