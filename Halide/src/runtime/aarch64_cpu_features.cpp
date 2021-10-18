#include "HalideRuntime.h"
#include "cpu_features.h"

namespace Halide { namespace Runtime { namespace Internal {

WEAK CpuFeatures halide_get_cpu_features() {
    // AArch64 has no CPU-specific Features.
    return CpuFeatures();
}

}}} // namespace Halide::Runtime::Internal