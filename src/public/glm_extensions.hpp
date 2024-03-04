#include "glm/ext/vector_uint4_sized.hpp"

namespace std
{
	template<> struct hash<glm::u8vec4>
	{
		size_t operator()(const glm::u8vec4& k) const
		{
			return ((hash<uint8_t>()(k.x)
				^ (hash<uint8_t>()(k.y) << 1)) >> 1)
				^ (hash<uint8_t>()(k.z) << 1)
				^ (hash<uint8_t>()(k.w) << 1);
		}
	};
}