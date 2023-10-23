
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <Windows.h>

void doubleClick(int x, int y) {
    INPUT input[4] = { 0 };

    // Set the mouse position
    input[0].type = INPUT_MOUSE;
    input[0].mi.mouseData = 0;
    input[0].mi.dx = (x * 65536) / GetSystemMetrics(SM_CXSCREEN);
    input[0].mi.dy = (y * 65536) / GetSystemMetrics(SM_CYSCREEN);
    input[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

    // First click
    input[1] = input[0];
    input[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;

    // Second click
    input[2] = input[0];
    input[2].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input[3] = input[0];
    input[3].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    // Send input events
    if (SendInput(4, input, sizeof(INPUT)) != 4) {
        std::cerr << "Error sending mouse input." << std::endl;
    }
}

cv::Mat captureScreen() {
    HDC hScreen = GetDC(NULL);
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HDC hMemoryDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, screenWidth, screenHeight);
    SelectObject(hMemoryDC, hBitmap);

    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreen, 0, 0, SRCCOPY);

    BITMAPINFOHEADER bmi = { 0 };
    bmi.biSize = sizeof(BITMAPINFOHEADER);
    bmi.biWidth = screenWidth;
    bmi.biHeight = -screenHeight;  // Negative to indicate top-down image
    bmi.biPlanes = 1;
    bmi.biBitCount = 24;
    bmi.biCompression = BI_RGB;

    cv::Mat screenMat(screenHeight, screenWidth, CV_8UC3);
    GetDIBits(hMemoryDC, hBitmap, 0, screenHeight, screenMat.data, (BITMAPINFO*)&bmi, DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreen);

    return screenMat;
}

int main() {
    while (true) {
        if (GetAsyncKeyState(VK_F6)) {
            cv::Mat screen = captureScreen();
            cv::Mat templateImg = cv::imread("C:\\1.png");

            if (screen.empty() || templateImg.empty()) {
                std::cerr << "Error loading images." << std::endl;
                continue;
            }

            cv::Mat result;
            cv::matchTemplate(screen, templateImg, result, cv::TM_CCOEFF_NORMED);
            double maxVal;
            cv::Point maxLoc;
            cv::minMaxLoc(result, nullptr, &maxVal, nullptr, &maxLoc);

            if (maxVal > 0.8) {
                int centerX = maxLoc.x + templateImg.cols / 2;
                int centerY = maxLoc.y + templateImg.rows / 2;

                doubleClick(centerX, centerY);  // Double click the center of the matched image

                std::cout << "Match found and double clicked at (" << centerX << ", " << centerY << ")." << std::endl;
            }
            else {
                std::cout << "Match not found." << std::endl;
            }

            Sleep(1000);
        }
    }

    return 0;
}
