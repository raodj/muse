#include <iostream>
#include <string>

double scale(double value, double mult, double& pow10) {
    pow10 = 1;
    while ((value < 10) || (value > 100)) {
        value *= mult;
        pow10 /= mult;
    }
    return long(value);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Specify tick interval to round up.\n";
        return 1;
    }
    double spacing = std::stod(argv[1]);
    double pow10   = 1;
    // Get spacing to a 2 digit value.
    if (spacing < 10) {
        spacing = scale(spacing, 10, pow10);
    } else if (spacing > 100) {
        spacing = scale(spacing, 0.1, pow10);
    }
    // Now round spacing to next highest multiple of 25
    spacing = int((spacing / 25) + 1) * 25;
    spacing *= pow10;
    std::cout << spacing << std::endl;
    return 0;
}
