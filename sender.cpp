#include <stdio.h>
#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>

void printHelp() {
    printf("Usage: sender [-t] <filename>\n");
    printf("  -t : text mode (default: hex mode)\n");
    printf("  filename: the file to send\n");
}

BOOL CALLBACK enumWindowsProc(HWND hWnd, LPARAM lParam) {
    // 获取窗口标题的长度
    int length = ::GetWindowTextLengthA(hWnd);
    if( 0 == length ) return TRUE;

    // 获取窗口标题的文字
    char* buffer = new char[length + 1];
    memset(buffer, 0, (length + 1) * sizeof(char));
    ::GetWindowTextA(hWnd, buffer, length + 1);
    std::string title = buffer;
    delete[] buffer;

    // 根据窗口标题来判定，如果是目标窗口，则将其调到最前面（输入焦点）
    std::string prefix = "\xb6\xc0\xcf\xed\xd7\xc0\xc3\xe6 - "; // 字符串里面是GB2312编码的中文“独享桌面”
    if (title.rfind(prefix, 0) == 0) {
        ::SetForegroundWindow(hWnd);
    }

    return TRUE;
}

// ASCII 字符与键盘 scan code 的对应表
WORD sc_map[] = {
    0x0039, // 32  SPACE
    0x0102, // 33  !
    0x0128, // 34  "
    0x0104, // 35  #
    0x0105, // 36  $
    0x0106, // 37  %
    0x0108, // 38  &
    0x0028, // 39  '
    0x010a, // 40  (
    0x010b, // 41  )
    0x0109, // 42  *
    0x010d, // 43  +
    0x0033, // 44  ,
    0x000c, // 45  -
    0x0034, // 46  .
    0x0035, // 47  /
    0x000b, // 48  0
    0x0002, // 49  1
    0x0003, // 50  2
    0x0004, // 51  3
    0x0005, // 52  4
    0x0006, // 53  5
    0x0007, // 54  6
    0x0008, // 55  7
    0x0009, // 56  8
    0x000a, // 57  9
    0x0127, // 58  :
    0x0027, // 59  ;
    0x0133, // 60  <
    0x000d, // 61  =
    0x0134, // 62  >
    0x0135, // 63  ?
    0x0103, // 64  @
    0x011e, // 65  A
    0x0130, // 66  B
    0x012e, // 67  C
    0x0120, // 68  D
    0x0112, // 69  E
    0x0121, // 70  F
    0x0122, // 71  G
    0x0123, // 72  H
    0x0117, // 73  I
    0x0124, // 74  J
    0x0125, // 75  K
    0x0126, // 76  L
    0x0132, // 77  M
    0x0131, // 78  N
    0x0118, // 79  O
    0x0119, // 80  P
    0x0110, // 81  Q
    0x0113, // 82  R
    0x011f, // 83  S
    0x0114, // 84  T
    0x0116, // 85  U
    0x012f, // 86  V
    0x0111, // 87  W
    0x012d, // 88  X
    0x0115, // 89  Y
    0x012c, // 90  Z
    0x001a, // 91  [
    0x002b, // 92  \ ...
    0x001b, // 93  ]
    0x0107, // 94  ^
    0x010c, // 95  _
    0x0029, // 96  `
    0x001e, // 97  a
    0x0030, // 98  b
    0x002e, // 99  c
    0x0020, //100  d
    0x0012, //101  e
    0x0021, //102  f
    0x0022, //103  g
    0x0023, //104  h
    0x0017, //105  i
    0x0024, //106  j
    0x0025, //107  k
    0x0026, //108  l
    0x0032, //109  m
    0x0031, //110  n
    0x0018, //111  o
    0x0019, //112  p
    0x0010, //113  q
    0x0013, //114  r
    0x001f, //115  s
    0x0014, //116  t
    0x0016, //117  u
    0x002f, //118  v
    0x0011, //119  w
    0x002d, //120  x
    0x0015, //121  y
    0x002c, //122  z
    0x011a, //123  {
    0x012b, //124  |
    0x011b, //125  }
    0x0129, //126  ~
};

void typeInChar(char c) {
    // 获取字符对应的 scan code
    BOOL isShift = FALSE;
    BYTE sc = 0;
    if (c == '\t') {
        sc = 0x0f;
    } else if (c == '\r') {
        return;
    } else if (c == '\n') {
        sc = 0x1c;
    } else if (c >= ' ' && c <= '~') {
        int pos = c - ' ';
        // printf("%d -> %04x\n", pos, sc_map[pos]);
        sc = sc_map[pos] & 0xFF;
        isShift = sc_map[pos] & 0xFF00 ? TRUE : FALSE;
    } else {
      return;
    }
    // printf("%c[%02X]: %02X %c\n", c, c, sc, isShift?'*':' ');

    // 模拟键盘的按键动作
    if (isShift) {
        // 按下 SHIFT 键
        keybd_event(0, 0x2A, 0, 0);
    }
    // 按下字符键
    keybd_event(0, sc, 0, 0);
    // 松开字符键
    keybd_event(0, sc, KEYEVENTF_KEYUP, 0);
    if (isShift) {
        // 松开 SHIFT 键
        keybd_event(0, 0x2A, KEYEVENTF_KEYUP, 0);
    }
}

void typeInString(std::string str, BOOL bTextMode) {
    int cnt = 0;
    int sofar = 0;
    int total = str.length();
    for (std::string::iterator it = str.begin(); it != str.end(); ++it) {
        if (cnt == 0) {
            printf("\r%4.1f%%: ", (float)sofar * 100 / total);
        }
        sofar ++;
        if (bTextMode) {
            // 文本模式，直接模拟键盘输入该字符
            typeInChar(*it);
        } else {
            // hex 模式，把字符转换成 hex 格式并模拟键盘输入
            char hex[3];
            sprintf(hex, "%02x", (BYTE)(*it));
            typeInChar(hex[0]);
            typeInChar(hex[1]);

            // 适当插入换行符，便于检查是否出现丢字
            printf(hex);
            if (++cnt >= 32) {
                typeInChar('\n');
                printf("\n");
                cnt = 0;
            }
        }
        // 略做停顿，避免消息大量堆积
        ::Sleep(1);
    }
}

int main(int argc, char *argv[]) {
    // 解析命令行参数
    std::string filename;
    BOOL bTextMode = FALSE;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "-t") {
            bTextMode = TRUE;
        } else {
            if (filename.empty()) {
                filename = arg;
            } else {
                printHelp();
                return EXIT_SUCCESS;
            }
        }
    }
    if (filename.empty()) {
        printHelp();
        return EXIT_SUCCESS;
    }

    // 遍历所有主窗口，找到 VDI 窗口并将其调到最前面
    ::EnumWindows(enumWindowsProc, 0L);

    // 读入文件内容并模拟敲键盘进行发送
    std::ifstream file(filename, std::ios::binary);
    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    // printf("content: %s\n", str.c_str());
    typeInString(str, bTextMode);

    return EXIT_SUCCESS;
}

// g++ -o sender.exe sender.cpp
// sender test.txt -t
