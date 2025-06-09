// pch.h: プリコンパイル済みヘッダー ファイルです。
// 次のファイルは、その後のビルドのビルド パフォーマンスを向上させるため 1 回だけコンパイルされます。
// コード補完や多くのコード参照機能などの IntelliSense パフォーマンスにも影響します。
// ただし、ここに一覧表示されているファイルは、ビルド間でいずれかが更新されると、すべてが再コンパイルされます。
// 頻繁に更新するファイルをここに追加しないでください。追加すると、パフォーマンス上の利点がなくなります。

#ifndef PCH_H
#define PCH_H

#include "targetver.h"
#include <afxwin.h>
#include <afxdialogex.h>
#include "framework.h"
#include "resource.h"

// ===== ↓↓↓ 必要なものだけを残します ↓↓↓ =====
#include <vector>
#include <string>
#include <deque>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <cmath>
// ===== ↑↑↑ ここまで ↑↑↑ =======================

#endif //PCH_H



