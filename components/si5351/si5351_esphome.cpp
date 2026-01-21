#include "si5351_esphome.h"

namespace esphome {
namespace si5351 {

static const char *const TAG = "si5351";


#define AW87559_ERROR_FAILED(func) \
  if(!(func)) {                                 \
 this->mark_failed(); \
    return; \
  } 

// Return false; use outside of setup
#define AW87559_ERROR_CHECK(func) \
  if (!(func)) { \
    return false; \
  }


}  // namespace si5351
}  // namespace esphome
