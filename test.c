int main(void) {
    int a = 5;
    int b = 3;
    int c = 8;
    int d = 2;
    int e = -1;

    int i = 0;
    int f = 0;
    for (i = 1; i <= 10; i++) {
        f += i;
    }

    // (((5 + ((3 * 8) / 2)) - (-1)) + 55)
    // = (((5 + (24 / 2))) + 1 + 55)
    // = ((5 + 12) + 1 + 55)
    // = (17 + 1 + 55)
    // = 73
    int result = a + b * c / d - e + f;

    if (result == 73) {
        return 0; // Ok
    } else {
        return -1; // Err
    }
}
