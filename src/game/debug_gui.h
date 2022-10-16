#pragma once

namespace game {

// ImGui にマルチスレッドでアクセスしないための識別子
// 引数に `MutexRes<DebugGui>` をもつ関数は同時に実行されない。
struct DebugGui {};

}  // namespace game