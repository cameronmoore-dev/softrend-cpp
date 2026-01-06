
#include <vector>
#include <cstdint>

namespace _priv
{
    typedef struct 
    {
        int index;
        int count;
    } attrib_t;

    typedef struct
    {
        std::vector<attrib_t> attribs;
        uint16_t size;
        uint8_t stride;
    } vertexdef_t;

    inline vertexdef_t vertexdef = {};
}