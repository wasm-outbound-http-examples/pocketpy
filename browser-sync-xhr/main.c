// tested with PocketPy v2.0.0 (commit 7d6a590e39afca29fb5220aa3d2c70083bd84f1b)
#include <stdio.h>
#include <string.h>
#include <emscripten/fetch.h>
#include "pocketpy.h"

static bool httpget(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    const char* url = py_tostr(py_arg(0));

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes =
        EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS | EMSCRIPTEN_FETCH_REPLACE;
    emscripten_fetch_t* fetch = emscripten_fetch(&attr, url);
    if(fetch->status == 200) {
        py_newstrv(py_retval(), (c11_sv){(const char*)fetch->data, fetch->numBytes});
        emscripten_fetch_close(fetch);
        return true;
    }

    emscripten_fetch_close(fetch);
    return py_exception(tp_RuntimeError, "HTTP failure status code: %d.", fetch->status);
}

int main() {
    py_initialize();

    py_Ref mod = py_newmodule("http");
    py_bindfunc(mod, "get", httpget);

    bool ok = py_exec("import http; print(http.get('https://httpbin.org/anything'))",
                      "<string>",
                      EXEC_MODE,
                      NULL);
    if(!ok) goto __ERROR;

    py_finalize();
    return 0;

__ERROR:
    py_printexc();
    py_finalize();
    return 1;
}
