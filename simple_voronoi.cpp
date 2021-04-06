#include <iostream>
#include <math.h>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#define cimg_display 0
#include "CImg.h"
using namespace std;

// #define DISTANCE_THREDSHOLD 30
#define MAX_NUMBER_OF_NEIGHBORS 8
#define MAX_FLOOD_FILL_RECURSION_DEPTH 5000

int DISTANCE_THREDSHOLD = 30;
typedef unordered_map<uint32_t, unordered_set<uint32_t>> hash_uint_uset;
typedef pair<int,int> pii;
const int NEIGHBOR_DIRECTIONS[] = {0,-1,-1,1,1,-1,0,1,0};

const int COLOR_STEP_COUNT = 6;
const int COLOR_STEPS[COLOR_STEP_COUNT] = {0, 0x5f, 0x87, 0xaf, 0xd7, 0xff};

const int GRAYSCALE_STEP_COUNT = 24;
const int GRAYSCALE_STEPS[GRAYSCALE_STEP_COUNT] = {
  0x08, 0x12, 0x1c, 0x26, 0x30, 0x3a, 0x44, 0x4e, 0x58, 0x62, 0x6c, 0x76,
  0x80, 0x8a, 0x94, 0x9e, 0xa8, 0xb2, 0xbc, 0xc6, 0xd0, 0xda, 0xe4, 0xee};



const uint32_t BITMAPS[] = {
  0x00000000, 0x00a0,

  // Block graphics
  // 0xffff0000, 0x2580,  // upper 1/2; redundant with inverse lower 1/2

  0x0000000f, 0x2581,  // lower 1/8
  0x000000ff, 0x2582,  // lower 1/4
  0x00000fff, 0x2583,
  0x0000ffff, 0x2584,  // lower 1/2
  0x000fffff, 0x2585,
  0x00ffffff, 0x2586,  // lower 3/4
  0x0fffffff, 0x2587,
//0xffffffff, 0x2588,  // full; redundant with inverse space

  0xeeeeeeee, 0x258a,  // left 3/4
  0xcccccccc, 0x258c,  // left 1/2
  0x88888888, 0x258e,  // left 1/4

  0x0000cccc, 0x2596,  // quadrant lower left
  0x00003333, 0x2597,  // quadrant lower right
  0xcccc0000, 0x2598,  // quadrant upper left
// 0xccccffff, 0x2599,  // 3/4 redundant with inverse 1/4
  0xcccc3333, 0x259a,  // diagonal 1/2
// 0xffffcccc, 0x259b,  // 3/4 redundant
// 0xffff3333, 0x259c,  // 3/4 redundant
  0x33330000, 0x259d,  // quadrant upper right
// 0x3333cccc, 0x259e,  // 3/4 redundant
// 0x3333ffff, 0x259f,  // 3/4 redundant

// Line drawing subset: no double lines, no complex light lines

  0x000ff000, 0x2501,  // Heavy horizontal
  0x66666666, 0x2503,  // Heavy vertical

  0x00077666, 0x250f,  // Heavy down and right
  0x000ee666, 0x2513,  // Heavy down and left
  0x66677000, 0x2517,  // Heavy up and right
  0x666ee000, 0x251b,  // Heavy up and left

  0x66677666, 0x2523,  // Heavy vertical and right
  0x666ee666, 0x252b,  // Heavy vertical and left
  0x000ff666, 0x2533,  // Heavy down and horizontal
  0x666ff000, 0x253b,  // Heavy up and horizontal
  0x666ff666, 0x254b,  // Heavy cross

  0x000cc000, 0x2578,  // Bold horizontal left
  0x00066000, 0x2579,  // Bold horizontal up
  0x00033000, 0x257a,  // Bold horizontal right
  0x00066000, 0x257b,  // Bold horizontal down

  0x06600660, 0x254f,  // Heavy double dash vertical

  0x000f0000, 0x2500,  // Light horizontal
  0x0000f000, 0x2500,  //
  0x44444444, 0x2502,  // Light vertical
  0x22222222, 0x2502,

  0x000e0000, 0x2574,  // light left
  0x0000e000, 0x2574,  // light left
  0x44440000, 0x2575,  // light up
  0x22220000, 0x2575,  // light up
  0x00030000, 0x2576,  // light right
  0x00003000, 0x2576,  // light right
  0x00004444, 0x2577,  // light down
  0x00002222, 0x2577,  // light down

// Misc technical

  0x44444444, 0x23a2,  // [ extension
  0x22222222, 0x23a5,  // ] extension

  0x0f000000, 0x23ba,  // Horizontal scanline 1
  0x00f00000, 0x23bb,  // Horizontal scanline 3
  0x00000f00, 0x23bc,  // Horizontal scanline 7
  0x000000f0, 0x23bd,  // Horizontal scanline 9

// Geometrical shapes. Tricky because some of them are too wide.

// 0x00ffff00, 0x25fe,  // Black medium small square
  0x00066000, 0x25aa,  // Black small square

0x11224488, 0x2571,  // diagonals
0x88442211, 0x2572,
0x99666699, 0x2573,
0x000137f0, 0x25e2,  // Triangles
0x0008cef0, 0x25e3,
0x000fec80, 0x25e4,
0x000f7310, 0x25e5,

  0, 0,  // End marker for "regular" characters
};

