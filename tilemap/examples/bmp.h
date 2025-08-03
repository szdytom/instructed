#ifndef BMP_H
#define BMP_H

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

/**
 * @brief Simple BMP image writer - single header library
 *
 * Usage:
 *   BmpWriter bmp(width, height);
 *   bmp.set_pixel(x, y, r, g, b);
 *   bmp.save("output.bmp");
 */
class BmpWriter {
private:
	int width_;
	int height_;
	std::vector<std::uint8_t> pixels_;

	struct BmpHeader {
		// BMP file header (14 bytes)
		std::uint8_t signature[2] = {'B', 'M'};
		std::uint32_t file_size;
		std::uint32_t reserved = 0;
		std::uint32_t data_offset = 54; // Header size

		// DIB header (40 bytes)
		std::uint32_t dib_header_size = 40;
		std::int32_t width;
		std::int32_t height;
		std::uint16_t planes = 1;
		std::uint16_t bits_per_pixel = 24;
		std::uint32_t compression = 0;
		std::uint32_t image_size = 0;
		std::int32_t x_pixels_per_meter = 2835; // 72 DPI
		std::int32_t y_pixels_per_meter = 2835; // 72 DPI
		std::uint32_t colors_used = 0;
		std::uint32_t colors_important = 0;
	};

public:
	/**
	 * @brief Create a BMP writer with specified dimensions
	 * @param width Image width in pixels
	 * @param height Image height in pixels
	 */
	BmpWriter(int width, int height): width_(width), height_(height) {
		pixels_.resize(width * height * 3, 0); // RGB format
	}

	/**
	 * @brief Set a pixel color
	 * @param x X coordinate (0 to width-1)
	 * @param y Y coordinate (0 to height-1)
	 * @param r Red component (0-255)
	 * @param g Green component (0-255)
	 * @param b Blue component (0-255)
	 */
	void set_pixel(
		int x, int y, std::uint8_t r, std::uint8_t g, std::uint8_t b
	) {
		if (x >= 0 && x < width_ && y >= 0 && y < height_) {
			// BMP stores pixels bottom-to-top
			int flipped_y = height_ - 1 - y;
			int index = (flipped_y * width_ + x) * 3;
			pixels_[index] = b;     // Blue
			pixels_[index + 1] = g; // Green
			pixels_[index + 2] = r; // Red
		}
	}

	/**
	 * @brief Set a pixel using a grayscale value
	 * @param x X coordinate
	 * @param y Y coordinate
	 * @param gray Grayscale value (0-255)
	 */
	void set_pixel_gray(int x, int y, std::uint8_t gray) {
		set_pixel(x, y, gray, gray, gray);
	}

	/**
	 * @brief Set a pixel using a normalized value [0,1]
	 * @param x X coordinate
	 * @param y Y coordinate
	 * @param value Normalized value (0.0 = black, 1.0 = white)
	 */
	void set_pixel_normalized(int x, int y, double value) {
		value = std::max(0.0, std::min(1.0, value)); // Clamp to [0,1]
		std::uint8_t gray = static_cast<std::uint8_t>(value * 255);
		set_pixel_gray(x, y, gray);
	}

	/**
	 * @brief Fill the entire image with a color
	 * @param r Red component (0-255)
	 * @param g Green component (0-255)
	 * @param b Blue component (0-255)
	 */
	void fill(std::uint8_t r, std::uint8_t g, std::uint8_t b) {
		for (int y = 0; y < height_; ++y) {
			for (int x = 0; x < width_; ++x) {
				set_pixel(x, y, r, g, b);
			}
		}
	}

	/**
	 * @brief Draw a rectangle
	 * @param x1 Left coordinate
	 * @param y1 Top coordinate
	 * @param x2 Right coordinate
	 * @param y2 Bottom coordinate
	 * @param r Red component (0-255)
	 * @param g Green component (0-255)
	 * @param b Blue component (0-255)
	 */
	void draw_rect(
		int x1, int y1, int x2, int y2, std::uint8_t r, std::uint8_t g,
		std::uint8_t b
	) {
		for (int y = y1; y <= y2; ++y) {
			for (int x = x1; x <= x2; ++x) {
				set_pixel(x, y, r, g, b);
			}
		}
	}

