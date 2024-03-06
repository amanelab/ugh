/*-
 * This file is part of Ugh project. For license details, see the file
 * 'LICENSE' in this package.
 */

#include <windows.h>

#include <stdexcept>

#include "app.h"

int WINAPI
wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int) {
  int ret = EXIT_SUCCESS;

  try {
    ugh::App app(hInstance);
    app.run();
  } catch (const std::exception& e) {
    ::MessageBoxA(NULL, e.what(), "Ugh - Error", MB_ICONERROR);
    ret = EXIT_FAILURE;
  }

  return ret;
}
