#include <uv.h>
#include <stdio.h>
#include <stdlib.h>

uv_loop_t* loop;

uv_fs_t open_req;
uv_fs_t read_req;
uv_fs_t close_req;

char buffer[32];
uv_buf_t iov;
int64_t offset = 0;

void on_open(uv_fs_t* req);
void on_read(uv_fs_t* req);
void on_close(uv_fs_t* req);

int main(int argc, char **argv) {
    loop = uv_default_loop();

    char* file = argv[1];

    uv_fs_open(loop, &open_req, file, O_RDONLY, 0, on_open);

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}

void on_open(uv_fs_t* req) {
    if (req->result >= 0) {
        // read file
        memset(buffer, 0, sizeof (buffer));
        iov = uv_buf_init(buffer, sizeof (buffer));
        offset = 0;
        uv_fs_read(loop, &read_req, open_req.result, &iov, 1, offset, on_read);
    } else {
        // handle error
        fprintf(stderr, "open failed:%s\n", uv_strerror(req->result));
        exit(1);
    }
      
    uv_fs_req_cleanup(req);
}

void on_read(uv_fs_t* req) {
    if (req->result >= 0) {
        offset += req->result;
        fwrite(buffer, 1, req->result, stdout);
    } else {
        // handle error
        fprintf(stderr, "read failed:%s\n", uv_strerror(req->result));
        exit(1);
    }

    uv_fs_req_cleanup(req);

    if (req->result >= sizeof (buffer)) {
        // try again
        uv_fs_read(loop, &read_req, open_req.result, &iov, 1, offset, on_read);
    } else {
        uv_fs_close(loop, &close_req, open_req.result, on_close);
    }
}

void on_close(uv_fs_t* req) {
    if (req->result < 0) {
        // handle error
        fprintf(stderr, "close failed:%s\n", uv_strerror(req->result));
        exit(1);
    }

    // It seems that we don't need to call uv_close, because there are no handles.
    // I could not find uv_close in official test case here(https://github.com/libuv/libuv/blob/v1.x/test/test-fs.c)
    uv_fs_req_cleanup(req);
}

