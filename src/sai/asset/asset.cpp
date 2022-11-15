#include "asset.h"

#include <SDL.h>

namespace sai::asset {

/*explicit*/ LoadAssetJob::LoadAssetJob(std::shared_ptr<Asset> p)
    : job::Job("LoadAssetJob"), asset_(std::move(p)) {}

/*virtual*/ void LoadAssetJob::on_exec() /*override*/ {
  load_asset(asset_.get());
}

bool load_asset(Asset* a) {
  SDL_RWops* file = SDL_RWFromFile(a->path.c_str(), "rb");
  if (!file) {
    a->is_not_found = true;
    return false;
  }
  auto size = SDL_RWsize(file);
  a->data.resize(size);
  Sint64 total = 0;
  while (total < size) {
    auto n = SDL_RWread(file, a->data.data() + total, 1, size - total);
    if (n == 0) break;
    total += n;
  }
  a->is_loaded = true;
  return total == size;
}

}  // namespace sai::asset
