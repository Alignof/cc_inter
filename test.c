int add(int x, int y) {
    return x + y;
}

int main(void) {
    int a = 5;
    int b = 3;
    int c = 8;
    int d = 2;
    int e = -1;
    int f = 4;
    int g = 6;

    int i = 0;
    int h = 0;
    for (i = 1; i <= 10; i++) {
        h += i;
    }

    // (((5 + ((3 * 8) / 2)) - (-1)) + 10 + 55)
    // = (((5 + (24 / 2))) + 1 + 10 + 55)
    // = ((5 + 12) + 1 + 10 + 55)
    // = (17 + 1 + 10 + 55)
    // = 83
    int result = a + b * c / d - e + add(f, g) + h;

    if (result == 83) {
        return result; // Ok == 83
    } else {
        return -1; // Err
    }
}
