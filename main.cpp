#include <iostream>
#include <fstream>
#include <vector>
#include <string>


const int kHeader_size = 14;
const int kInformation_size = 40;


struct bmp_img {
public:
    bmp_img(uint16_t width_i, uint16_t height_i, std::vector<std::vector<uint64_t>> &grid) {
        width = width_i;
        height = height_i;
        matrix = grid;
    }

    void create_img(const std::string &path) {
        const int extra_bytes = ((4 - (width * 3) % 4) % 4);
        unsigned char end_of_line_extra_bytes[] = {0, 0, 0};
        const int file_size = kHeader_size + kInformation_size + width * height * 3 + height * extra_bytes;
        std::ofstream f;
        f.open(path, std::ios::out);
        unsigned char header[kHeader_size] = {0};
        header[0] = 'B';
        header[1] = 'M';
        header[2] = file_size;
        header[3] = file_size >> 8;
        header[4] = file_size >> 16;
        header[5] = file_size >> 24;
        header[10] = kHeader_size + kInformation_size;

        unsigned char information[kInformation_size] = {0};
        information[0] = kInformation_size;
        information[4] = width;
        information[5] = width >> 8;
        information[8] = height;
        information[9] = height >> 8;
        information[14] = 24;

        f.write(reinterpret_cast<char *>(header), kHeader_size);
        f.write(reinterpret_cast<char *>(information), kInformation_size);

        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (matrix[i][j] == 0) {
                    unsigned char pixel[3] = {255, 255, 255};
                    f.write(reinterpret_cast<char *>(pixel), 3);
                } else if (matrix[i][j] == 1) {
                    unsigned char pixel[3] = {0, 255, 0};
                    f.write(reinterpret_cast<char *>(pixel), 3);
                } else if (matrix[i][j] == 2) {
                    unsigned char pixel[3] = {184, 0, 154};
                    f.write(reinterpret_cast<char *>(pixel), 3);
                } else if (matrix[i][j] == 3) {
                    unsigned char pixel[3] = {0, 255, 255};
                    f.write(reinterpret_cast<char *>(pixel), 3);
                } else {
                    unsigned char pixel[] = {0, 0, 0};
                    f.write(reinterpret_cast<char *>(pixel), 3);
                }
            }
            f.write(reinterpret_cast<char *>(end_of_line_extra_bytes), extra_bytes);
        }

        f.close();
    }

private:
    uint16_t width;
    uint16_t height;
    std::vector<std::vector<uint64_t>> matrix;
};


