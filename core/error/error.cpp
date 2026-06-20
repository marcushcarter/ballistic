#include <core/error/error.h>
#include <iterator>

namespace ballistic {

const char *error_names[] = {
	"OK",
	"Failed",
};

static_assert(std::size(error_names) == static_cast<size_t>(Error::Max));

}