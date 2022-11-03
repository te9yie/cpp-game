#pragma once
#include <string>

#include "../handle.h"

namespace sai::asset {

// Asset.
struct Asset {
  std::string path;
};

using AssetHandle = Handle<Asset>;
using AssetRemoveObserver = HandleRemoveObserver<Asset>;
using AssetStorage = HandleStorage<Asset>;

}  // namespace sai::asset