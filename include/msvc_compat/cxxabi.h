#pragma once

// This is meant to work around a bug with MSVC and boost

namespace abi {
	inline char* __cxa_demangle(
		const char* /*mangled_name*/,
		char* /*buf*/,
		size_t* /*n*/,
		int* status
	) {
		if (status)
			*status = -1;
		return nullptr;
	}
} // namespace abi