int main(int argc, char *argv[]) {
    uint16_t input_x;                    //input_x, input_y, x_collapse_coordinate, y_collapse_coordinate are used for coordinates of matrix they are <= 2^16
    uint16_t input_y;
    uint16_t x_collapse_coordinate;
    uint16_t y_collapse_coordinate;
    uint64_t input_num;                    //amount of sand grains in each cell
    uint64_t iter = 0;                  //number of iteration
    uint16_t height_par;                //height of image and matrix
    uint16_t width_par;                 //width of image and matrix
    uint64_t max_iter = 1000000000;     //max amount of iterations default = 1000000000
    int freq = 0;                       //interim pictures save frequency
    std::string iter_for_filename;      //string adding to filename of interim pictures
    std::string input_path;             //path of the input file
    std::string output_path;            //path of the output file
    std::string argv_i;                 //string for converting argv[i] from char* in string
    std::string tmp_argv;               //string for converting argv[i+1] in string
    for (int ind = 0; ind < argc; ind++) {
        argv_i = argv[ind];
        if ((argv_i == "-l") || (argv_i == "--length")) {
            tmp_argv = argv[ind + 1];
            height_par = std::stoi(tmp_argv);
        } else if ((argv_i == "-w") || (argv_i == "--width")) {
            tmp_argv = argv[ind + 1];
            width_par = std::stoi(tmp_argv);
        } else if ((argv_i == "-i") || (argv_i == "--input")) {
            input_path = argv[ind + 1];
        } else if ((argv_i == "-o") || (argv_i == "--output")) {
            output_path = argv[ind + 1];
        } else if ((argv_i == "-m") || (argv_i == "--max-iter")) {
            tmp_argv = argv[ind + 1];
            max_iter = std::stoi(tmp_argv);
        } else if ((argv_i == "-f") || (argv_i == "--freq")) {
            tmp_argv = argv[ind + 1];
            freq = std::stoi(tmp_argv);
        }
    }

    std::vector<uint32_t> collapse_vector;            //vector for coordinates with >= 4 sand grains
    std::vector<std::vector<uint64_t>> grid(height_par,
                                            std::vector<uint64_t>(width_par, 0));     //constructor of our main matrix

    std::ifstream fin;
    fin.open(input_path);
    while (true) {
        fin >> input_x;
        if (fin.eof()) break;
        fin >> input_y;
        fin >> input_num;
        grid[input_x][input_y] = input_num;
        if (input_num > 3) {
            collapse_vector.push_back(static_cast<uint32_t>(input_x) * width_par +
                                      input_y);        //we add every coordinate with >= 4 sand grains in vector
        }
    }
    fin.close();

    if (freq != 0) {
        while ((!collapse_vector.empty()) && (iter < max_iter)) {
            x_collapse_coordinate = collapse_vector.back() / width_par;
            y_collapse_coordinate = collapse_vector.back() % width_par;
            if ((x_collapse_coordinate + 1) < height_par) {
                grid[x_collapse_coordinate + 1][y_collapse_coordinate] += 1;
            }
            if ((x_collapse_coordinate - 1) >= 0) {
                grid[x_collapse_coordinate - 1][y_collapse_coordinate] += 1;
            }
            if ((y_collapse_coordinate + 1) < width_par) {
                grid[x_collapse_coordinate][y_collapse_coordinate + 1] += 1;
            }
            if ((y_collapse_coordinate - 1) >= 0) {
                grid[x_collapse_coordinate][y_collapse_coordinate - 1] += 1;
            }
            grid[x_collapse_coordinate][y_collapse_coordinate] -= 4;
            collapse_vector.pop_back();

            if (collapse_vector.empty()) {
                iter++;
                if (iter % freq == 0) {
                    iter_for_filename = std::to_string(iter);
                    bmp_img img(width_par, height_par, grid);
                    img.create_img(output_path + "picture" + iter_for_filename + ".bmp");
                }
                for (uint32_t i = 0; i < height_par * width_par; i++) {
                    if (grid[i / width_par][i % width_par] > 3) collapse_vector.push_back(i);
                }
            }
        }
    } else {
        while ((!collapse_vector.empty()) && (iter < max_iter)) {
            x_collapse_coordinate = collapse_vector.back() / width_par;
            y_collapse_coordinate = collapse_vector.back() % width_par;
            if ((x_collapse_coordinate + 1) < height_par) {
                grid[x_collapse_coordinate + 1][y_collapse_coordinate] +=
                        grid[x_collapse_coordinate][y_collapse_coordinate] / 4;
            }
            if ((x_collapse_coordinate - 1) >= 0) {
                grid[x_collapse_coordinate - 1][y_collapse_coordinate] +=
                        grid[x_collapse_coordinate][y_collapse_coordinate] / 4;
            }
            if ((y_collapse_coordinate + 1) < width_par) {
                grid[x_collapse_coordinate][y_collapse_coordinate + 1] +=
                        grid[x_collapse_coordinate][y_collapse_coordinate] / 4;
            }
            if ((y_collapse_coordinate - 1) >= 0) {
                grid[x_collapse_coordinate][y_collapse_coordinate - 1] +=
                        grid[x_collapse_coordinate][y_collapse_coordinate] / 4;
            }
            grid[x_collapse_coordinate][y_collapse_coordinate] %= 4;
            collapse_vector.pop_back();
            if (collapse_vector.empty()) {
                for (uint32_t i = 0; i < height_par * width_par; i++) {
                    if (grid[i / width_par][i % width_par] > 3) collapse_vector.push_back(i);
                }
            }
        }
    }

    bmp_img final_img(width_par, height_par, grid);
    final_img.create_img(output_path + "final_picture.bmp");
}