	/**
	 * @brief Save the image to a BMP file
	 * @param filename Output filename
	 * @return true if successful, false otherwise
	 */
	bool save(const std::string &filename) {
		std::ofstream file(filename, std::ios::binary);
		if (!file.is_open()) {
			return false;
		}

		// Calculate row padding (BMP rows must be multiple of 4 bytes)
		int row_size = width_ * 3;
		int padding = (4 - (row_size % 4)) % 4;
		int padded_row_size = row_size + padding;

		// Prepare header
		BmpHeader header;
		header.width = width_;
		header.height = height_;
		header.image_size = padded_row_size * height_;
		header.file_size = 54 + header.image_size;

		// Write BMP file header
		file.write(reinterpret_cast<const char *>(header.signature), 2);
		file.write(reinterpret_cast<const char *>(&header.file_size), 4);
		file.write(reinterpret_cast<const char *>(&header.reserved), 4);
		file.write(reinterpret_cast<const char *>(&header.data_offset), 4);

		// Write DIB header
		file.write(reinterpret_cast<const char *>(&header.dib_header_size), 4);
		file.write(reinterpret_cast<const char *>(&header.width), 4);
		file.write(reinterpret_cast<const char *>(&header.height), 4);
		file.write(reinterpret_cast<const char *>(&header.planes), 2);
		file.write(reinterpret_cast<const char *>(&header.bits_per_pixel), 2);
		file.write(reinterpret_cast<const char *>(&header.compression), 4);
		file.write(reinterpret_cast<const char *>(&header.image_size), 4);
		file.write(
			reinterpret_cast<const char *>(&header.x_pixels_per_meter), 4
		);
		file.write(
			reinterpret_cast<const char *>(&header.y_pixels_per_meter), 4
		);
		file.write(reinterpret_cast<const char *>(&header.colors_used), 4);
		file.write(reinterpret_cast<const char *>(&header.colors_important), 4);

		// Write pixel data with padding
		std::vector<std::uint8_t> padding_bytes(padding, 0);
		for (int y = 0; y < height_; ++y) {
			// Write row data
			file.write(
				reinterpret_cast<const char *>(&pixels_[y * width_ * 3]),
				row_size
			);
			// Write padding
			if (padding > 0) {
				file.write(
					reinterpret_cast<const char *>(padding_bytes.data()),
					padding
				);
			}
		}

		file.close();
		return true;
	}

	/**
	 * @brief Get image width
	 */
	int width() const {
		return width_;
	}

	/**
	 * @brief Get image height
	 */
	int height() const {
		return height_;
	}
};

// Predefined colors for convenience
namespace BmpColors {
struct Color {
	std::uint8_t r, g, b;
	constexpr Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue)
		: r(red), g(green), b(blue) {}
};

constexpr Color BLACK(0, 0, 0);
constexpr Color WHITE(255, 255, 255);
constexpr Color RED(255, 0, 0);
constexpr Color GREEN(0, 255, 0);
constexpr Color BLUE(0, 0, 255);
constexpr Color YELLOW(255, 255, 0);
constexpr Color CYAN(0, 255, 255);
constexpr Color MAGENTA(255, 0, 255);

// Tile type colors for terrain visualization
constexpr Color LAND(144, 238, 144);   // Light green
constexpr Color MOUNTAIN(139, 69, 19); // Saddle brown
constexpr Color SAND(244, 164, 96);    // Sandy brown
constexpr Color WATER(30, 144, 255);   // Dodger blue
constexpr Color ICE(176, 224, 230);    // Powder blue
constexpr Color DEEPWATER(0, 0, 139);  // Dark blue
constexpr Color OIL(0, 0, 0);          // Black

// Mineral colors
constexpr Color HEMATITE(255, 0, 0);          // Red
constexpr Color TITANOMAGNETITE(128, 0, 128); // Purple
constexpr Color GIBBSITE(255, 255, 0);        // Yellow
constexpr Color COAL(64, 64, 64);             // Dark gray
} // namespace BmpColors

#endif // BMP_H
