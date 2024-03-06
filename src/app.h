/*-
 * This file is part of Ugh project. For license details, see the file
 * 'LICENSE' in this package.
 */

#ifndef UGH_APP_H_
#define UGH_APP_H_

#pragma once

#include <windows.h>

namespace ugh {

class App {
  App() = delete;
public:
  App(HINSTANCE instance);
  ~App();
public:
  void run();
private:
  class Impl;
  Impl* impl_;
};

}

#endif  // UGH_APP_H_