struct the_size {
  unsigned int width;
  unsigned int height;

  the_size(unsigned int in_width, unsigned int in_height) :
        width(in_width), height(in_height) {
  }
  the_size(cimg_library::CImg<unsigned int> img) :
        width(img.width()), height(img.height()) {
  }
  
  the_size scaled(double scale) {
    return the_size(width*scale, height*scale);
  }
  the_size fitted_within(the_size container) {
    double scale = std::min(container.width / (double) width, container.height / (double) height);
    return scaled(scale);
  }
};
std::ostream& operator<<(std::ostream& stream, const the_size& sz) {
  stream << sz.width << "x" << sz.height;
  return stream;
}


struct point{
    int x, y;
    point():x(0), y(0){}
    point(int x, int y):x(x), y(y){}   
};

struct center{
    float x,y;
    float r, g, b;
};


void print_utf8(int codepoint) {
  
  if (codepoint < 128) {

    cout << (char) codepoint;

  } else if (codepoint < 0x7ff) {
    cout << (char) (0xc0 | (codepoint >> 6));
    cout << (char) (0x80 | (codepoint & 0x3f));


  } else if (codepoint < 0xffff) {

    cout << (char) (0xe0 | (codepoint >> 12));
    cout << (char) (0x80 | ((codepoint >> 6) & 0x3f));
    cout << (char) (0x80 | (codepoint & 0x3f));

  } else if (codepoint < 0x10ffff) {

    cout << (char) (0xf0 | (codepoint >> 18));
    cout << (char) (0x80 | ((codepoint >> 12) & 0x3f));
    cout << (char) (0x80 | ((codepoint >> 6) & 0x3f));
    cout << (char) (0x80 | (codepoint & 0x3f));

  } else {
    cerr << "ERROR";
  }
}

int best_index(int value, const int data[], int count) {
  int best_diff = abs(data[0] - value), diff;
  int result = 0;
  for (int i = 1; i < count; i++) {
    diff = abs(data[i] - value);
    if (diff < best_diff) {
      result = i;
      best_diff = diff;
    }
  }
  return result;
}

float clamp_byte(float value) {
  return value < 0 ? 0 : (value > 255 ? 255 : value);
}

float sqr(float num){
  return num * num;
}

