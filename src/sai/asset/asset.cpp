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
  auto len = SDL_RWseek(file, 0, RW_SEEK_END);
  a->data.resize(len);
  SDL_RWseek(file, 0, RW_SEEK_SET);
  SDL_RWread(file, a->data.data(), len, 1);
  SDL_RWclose(file);
  a->is_loaded = true;

  return true;
}

}  // namespace sai::asset
