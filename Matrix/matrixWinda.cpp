#include <iostream>
#include <vector>
#include <windows.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

HANDLE mtx;

// N matrix
// k blok size

//КОММЕНТ ДЛЯ ПРОВЕРЯЮЩЕГО
//я не стала вывести зависимость времени для всех размеров блоков от 1 до N
//тк тестила на очень больших числах (до 1000) и выводить 1000 строчек ну такое себе
//поэтому сам выбирай размер блока и смотри

void matrixPrint(int N, const vector<vector<int>>& A){
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout << A[i][j] << " ";
        }
    cout << "\n";
    }
}

struct ThreadData {
    int rowStart;
    int colStart;
    int k;
    int N;
    const vector<vector<int>>* A;
    const vector<vector<int>>* B;
    vector<vector<int>>* C;
};

DWORD WINAPI multiBlok(void* smth) {
    ThreadData* data = (ThreadData*)smth;
    
    int rowStart = data->rowStart;
    int colStart = data->colStart;
    int k = data->k;
    int N = data->N;
    const vector<vector<int>>& A = *(data->A);
    const vector<vector<int>>& B = *(data->B);
    vector<vector<int>>& C = *(data->C);
    
    for (int i = rowStart; i < rowStart + k && i < N; i++) {
        for (int j = colStart; j < colStart + k && j < N; j++) {
            int sum = 0;
            for (int x = 0; x < N; x++) {
                sum += A[i][x] * B[x][j];
            }
            WaitForSingleObject(mtx, INFINITE);
            C[i][j] = sum;
            ReleaseMutex(mtx);
        }
    }
    
    delete data;
    return 0;
}

void simpleMulti(const vector<vector<int>>& A, const vector<vector<int>>& B, vector<vector<int>>& C, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            C[i][j] = 0;
            for (int k = 0; k < N; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

int main() {
    mtx = CreateMutex(NULL, FALSE, NULL);

    int N, k;
    
    cout << "matrix size N (>5): ";
    cin >> N;
    cout << "blok size k (N>k>1): ";
    cin >> k;
    
    vector<vector<int>> A(N, vector<int>(N));
    vector<vector<int>> B(N, vector<int>(N));
    vector<vector<int>> C(N, vector<int>(N, 0));
    
    //мне лень самой вечно вводить сори
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % 10;
            B[i][j] = rand() % 10;
        }
    }

    if (N <= 20) {
        cout << "matrix A:\n";
        matrixPrint(N,A);
        cout << "matrix B:\n";
        matrixPrint(N, B);
    }

    auto startBloks = high_resolution_clock::now();
    
    int bloksNum = (N + k - 1) / k; //в большую округляем если инвалидный блок
    vector<HANDLE> threads;
    
    for (int i = 0; i < bloksNum; i++) {
        for (int j = 0; j < bloksNum; j++) {
            int rowStart = i * k;
            int colStart = j * k;
            
            ThreadData* data = new ThreadData;
            data->rowStart = rowStart;
            data->colStart = colStart;
            data->k = k;
            data->N = N;
            data->A = &A;
            data->B = &B;
            data->C = &C;
            
            HANDLE threadID = CreateThread(NULL, 0, multiBlok, data, 0, NULL);
            threads.push_back(threadID);
        }
    }
    
    // join
    WaitForMultipleObjects(threads.size(), threads.data(), TRUE, INFINITE);
    
    for (int i = 0; i < threads.size(); i++) {
        CloseHandle(threads[i]);
    }
    
    auto endBloks = high_resolution_clock::now();

    //когда ставила милисекунды то на матрицах маленьких не было видно время
    auto durationBloks = duration_cast<microseconds>(endBloks - startBloks);

    vector<vector<int>> simpleC(N, vector<int>(N, 0));
    auto startMatrix = high_resolution_clock::now();
    simpleMulti(A, B, simpleC, N);
    auto endMatrix = high_resolution_clock::now();
    auto durationMatrix = duration_cast<microseconds>(endMatrix - startMatrix);
    
    cout << "bloks number: " << bloksNum << "x" << bloksNum << "\n";
    cout << "treads number: " << threads.size() << "\n";
    cout << "bloksMalti time: " << durationBloks.count() << " mcs\n";
    cout << "matrixMalti time: " << durationMatrix.count() << " mcs\n";

    cout << "time differenties (matrix/bloks): " << (double)durationMatrix.count() / durationBloks.count() << "\n";

    if (N <= 20) {
        cout << "result matrix C (bloksMulti):\n";
        matrixPrint(N, C);
        cout << "result matrix C (matrixMulti):\n";
        matrixPrint(N, simpleC);
    }
    
    CloseHandle(mtx);
}
