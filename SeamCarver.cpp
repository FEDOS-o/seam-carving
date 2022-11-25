#include "SeamCarver.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <utility>

const double ENERGY_INF = std::numeric_limits<double>::max();
const double EPS = 1e-7;
//я пробовал хранить это в самой структуре, но тогда findseam нельзя было сделать конст

SeamCarver::SeamCarver(Image image)
    : m_image(std::move(image))
{
}

const Image & SeamCarver::GetImage() const
{
    return m_image;
}

size_t SeamCarver::GetImageWidth() const
{
    return m_image.m_table.size();
}

size_t SeamCarver::GetImageHeight() const
{
    return ((m_image.m_table.empty()) ? 0 : m_image.m_table[0].size());
}

double delta(Image::Pixel p1, Image::Pixel p2)
{
    double result = 0;
    result += std::pow(p1.m_red - p2.m_red, 2);
    result += std::pow(p1.m_green - p2.m_green, 2);
    result += std::pow(p1.m_blue - p2.m_blue, 2);
    return result;
}

double SeamCarver::GetPixelEnergy(size_t columnId, size_t rowId) const
{
    size_t width = GetImageWidth(), height = GetImageHeight();
    size_t c1 = (columnId + 1) % width,
           c2 = (columnId + width - 1) % width;
    size_t r1 = (rowId + 1) % height,
           r2 = (rowId + height - 1) % height;
    double delta_x = delta(m_image.GetPixel(c1, rowId),
                           m_image.GetPixel(c2, rowId));
    double delta_y = delta(m_image.GetPixel(columnId, r1),
                           m_image.GetPixel(columnId, r2));
    return std::sqrt(delta_x + delta_y);
}

SeamCarver::Seam SeamCarver::FindHorizontalSeam() const
{
    return FindSeam(true);
}

SeamCarver::Seam SeamCarver::FindVerticalSeam() const
{
    return FindSeam(false);
}

void SeamCarver::RemoveHorizontalSeam(const Seam & seam)
{
    for (size_t x = 0; x < GetImageWidth(); x++) {
        m_image.m_table[x].erase(m_image.m_table[x].begin() + seam[x]);
    }
}

void SeamCarver::RemoveVerticalSeam(const Seam & seam)
{
    for (size_t y = 0; y < GetImageHeight(); y++) {
        for (size_t x = seam[y]; x < GetImageWidth() - 1; x++) {
            m_image.m_table[x][y] = m_image.m_table[x + 1][y];
        }
    }
    m_image.m_table.pop_back();
}
SeamCarver::Seam SeamCarver::FindSeam(bool is_transposed) const
{
#define GetPixelEnergy(x, y) GetPixelEnergy(x, y, is_transposed)
    size_t width = GetImageWidth(), height = GetImageHeight();
    if (is_transposed) {
        std::swap(width, height);
    }
    std::vector<std::vector<double>> dp(width + 2, std::vector<double>(height, ENERGY_INF));
    for (size_t x = 0; x < width; x++) {
        dp[x + 1][0] = GetPixelEnergy(x, 0);
    }
    for (size_t y = 1; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            dp[x + 1][y] = GetPixelEnergy(x, y) + std::min({dp[x][y - 1], dp[x + 1][y - 1], dp[x + 2][y - 1]});
        }
    }
    double min = ENERGY_INF;
    size_t min_index = -1;
    for (size_t x = 0; x < width; x++) {
        if (dp[x + 1][height - 1] < min) {
            min = dp[x + 1][height - 1];
            min_index = x;
        }
    }
    Seam result;
    result.reserve(height);
    size_t current = min_index;
    result.push_back(current);
    for (size_t y = height - 1; y > 0; y--) {
        if (std::abs(dp[current][y - 1] - (dp[current + 1][y] - GetPixelEnergy(current, y))) < EPS) {
            current -= 1;
        }
        else if (std::abs(dp[current + 1][y - 1] - (dp[current + 1][y] - GetPixelEnergy(current, y))) < EPS) {
            //do nothing
        }
        else {
            current += 1;
        }
        result.push_back(current);
    }
    std::reverse(result.begin(), result.end());
#undef GetPixelEnergy
    return result;
}
double SeamCarver::GetPixelEnergy(size_t columnId, size_t rowId, bool is_transposed) const
{
    if (is_transposed) {
        std::swap(columnId, rowId);
    }
    return GetPixelEnergy(columnId, rowId);
}