int rgb_256_rgb_6(float r, float g, float b){

  r = clamp_byte(r);
  g = clamp_byte(g);
  b = clamp_byte(b);

  int r_6_index = best_index(r, COLOR_STEPS, COLOR_STEP_COUNT);
  int g_6_index = best_index(g, COLOR_STEPS, COLOR_STEP_COUNT);
  int b_6_index = best_index(b, COLOR_STEPS, COLOR_STEP_COUNT);

  int r_256_from_r_6 = COLOR_STEPS[ r_6_index ];
  int g_256_from_g_6 = COLOR_STEPS[ g_6_index ];
  int b_256_from_b_6 = COLOR_STEPS[ b_6_index ];

  float gray = r * 0.2989f + g * 0.5870f + b * 0.1140f;
  int gray_24_index = best_index(gray, GRAYSCALE_STEPS, GRAYSCALE_STEP_COUNT);
  int gray_256_from_gray_24 = GRAYSCALE_STEPS[ gray_24_index ];

  int color_index;
  if (0.3 * sqr( r_256_from_r_6 - r) + 0.59 * sqr(g_256_from_g_6-g) + 0.11 * sqr(b_256_from_b_6-b) <
      0.3 * sqr(gray_24_index-r) + 0.59 * sqr(gray_24_index-g) + 0.11 * sqr(gray_24_index-b)) {
    color_index = 16 + 36 * r_6_index + 6 * g_6_index + b_6_index;
  } else {
    color_index = 232 + gray_24_index;  // 1..24 -> 232..255
  }

  return color_index;

}

cimg_library::CImg<unsigned char> load_rgb_CImg(const char * const filename) {
  cimg_library::CImg<unsigned char> image(filename);
  if(image.spectrum() == 1) {
    // Greyscale. Just copy greyscale data to all channels
    cimg_library::CImg<unsigned char> rgb_image(image.width(), image.height(), image.depth(), 3);
    for(uint32_t chn = 0; chn < 3; chn++) {
      rgb_image.draw_image(0, 0, 0,chn, image);
    }
    return rgb_image;
  }

  return image;
}

int calc_distance(int y, int x, int ny, int nx, 
                    const cimg_library::CImg<uint32_t>& image){
    int dist = 0, val;
    for(int i = 0; i < 3; ++i){
      val = static_cast<int>(image(x, y, 0, i)) - static_cast<int>(image(nx, ny, 0, i));
      if(val < 0) val = -val;
      dist += val;
    }
    return dist;
}

void flood_fill(int y, int x, uint32_t color, 
                vector<vector<uint32_t>>& colors, 
                const cimg_library::CImg<uint32_t>& img,
                int recursion_depth = 0){
    if(recursion_depth == MAX_FLOOD_FILL_RECURSION_DEPTH)
      return;
    colors[y][x] = color;
    for(int nei = 0; nei < MAX_NUMBER_OF_NEIGHBORS; ++nei ){
        int ny = y + NEIGHBOR_DIRECTIONS[nei];
        int nx = x + NEIGHBOR_DIRECTIONS[nei+1];
        if(ny < 0 || nx < 0 || ny == colors.size() || nx == colors[0].size())
            continue;
        
        if(colors[ny][nx] != 0 || calc_distance(y,x,ny,nx, img) > DISTANCE_THREDSHOLD)
            continue;
        flood_fill(ny, nx, color, colors, img, recursion_depth + 1);
    }
}

int get_regions(vector<vector<uint32_t>>& regions,
                const cimg_library::CImg<uint32_t>& img){
    uint32_t region_index = 1;
    
    int img_width = img.width(), img_height = img.height();
    for(int y = 0; y < img_height; ++y){
        for(int x = 0; x < img_width; ++x){
            if(regions[y][x] == 0){
              try{
                flood_fill(y,x, region_index, regions, img);
              }catch(std::exception& e){
                std::cerr << "exception caught: " << e.what() << '\n';
              }
                
                ++region_index;
            }
        }
    }
    for(int y = 0; y < img_height; ++y){
        for(int x = 0; x < img_width; ++x){
            --regions[y][x];
        }
    }
    return --region_index;

}


/*

Compute the center coordinate for each region
and average red, green, blue value in the region
as the center's attributes

*/

