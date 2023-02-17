# cc_inter
[GB27001] ソフトウェアサイエンス特別講義Aの最終課題です．  
昔，セルフホストできるCコンパイラを書いたことがあったので授業内容を踏まえて
インタプリタに改造してみました．

以下の方法でコードを走らせ動作を確認することが出来ます．
```sh
$ git clone https://github.com/Alignof/cc_inter.git
$ cd cc_inter
$ make
$ ./cc_inter test.c
```

test.cは以下の内容です．
```c
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
```

このコードは課題要件にある以下の条件を満たしています．
- 四則演算
- if, for(whileもできます)
- ローカル変数，初期化式
- 連接
- 関数定義，関数呼び出し，引数
- コメント
- return

入力にCのソースコードを受け取りmain関数の返り値を実行結果として出力します．
ソースコードは計算を行い最終的にmain関数から83が返るので，
標準出力に83が表示されれば成功です．

