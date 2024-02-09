#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <cstring>
#include <limits>

#include <mutex>

class framebuffer {
private:
    int dev;
    int bytes;
    char* fb = nullptr;
    char* buffer = nullptr;
    double* z_buffer = nullptr;

    int bytes_per_pixel;
    int xoffset, yoffset, line_length;
    int xsize, ysize;

    std::string error;

    std::mutex mutex;
public:
    framebuffer(const std::string& name) {
        dev = open(name.c_str(), O_RDWR);
        if(dev == -1) error = "failed to open framebuffer";
        else {
            fb_fix_screeninfo fix_info;
            fb_var_screeninfo var_info;
            if(ioctl(dev, FBIOGET_FSCREENINFO, &fix_info) == -1) {
                error = "cannot get screen info";
            } else if(ioctl(dev, FBIOGET_VSCREENINFO, &var_info) == -1) {
                error = "cannot get screen info";
            } else {
                bytes_per_pixel = var_info.bits_per_pixel / 8;
                xoffset = var_info.xoffset;
                yoffset = var_info.yoffset;
                line_length = fix_info.line_length;

                xsize = var_info.xres;
                ysize = var_info.yres;

                bytes = xsize * ysize * var_info.bits_per_pixel / 8;
                fb = (char*)mmap(0, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, dev, 0);
                buffer = (char*)malloc(bytes);

                for(int i = 0; i < bytes; ++i) buffer[i] = 0;

                z_buffer = (double*)malloc(xsize * ysize * sizeof(double));
                for(int i = 0; i < xsize * ysize; ++i) {
                    z_buffer[i] = -std::numeric_limits<double>::infinity();
                }
            }
        }
    }

    ~framebuffer() {
        free(z_buffer);
        free(buffer);
        munmap(fb, bytes);
        close(dev);
    }

    int width() const {
        return xsize;
    }

    int height() const {
        return ysize;
    }

    void set_pixel(int x, int y, double z, int color) {
        if(x >= 0 && y >= 0) {
            const size_t pos = (x + xoffset) * (bytes_per_pixel)
                + (ysize - y + yoffset) * line_length;

            int r = (color >> 0) & 0xFF;
            int g = (color >> 8) & 0xFF;
            int b = (color >> 16) & 0xFF;

            const size_t index = x + y * ysize;

            if(z > z_buffer[index]) {
                z_buffer[index] = z;
                *(buffer + pos + 0) = r;
                *(buffer + pos + 1) = g;
                *(buffer + pos + 2) = b;
                *(buffer + pos + 3) = 0;
            }
        }
    }

    void swap() {
        memcpy(fb, buffer, bytes);
    }

    void clear() {
        memset(buffer, 0, bytes);

        const auto inf = -std::numeric_limits<double>::infinity();

        for(int i = 0; i < xsize * ysize; ++i) z_buffer[i] = inf;
    }

    bool valid() const {
        return fb && error.empty();
    }

    const std::string& get_error() const {
        return error;
    }
};
