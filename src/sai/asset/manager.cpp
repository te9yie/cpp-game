#include "manager.h"

#include <memory>

#include "t9/hash.h"

namespace sai::asset {

AssetHandle Manager::load(std::string_view path) {
  auto hash = t9::fxhash(path.data(), path.length());
  auto r = path_map_.equal_range(hash);
  for (auto it = r.first; it != r.second; ++it) {
    auto id = it->second;
    auto a = assets_.get(id);
    if (a->path == path) return assets_.make_handle(id);
  }
  auto a = std::make_unique<Asset>();
  a->path = path;
  auto handle = assets_.add(std::move(a));
  path_map_.emplace(hash, handle.id);
  return handle;
}

AssetHandle Manager::find(std::string_view path) {
  auto hash = t9::fxhash(path.data(), path.length());
  auto r = path_map_.equal_range(hash);
  for (auto it = r.first; it != r.second; ++it) {
    auto id = it->second;
    auto a = assets_.get(id);
    if (a->path == path) return assets_.make_handle(id);
  }
  return AssetHandle{};
}

void Manager::update() { assets_.update(this); }

/*virtual*/ void Manager::on_remove(Asset* asset) /*override*/ {
  auto hash = t9::fxhash(asset->path.c_str(), asset->path.length());
  auto r = path_map_.equal_range(hash);
  for (auto it = r.first; it != r.second; ++it) {
    auto a = assets_.get(it->second);
    if (a->path == asset->path) {
      path_map_.erase(it);
      break;
    }
  }
}

void update_manager(Manager* manager) { manager->update(); }

}  // namespace sai::asset