void compute_center(const vector<point>& pts, center& the_center,
                    const cimg_library::CImg<uint32_t>& img){

    float x = 0, y = 0;
    float r = 0, g = 0, b = 0;
    int n = pts.size();
    for(int i = 0; i < pts.size(); ++i){

        x += (float) pts[i].x/n;
        y += (float) pts[i].y/n;
        int xx = pts[i].x, yy = pts[i].y;

        r += (float) img(xx, yy, 0, 0)/n;
        g += (float) img(xx, yy, 0, 1)/n;
        b += (float) img(xx, yy, 0, 2)/n;
        
    }

    the_center.x = x; //floor(x);
    the_center.y = y; //floor(y);
    the_center.r = r;
    the_center.g = g;
    the_center.b = b;
}

double sqr_distance(double x1, double y1, double x2, double y2){
    return (x1-x2) * (x1-x2) + (y1-y2) * (y1-y2);
}

/*

Compute the center for each region 
alongside with average x,y coordinate 
and average r,g,b values for the region

*/

void get_centers(uint32_t number_of_regions, 
                const vector<vector<uint32_t>>& regions,
                const cimg_library::CImg<uint32_t>& img,
                vector<center>& centers){

    int img_width = img.width(), img_height = img.height();
    vector<vector<point>> region_members(number_of_regions, vector<point>());
    for(int y = 0; y < img_height; ++y){
        for(int x = 0; x < img_width; ++x){
            region_members[regions[y][x]].push_back({x,y});
            
        }
    }
    
    for(int i = 0; i < number_of_regions; ++i){
        center the_center;
        compute_center(region_members[i], the_center, img);
        centers.push_back(the_center);
    }
}

/*
Compute neighbor region for each pixel
*/

void get_neighbors_per_region(const vector<vector<uint32_t>>& regions,
                            hash_uint_uset& neighbors_per_region){

    int img_width = regions[0].size(), img_height = regions.size();
    uint32_t pixel_region, nei_pixel_region;
    for(int y = 0; y < img_height; ++y){
        for(int x = 0; x < img_width; ++x){

            pixel_region = regions[y][x];
            neighbors_per_region[ pixel_region ].insert(pixel_region);

            for(int nei = 0; nei < MAX_NUMBER_OF_NEIGHBORS; ++nei){

              int ny = y + NEIGHBOR_DIRECTIONS[nei];
              int nx = x + NEIGHBOR_DIRECTIONS[nei+1];

              if(ny < 0 || nx < 0 || ny == img_height || nx == img_width)
                  continue;

              nei_pixel_region = regions[ny][nx];
              neighbors_per_region[ pixel_region ].insert(nei_pixel_region);

            }
        }
    }

}

/*

Relabelling each pixel to an appropriate vonoroi cell
of the centroid closest to it
For each pixel assign color of vonoroi cell it belongs to
by looping through neighboring sites 
E[deg(vertex)] = 6 so we expect a constant number of neighbors
for each pixel

*/

