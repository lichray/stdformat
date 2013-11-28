#include "../__aux.h"

enum _unscoped { label = -1 };
enum class _scoped { label };

using namespace stdex::detail;

static_assert(is_unscoped_enum<_unscoped>{}, "");
static_assert(not is_unscoped_enum<_scoped>{}, "");
static_assert(not is_unscoped_enum<int>{}, "");

static_assert(is_nonarrow_convertible<unsigned, long>{}, "");
static_assert(not is_nonarrow_convertible<unsigned, int>{}, "");
static_assert(not is_nonarrow_convertible<double, float>{}, "");
static_assert(not is_nonarrow_convertible<long, float>{}, "");
static_assert(not is_nonarrow_convertible<float, long>{}, "");

static_assert(is_nonarrow_convertible<_unscoped, int>{}, "");
static_assert(not is_nonarrow_convertible<_unscoped, float>{}, "");
static_assert(not is_nonarrow_convertible<int, _unscoped>{}, "");
static_assert(not is_nonarrow_convertible<_scoped, int>{}, "");
static_assert(not is_nonarrow_convertible<int, _scoped>{}, "");

static_assert(pow2_roundup(0) == 0, "");
static_assert(pow2_roundup(1) == 1, "");
static_assert(pow2_roundup(63) == 64, "");
static_assert(pow2_roundup(64) == 64, "");
static_assert(pow2_roundup(65) == 128, "");
static_assert(pow2_roundup('\x80') == 128, "");
static_assert(pow2_roundup('\x81') == 0, "");

int main() {}
