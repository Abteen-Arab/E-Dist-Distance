#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

// Constants for the PPM format
const int MAX_COLOR = 255;
const char COMMENT_CHAR = '#';

// Constants for the color histogram
const int NUM_BINS = 8;
const int BIN_SIZE = MAX_COLOR / NUM_BINS;

// A simple struct to hold an RGB color
struct RGBColor {
  int r, g, b;
};

// A simple struct to hold the position and size of a bounding box
struct BoundingBox {
  int x, y, width, height;
};

// A simple struct to hold a training example
struct TrainingExample {
  BoundingBox bbox;
  std::vector<int> color_histogram;
};

// Compute the color histogram of an image
std::vector<int> computeColorHistogram(const std::vector<RGBColor>& pixels) {
    std::vector<int> histogram(NUM_BINS * NUM_BINS * NUM_BINS, 0);
    for (const auto& pixel : pixels) {
        int r_bin = pixel.r / BIN_SIZE;
        int g_bin = pixel.g / BIN_SIZE;
        int b_bin = pixel.b / BIN_SIZE;
        int bin_index = r_bin * NUM_BINS * NUM_BINS + g_bin * NUM_BINS + b_bin;
        histogram[bin_index]++;
    }
    return histogram;
}

// Load an image from a PPM file
std::vector<RGBColor> loadImage(const std::string& filename) {
    std::ifstream ifs(filename);
    if (!ifs) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(1);
    }
    std::string magic;
    ifs >> magic;
    if (magic != "P3") {
        std::cerr << "Invalid magic number: " << magic << std::endl;
        exit(1);
    }
    int width, height, max_color;
    ifs >> width >> height >> max_color;
    if (max_color != MAX_COLOR) {
        std::cerr << "Invalid max color value: " << max_color << std::endl;
        exit(1);
    }
    char comment_char = ifs.get();
    while (comment_char == COMMENT_CHAR) {
        ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        comment_char = ifs.get();
    }
    ifs.unget();
    std::vector<RGBColor> pixels;
    for (int i = 0; i < width * height; i++) {
        RGBColor pixel;
        ifs >> pixel.r >> pixel.g >> pixel.b;
        pixels.push_back(pixel);
    }
    return pixels;
}

// Compute the Euclidean distance between two histograms
double histogramDistance(const std::vector<int>& histogram1, const std::vector<int>& histogram2) {
    double distance = 0.0;
    for (int i = 0; i < histogram1.size(); i++) {
        distance += std::pow(histogram1[i] - histogram2[i], 2);
    }
    return std::sqrt(distance);
}

// Find the nearest neighbor to a query histogram in a set of training examples
BoundingBox nearestNeighbor(const std::vector<int>& query_histogram, const std::vector<TrainingExample>& training_examples) {
    BoundingBox best_bbox;
    double best_distance = std::numeric_limits<double>::max();
    for (const auto& example: training_examples) {
        double distance = histogramDistance(query_histogram, example.color_histogram);
        if (distance < best_distance) {
            best_distance = distance;
            best_bbox = example.bbox;
            
        }
        
    }
    return best_bbox;
    
}

       // Draw a bounding box on an image
void drawBoundingBox(std::vector<RGBColor>& pixels, const BoundingBox& bbox) {
    for (int y = bbox.y; y < bbox.y + bbox.height; y++) {
        for (int x = bbox.x; x < bbox.x + bbox.width; x++) {
            int index = y * bbox.width + x;
            pixels[index].r = MAX_COLOR;
            pixels[index].g = 0;
            pixels[index].b = 0;
            
        }
    }
}
// Save an image to a PPM file

void saveImage(const std::vector<RGBColor>& pixels, const std::string& filename, int width, int height) {
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        exit(1);
        
    }
    ofs << "P3" << std::endl;
    ofs << width << " " << height << std::endl;
    ofs << MAX_COLOR << std::endl;
    for (const auto& pixel : pixels) {
        ofs << pixel.r << " " << pixel.g << " " << pixel.b << std::endl;
    }
}

int main() {
    // Load the training example
    TrainingExample example;
    example.bbox.x = 60;
    example.bbox.y = 40;
    example.bbox.width = 100;
    example.bbox.height = 80;
    std::vector<RGBColor> example_pixels = loadImage("example.ppm");
    example.color_histogram = computeColorHistogram(example_pixels);
    
    // Load the test image
    std::vector<RGBColor> test_pixels = loadImage("test.ppm");
    
    // Compute the color histogram of the test image
    std::vector<int> test_histogram = computeColorHistogram(test_pixels);
    
    // Find the nearest neighbor to the test image
    BoundingBox bbox = nearestNeighbor(test_histogram, {example});
    
    // Draw the bounding box on the test image
    drawBoundingBox(test_pixels, bbox);
    
    // Save the result to a file
    saveImage(test_pixels, "result.ppm", bbox.width, bbox.height);
    
    return 0;
}