void relabel_pixels(const hash_uint_uset& neighbors_per_region, 
                    const vector<center>& centers,
                    const vector<vector<uint32_t>>& regions,
                    vector<vector<uint32_t>>& image){

    int img_width = image[0].size(), img_height = image.size(); 
    uint32_t pixel_region, centroid_region;
    double smallest_sqr_distance, distance;
    for(int y = 0; y < img_height; ++y){
      for(int x = 0; x < img_width; ++x){

          pixel_region = regions[y][x];
          smallest_sqr_distance = img_height * img_height + img_width * img_width;
          centroid_region = pixel_region;

          for(const uint32_t& nei_region : neighbors_per_region.find(pixel_region)->second){

            distance = sqr_distance(x,y, centers[ nei_region ].x, centers[ nei_region ].y);

            if(distance < smallest_sqr_distance){
                smallest_sqr_distance = distance;
                centroid_region = nei_region;
            }

          }

          image[y][x] = centroid_region;
      }
    }
}

  uint32_t get_block_bitmask(int y0, int x0, const vector<center>& centers,
                              const vector<vector<uint32_t>>& image,
                              uint32_t& most_freq_region,
                              uint32_t& second_freq_region){

      unordered_map<int, int> freq;
      float r,g,b;
      uint32_t center_index;
      int color;
      
      for(int y = 0; y < 8; ++y){
          for(int x = 0; x < 4; ++x){

            center_index = image[y0 + y][x0 + x];
            freq[ center_index ]++;

          }
      }

      vector<pair<int,int>> ordered_regions(freq.begin(), freq.end());
      sort(ordered_regions.begin(), ordered_regions.end(), 
          [&](const pii& a, const pii& b){
            return a.second > b.second;
          });

      most_freq_region = ordered_regions[0].first;
      second_freq_region = most_freq_region;
      if(ordered_regions.size() > 1)
          second_freq_region = ordered_regions[1].first;

      float most_freq_region_x = centers[most_freq_region].x;
      float most_freq_region_y = centers[most_freq_region].y;
      float second_freq_region_x = centers[second_freq_region].x;
      float second_freq_region_y = centers[second_freq_region].y;

      /*
          bit mask of most frequent 
          and second frequent color pattern
      */

      uint32_t block_bitmask = 0;
      float distance_to_region1, distance_to_region2;
      for(int y = 0; y < 8; ++y){
          for(int x = 0; x < 4; ++x){
          block_bitmask <<= 1;
          center_index = image[y0 + y][x0 + x];
          if(center_index != most_freq_region && center_index != second_freq_region){

              distance_to_region1 = sqr_distance(y,x, most_freq_region_x, most_freq_region_y);
              distance_to_region2 = sqr_distance(y,x, second_freq_region_x, second_freq_region_y);

              if( distance_to_region1 < distance_to_region2 ){
                  center_index = most_freq_region;
              }
              else center_index = second_freq_region;
              
          }

          block_bitmask |= center_index == second_freq_region;

          }
      }
      return block_bitmask;

  }

  int get_best_codepoint(uint32_t block_bitmask, bool& inverted){
     
      int end_marker = 0;
      uint32_t pattern;
      uint32_t codepoint = 0x2588;//full block, 1 color
      int best_diff = 32, diff;
      for(int i = 0; BITMAPS[i+1] != end_marker; i+=2 ){
          pattern = BITMAPS[i];
          for(int j = 0; j < 2; ++j){
            diff = __builtin_popcount(pattern ^ block_bitmask);
     
            if( diff < best_diff){
              best_diff = diff;
              codepoint = BITMAPS[i+1];
              inverted = pattern != BITMAPS[i];
            }
            pattern = (~pattern) & 0xffffffff;
          }
          
      }
      return codepoint;
  }

  /*

  Each block graphics has size 8x4.
  So for each 8x4 region, compute the best block graphics by 
  choosing 2 most frequent colors and select the block graphics
  that best captures the locations of these 2 colors in the region.
  Print out the block

  */

  void print_block(int y0, int x0, const vector<center>& centers,
                  const vector<vector<uint32_t>>& image){
      
      uint32_t most_freq_region, second_freq_region;
      uint32_t block_bitmask = get_block_bitmask(y0, x0, centers, image,
                              most_freq_region,
                              second_freq_region);
      bool inverted;
      int codepoint = get_best_codepoint(block_bitmask, inverted);
      if(inverted) swap(second_freq_region, most_freq_region);

      float r = centers[ second_freq_region ].r;
      float g = centers[ second_freq_region ].g;
      float b = centers[ second_freq_region ].b;
      
      int color = rgb_256_rgb_6(r,g,b);

      cout << "\u001B[38;5;" << color << "m";

      r = centers[ most_freq_region ].r;
      g = centers[ most_freq_region ].g;
      b = centers[ most_freq_region ].b;
      color = rgb_256_rgb_6(r,g,b);

      cout << "\x1b[48;5;" << color << "m";
      print_utf8(codepoint); 
      cout << "\x1b[0m";

  }

  void print_vonoroi(const vector<center>& centers,
                  const vector<vector<uint32_t>>& image){
      
      int img_width = image[0].size(), img_height = image.size();
      for(int y = 0; y <= img_height-8; y+=8){
          for(int x = 0; x <= img_width-4; x += 4){
            print_block(y,x, centers, image);
          }
          cout << endl;
      }
      cout << endl;
  }

  void calculate_voronoi(const cimg_library::CImg<uint32_t>& img){

      int img_width = img.width(), img_height = img.height();

      if(img_height == 0){
        cout << endl;
        return;
      }

      vector<vector<uint32_t>> regions(img_height, vector<uint32_t>(img_width, 0));
      int number_of_regions = get_regions(regions, img);

      vector<center> centers;
      get_centers(number_of_regions, regions, img, centers);

      hash_uint_uset neighbors_per_region;
      get_neighbors_per_region(regions, neighbors_per_region);
      
      vector<vector<uint32_t>> image(img_height, vector<uint32_t>(img_width));
      relabel_pixels(neighbors_per_region, centers, regions, image);

      print_vonoroi(centers, image);
  }

