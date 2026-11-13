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
constexpr int kBorder = 8;
constexpr double kMinArea = 1000.0;

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
    fs::path imagePath = "coins.png";
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

    cv::Mat smoothed;
    cv::GaussianBlur(gray, smoothed, cv::Size(5, 5), 0);

    // Coins are brighter than the background, so ordinary Otsu is used.
    cv::Mat binary;
    const double thresholdValue = cv::threshold(
        smoothed, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    // Remove the bright frame at the image boundary.
    cv::Mat foreground = binary.clone();
    foreground.rowRange(0, kBorder).setTo(0);
    foreground.rowRange(foreground.rows - kBorder, foreground.rows).setTo(0);
    foreground.colRange(0, kBorder).setTo(0);
    foreground.colRange(foreground.cols - kBorder, foreground.cols).setTo(0);

    const cv::Mat kernel = cv::getStructuringElement(
        cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::Mat cleaned;
    cv::morphologyEx(foreground, cleaned, cv::MORPH_CLOSE, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(cleaned.clone(), contours, cv::RETR_EXTERNAL,
                     cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Rect> coins;
    for (const auto& contour : contours)
    {
        if (cv::contourArea(contour) >= kMinArea)
        {
            coins.push_back(cv::boundingRect(contour));
        }
    }

    std::sort(coins.begin(), coins.end(), [](const cv::Rect& a,
                                             const cv::Rect& b)
    {
        if (a.y == b.y)
        {
            return a.x < b.x;
        }
        return a.y < b.y;
    });

    cv::Mat result = source.clone();
    for (std::size_t i = 0; i < coins.size(); ++i)
    {
        const cv::Rect& coin = coins[i];
        cv::rectangle(result, coin, cv::Scalar(0, 200, 0), 2, cv::LINE_AA);

        const std::string number = std::to_string(i + 1);
        const cv::Point textPosition(coin.x, std::max(20, coin.y - 6));
        cv::putText(result, number, textPosition, cv::FONT_HERSHEY_SIMPLEX,
                    0.7, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
    }

    const std::string countText = "Coins: " + std::to_string(coins.size());
    cv::putText(result, countText, cv::Point(15, 32),
                cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 255),
                2, cv::LINE_AA);

    const fs::path outputDirectory = "output";
    fs::create_directories(outputDirectory);
    const bool saved =
        writeImage(outputDirectory / "01_binary.png", binary) &&
        writeImage(outputDirectory / "02_cleaned.png", cleaned) &&
        writeImage(outputDirectory / "03_result.png", result);

    if (!saved)
    {
        std::cerr << "Failed to save output images\n";
        return 1;
    }

    std::cout << "Otsu threshold: " << thresholdValue << '\n'
              << "Coin count: " << coins.size() << '\n';
    for (std::size_t i = 0; i < coins.size(); ++i)
    {
        std::cout << "Coin " << i + 1 << ": "
                  << coins[i].width << " x " << coins[i].height << '\n';
    }

    if (showWindows)
    {
        cv::imshow("1 Binary", binary);
        cv::imshow("2 Cleaned", cleaned);
        cv::imshow("3 Result", result);
        cv::waitKey(0);
        cv::destroyAllWindows();
    }

    return 0;
}
