#pragma once
#include <string>

#include "../handle.h"
#include "../job/job.h"

namespace sai::asset {

// Asset.
struct Asset {
  std::string path;
  std::vector<std::uint8_t> data;
  bool is_not_found = false;
  bool is_loaded = false;

  Asset(std::string_view p) : path(p) {}
};

// LoadAssetJob.
class LoadAssetJob : public job::Job {
 private:
  std::shared_ptr<Asset> asset_;

 public:
  explicit LoadAssetJob(std::shared_ptr<Asset> p);

 protected:
  virtual void on_exec() override;
};

// load_asset.
bool load_asset(Asset* a);

using AssetHandle = Handle<Asset>;
using AssetRemoveObserver = HandleRemoveObserver<Asset>;
using AssetStorage = HandleStorage<Asset>;

}  // namespace sai::asset
