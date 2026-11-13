#include <opencv2/opencv.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace
{
// Hough circle parameters tuned for the four round holes in holes.png.
constexpr int kMinDist = 60;     // minimum distance between two hole centers
constexpr int kCannyHigh = 100;  // Hough param1: upper Canny threshold
constexpr int kAccumThresh = 20; // Hough param2: accumulator threshold
constexpr int kMinRadius = 15;
constexpr int kMaxRadius = 50;

// Read/write through imdecode/imencode so non-ASCII (Chinese) paths work.
cv::Mat readImage(const fs::path& path)
{
    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        return {};
    }

    const std::vector<unsigned char> bytes{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()};
    return cv::imdecode(bytes, cv::IMREAD_COLOR);
}

bool writeImage(const fs::path& path, const cv::Mat& image)
{
    std::vector<unsigned char> bytes;
    if (!cv::imencode(".png", image, bytes))
    {
        return false;
    }

    std::ofstream output(path, std::ios::binary);
    if (!output)
    {
        return false;
    }

    output.write(reinterpret_cast<const char*>(bytes.data()),
                 static_cast<std::streamsize>(bytes.size()));
    return output.good();
}
} // namespace

int main(int argc, char* argv[])
{
    fs::path imagePath = "holes.png";
    bool showWindows = true;

    for (int i = 1; i < argc; ++i)
    {
        const std::string argument = argv[i];
        if (argument == "--no-window")
        {
            showWindows = false;
        }
        else
        {
            imagePath = argument;
        }
    }

    const cv::Mat source = readImage(imagePath);
    if (source.empty())
    {
        std::cerr << "Cannot read image: " << imagePath.string() << '\n';
        return 1;
    }

    cv::Mat gray;
    cv::cvtColor(source, gray, cv::COLOR_BGR2GRAY);

    // Slight blur reduces noise before edge detection, which feeds the Hough
    // transform. Round holes then show up as strong, closed circular edges.
    cv::Mat blurred;
    cv::GaussianBlur(gray, blurred, cv::Size(7, 7), 1.5);

    cv::Mat edges;
    cv::Canny(blurred, edges, kCannyHigh / 2.0, kCannyHigh);

    std::vector<cv::Vec3f> found;
    cv::HoughCircles(blurred, found, cv::HOUGH_GRADIENT, /*dp=*/1,
                     kMinDist, kCannyHigh, kAccumThresh, kMinRadius,
                     kMaxRadius);

    std::vector<cv::Vec3f> holes(found.begin(), found.end());
    std::sort(holes.begin(), holes.end(), [](const cv::Vec3f& a,
                                             const cv::Vec3f& b)
    {
        if (std::abs(a[1] - b[1]) < 40.0f)
        {
            return a[0] < b[0];
        }
        return a[1] < b[1];
    });

    cv::Mat result = source.clone();
    for (std::size_t i = 0; i < holes.size(); ++i)
    {
        const cv::Point center(cvRound(holes[i][0]), cvRound(holes[i][1]));
        const int radius = cvRound(holes[i][2]);

        cv::circle(result, center, radius, cv::Scalar(0, 200, 0), 2,
                   cv::LINE_AA);
        cv::circle(result, center, 2, cv::Scalar(0, 0, 255), -1,
                   cv::LINE_AA);

        const std::string number = std::to_string(i + 1);
        cv::putText(result, number,
                    cv::Point(center.x - 8, std::max(20, center.y - radius - 6)),
                    cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 255), 2,
                    cv::LINE_AA);
    }

    const std::string countText = "Holes: " + std::to_string(holes.size());
    cv::putText(result, countText, cv::Point(15, 32),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), 2,
                cv::LINE_AA);

    const fs::path outputDirectory = "output";
    fs::create_directories(outputDirectory);
    const bool saved =
        writeImage(outputDirectory / "01_gray.png", gray) &&
        writeImage(outputDirectory / "02_edges.png", edges) &&
        writeImage(outputDirectory / "03_result.png", result);

    if (!saved)
    {
        std::cerr << "Failed to save output images\n";
        return 1;
    }

    std::cout << "Hole count: " << holes.size() << '\n';
    for (std::size_t i = 0; i < holes.size(); ++i)
    {
        std::cout << "Hole " << i + 1 << ": center=(" << cvRound(holes[i][0])
                  << ", " << cvRound(holes[i][1]) << ") radius="
                  << cvRound(holes[i][2]) << '\n';
    }

    if (showWindows)
    {
        cv::imshow("1 Gray", gray);
        cv::imshow("2 Edges", edges);
        cv::imshow("3 Result", result);
        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}
