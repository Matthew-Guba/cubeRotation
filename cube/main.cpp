#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;

const int WIDTH = 80;
const int HEIGHT = 40;
const float FOV = 3.14159 / 4; // Field of View
const float Z_DIST = 5.0f;

struct Point3D { float x, y, z; };
struct Point2D { int x, y; };

class ScreenBuffer {
private:
    char buffer[HEIGHT][WIDTH];
public:
    ScreenBuffer() { clear(); }

    void clear() {
        for (int y = 0; y < HEIGHT; y++)
            for (int x = 0; x < WIDTH; x++)
                buffer[y][x] = ' ';
    }

    void setPixel(int x, int y, char c) {
        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
            buffer[y][x] = c;
    }

    void draw() const {
#ifdef _WIN32
        system("cls");
#else
        cout << "\033[2J\033[1;1H"; // ANSI clear screen
#endif

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++)
                cout << buffer[y][x];
            cout << endl;
        }
    }
};

Point2D project(const Point3D& point, float angleX, float angleY, float angleZ) {
    // 3D rotation matrices
    float cosX = cos(angleX), sinX = sin(angleX);
    float cosY = cos(angleY), sinY = sin(angleY);
    float cosZ = cos(angleZ), sinZ = sin(angleZ);

    // Rotate point
    float x = point.x;
    float y = point.y;
    float z = point.z;

    // X-axis rotation
    float temp = y;
    y = y * cosX - z * sinX;
    z = temp * sinX + z * cosX;

    // Y-axis rotation
    temp = x;
    x = x * cosY + z * sinY;
    z = -temp * sinY + z * cosY;

    // Z-axis rotation
    temp = x;
    x = x * cosZ - y * sinZ;
    y = temp * sinZ + y * cosZ;

    // Projection
    float scale = 1 / (Z_DIST - z);
    return {
        static_cast<int>(WIDTH / 2 + x * scale * WIDTH * FOV),
        static_cast<int>(HEIGHT / 2 - y * scale * WIDTH * FOV)
    };
}

void drawLine(ScreenBuffer& buffer, Point2D p1, Point2D p2, char c) {
    int dx = abs(p2.x - p1.x), sx = p1.x < p2.x ? 1 : -1;
    int dy = -abs(p2.y - p1.y), sy = p1.y < p2.y ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        buffer.setPixel(p1.x, p1.y, c);
        if (p1.x == p2.x && p1.y == p2.y) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; p1.x += sx; }
        if (e2 <= dx) { err += dx; p1.y += sy; }
    }
}

int main() {
    ScreenBuffer screen;
    vector<Point3D> cubeVertices = {
        {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
        {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
    };

    vector<pair<int, int>> edges = {
        {0,1}, {1,2}, {2,3}, {3,0}, // Front face
        {4,5}, {5,6}, {6,7}, {7,4}, // Back face
        {0,4}, {1,5}, {2,6}, {3,7}  // Connecting edges
    };

    float angleX = 0, angleY = 0, angleZ = 0;

    while (true) {
        screen.clear();
        vector<Point2D> projected(8);

        // Project all vertices
        for (int i = 0; i < 8; i++)
            projected[i] = project(cubeVertices[i], angleX, angleY, angleZ);

        // Draw all edges
        for (auto& edge : edges) {
            Point2D p1 = projected[edge.first];
            Point2D p2 = projected[edge.second];
            drawLine(screen, p1, p2, '#');
        }

        // Add color and rotation
        screen.draw();
        angleX += 0.05;
        angleY += 0.03;
        angleZ += 0.02;

#ifdef _WIN32
        Sleep(5);
#else
        usleep(5);
#endif
    }

    return 0;
}