//https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c

inline bool file_exists (char* file_name) {
  struct stat file_stat;   
  return (stat (file_name, &file_stat) == 0); 
}


void print_usage(){
  cout << "./simple_vonoroi -w [width] -h [height] -d [distance thredshold for flood_fill ] file_name" << endl;
}

int main(int argc, char* argv[]){

    int maxWidth = 80;
    int maxHeight = 24;
    bool sizeDetectionSuccessful = true;
    struct winsize w;
    int ioStatus = ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    // If redirecting STDOUT to one file ( col or row == 0, or the previous ioctl call's failed )
    if (ioStatus != 0 || (w.ws_col | w.ws_row) == 0) {
        ioStatus = ioctl(STDIN_FILENO, TIOCGWINSZ, &w);
        if (ioStatus != 0 || (w.ws_col | w.ws_row) == 0) {
            std::cerr << "Warning: failed to determine most reasonable size, defaulting to 80x24" << std::endl;
            sizeDetectionSuccessful = false;
        }
    }
    if (sizeDetectionSuccessful)
    {
        maxWidth = w.ws_col * 4;
        maxHeight = w.ws_row * 8;
    }


    char* file_name = NULL;
    for(int i = 1; i < argc; ++i){
      if(strcmp(argv[i], "-w") == 0){
        if(i+1 == argc){
          print_usage();
        }
        else maxWidth = 4 * stoi(argv[++i]);

      }
      else if(strcmp(argv[i], "-h") == 0){
        if(i+1 == argc){
          print_usage();
        }
        else maxHeight = 8 * stoi(argv[++i]);

      }
      else if(strcmp(argv[i], "-d") == 0){
         if(i+1 == argc){
          print_usage();
        }
        else DISTANCE_THREDSHOLD = stoi(argv[++i]);
      }
      else if(strcmp(argv[i], "-help") == 0){
        print_usage();
        exit(0);
      }
      else {
        file_name = argv[i];
        // if(!file_exists(file_name)){
        //   cerr << "File not found\n";
        //   print_usage();
        //   exit(1);
        // }
      }
    }

    try {
        cimg_library::CImg<unsigned char> image = load_rgb_CImg(file_name);

        if (image.width() > maxWidth || image.height() > maxHeight) {
          the_size new_size = the_size(image).fitted_within(the_size(maxWidth,maxHeight));
          image.resize(new_size.width, new_size.height, -100, -100, 5);
        }
       
        calculate_voronoi(image);
        
    } catch(const cimg_library::CImgIOException & e) {
      std::cerr << "File format is not recognized for '" << file_name << "'" << std::endl;
      exit(1);
    }
    catch(std::exception& e){
      std::cerr << "exception caught: " << e.what() << '\n';
    }
}