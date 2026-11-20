
#include <iostream>
#include <cstdint>
#include <cstddef>
#include <algorithm>
#include <array>
#include <type_traits>

// ==========================================
// 1. 原始数据结构
// ==========================================

struct CameraFrame {
    int w, h;
    const unsigned char* buf;
};

struct RosImage {
    int w, h, step;
    const unsigned char* data;
};

struct VideoFrame {
    int w, h;
    const unsigned char* yuv;
};


// ==========================================
// 2. virt 版本：虚函数 + 多态
// ==========================================

namespace virt {

class ImageSource {
public:
    virtual ~ImageSource() = default;

    virtual int width() const = 0;
    virtual int height() const = 0;
    virtual unsigned char at(int x, int y) const = 0;
};


// Camera
class CameraSource final : public ImageSource {
private:
    CameraFrame frame_;

public:
    explicit CameraSource(CameraFrame f) : frame_(f) {}

    int width() const override {
        return frame_.w;
    }

    int height() const override {
        return frame_.h;
    }

    unsigned char at(int x, int y) const override {

        if (frame_.w <= 0 || frame_.h <= 0 ||
            x < 0 || y < 0 ||
            x >= frame_.w || y >= frame_.h ||
            frame_.buf == nullptr) {
            return 0;
        }

        std::size_t index =
            static_cast<std::size_t>(y) * frame_.w + x;

        return frame_.buf[index];
    }
};


// ROS
class RosSource final : public ImageSource 
{
private:
RosImage frame_;

public:
explicit RosSource(RosImage f) : frame_(f) {}

int width() const override 
{
    return frame_.w;
}

int height() const override 
{
    return frame_.h;
}

unsigned char at(int x, int y) const override 
{

    if (frame_.w <= 0 || frame_.h <= 0 ||
        x < 0 || y < 0 ||
        x >= frame_.w || y >= frame_.h ||
        frame_.data == nullptr ||
        frame_.step < frame_.w) 
        
        {
        return 0;
    }

    std::size_t index =static_cast<std::size_t>(y) * frame_.step + x;

    return frame_.data[index];
    }
};


// Video
class VideoSource final : public ImageSource {
private:
    VideoFrame frame_;

public:
    explicit VideoSource(VideoFrame f) : frame_(f) {}

    int width() const override {
        return frame_.w;
    }

    int height() const override {
        return frame_.h;
    }

    unsigned char at(int x, int y) const override {

        if (frame_.w <= 0 || frame_.h <= 0 ||
            x < 0 || y < 0 ||
            x >= frame_.w || y >= frame_.h ||
            frame_.yuv == nullptr) {
            return 0;
        }

        std::size_t index =
            static_cast<std::size_t>(y) * frame_.w + x;

        int value =
            std::max(static_cast<int>(frame_.yuv[index]) - 16, 0);

        return static_cast<unsigned char>(value);
    }
};


// 统一预处理
std::uint64_t preprocess(const ImageSource& src) {

    std::uint64_t sum = 0;

    const int w = src.width();
    const int h = src.height();

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {

            sum += src.at(x, y);

        }
    }

    return sum;
}

} // namespace virt



// ==========================================
// 3. tmpl 版本：模板 + 编译期判断
// ==========================================

namespace tmpl {

template <typename Frame>
std::uint64_t preprocess(const Frame& f) {

    static_assert(
        std::is_same_v<Frame, CameraFrame> ||
        std::is_same_v<Frame, RosImage> ||
        std::is_same_v<Frame, VideoFrame>,
        "Unsupported image frame type"
    );

    std::uint64_t sum = 0;

    if (f.w <= 0 || f.h <= 0) {
        return sum;
    }

    for (int y = 0; y < f.h; ++y) {
        for (int x = 0; x < f.w; ++x) {

            unsigned char pixel = 0;

            // Camera
            if constexpr (std::is_same_v<Frame, CameraFrame>) {

                if (f.buf != nullptr) {

                    std::size_t index =
                        static_cast<std::size_t>(y) * f.w + x;

                    pixel = f.buf[index];
                }
            }

            // ROS
            else if constexpr (std::is_same_v<Frame, RosImage>) {

                if (f.data != nullptr && f.step >= f.w) {

                    std::size_t index =
                        static_cast<std::size_t>(y) * f.step + x;

                    pixel = f.data[index];
                }
            }

            // Video
            else if constexpr (std::is_same_v<Frame, VideoFrame>) {

                if (f.yuv != nullptr) {

                    std::size_t index =
                        static_cast<std::size_t>(y) * f.w + x;

                    int value =
                        std::max(static_cast<int>(f.yuv[index]) - 16, 0);

                    pixel = static_cast<unsigned char>(value);
                }
            }

            sum += pixel;
        }
    }

    return sum;
}

} // namespace tmpl



// ==========================================
// 4. 测试程序
// ==========================================

int main() {

    // 准备测试数据

    const std::array<unsigned char, 4> camera_data{
        17, 18, 19, 20
    };

    // ROS 每行3字节，最后一个字节是 padding
    const std::array<unsigned char, 6> ros_data{
        17, 18, 99,
        19, 20, 99
    };

    const std::array<unsigned char, 4> video_data{
        17, 18, 15, 30
    };


    // 创建三种图像对象

    CameraFrame camera{
        2, 2, camera_data.data()
    };

    RosImage ros{
        2, 2, 3, ros_data.data()
    };

    VideoFrame video{
        2, 2, video_data.data()
    };


    // virt 版本

    virt::CameraSource camera_source(camera);
    virt::RosSource ros_source(ros);
    virt::VideoSource video_source(video);

    std::uint64_t v1 =
        virt::preprocess(camera_source);

    std::uint64_t v2 =
        virt::preprocess(ros_source);

    std::uint64_t v3 =
        virt::preprocess(video_source);


    // tmpl 版本

    std::uint64_t t1 =
        tmpl::preprocess(camera);

    std::uint64_t t2 =
        tmpl::preprocess(ros);

    std::uint64_t t3 =
        tmpl::preprocess(video);


    // 输出结果

    std::cout << "===== virt version =====\n";

    std::cout << "Camera: " << v1 << '\n';
    std::cout << "ROS:    " << v2 << '\n';
    std::cout << "Video:  " << v3 << '\n';


    std::cout << "\n===== tmpl version =====\n";

    std::cout << "Camera: " << t1 << '\n';
    std::cout << "ROS:    " << t2 << '\n';
    std::cout << "Video:  " << t3 << '\n';


    // 验证两套 API 结果一致
    bool equal =
        (v1 == t1) &&
        (v2 == t2) &&
        (v3 == t3);

    // 同时验证预期结果
    bool expected =
        (v1 == 74) &&
        (v2 == 74) &&
        (v3 == 17);

    if (equal && expected) {

        std::cout << "\nResult: PASS\n";

        return 0;
    }

    std::cout << "\nResult: FAIL\n";

    return 1;
}
