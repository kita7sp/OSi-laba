#include <iostream>
#include <vector>
#include <pthread.h>
#include <chrono>

using namespace std;
using namespace std::chrono;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// N matrix
// k blok size

//КОММЕНТ ДЛЯ ПРОВЕРЯЩЕГО
//я не стала выводить зависимость времени для всех размеров блоков от 1 до N
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
    int k;
    int N;
    const vector<vector<int>>* A;
    const vector<vector<int>>* B;
    vector<vector<int>>* C;
    int Ai;
    int Bj;
    int At;
};

void* multiBlok(void* smth) {
    ThreadData* data = (ThreadData*)smth;
    
    int Ai = data->Ai;
    int Bj = data->Bj;
    int At = data->At;
    int k = data->k;
    int N = data->N;
    const vector<vector<int>>& A = *(data->A);
    const vector<vector<int>>& B = *(data->B);
    vector<vector<int>>& C = *(data->C);
    
    int rowStartA = Ai * k;
    int colStartA = At * k;
    int rowStartB = At * k;
    int colStartB = Bj * k;
    int rowStartC = Ai * k;
    int colStartC = Bj * k;

    for (int i = 0; i < k && rowStartA + i < N; i++) {
        for (int j = 0; j < k && colStartB + j < N; j++) {
            int sum = 0;
            for (int x = 0; x < k && colStartA + x < N && rowStartB + x < N; x++) {
                sum += A[rowStartA + i][colStartA + x] * B[rowStartB + x][colStartB + j];
            }
            pthread_mutex_lock(&mtx);
            C[rowStartC + i][colStartC + j] += sum;
            pthread_mutex_unlock(&mtx);
        }
    }
    
    delete data;
    return nullptr;
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
    vector<pthread_t> threads;
    
    //цикл запуска комбинаций блоков по itj
    for (int i = 0; i < bloksNum; i++) {
        for (int j = 0; j < bloksNum; j++) {
            for (int t = 0; t < bloksNum; t++) {
                ThreadData* data = new ThreadData;

                data->k = k;
                data->N = N;
                data->A = &A;
                data->B = &B;
                data->C = &C;
                data->Ai = i;
                data->Bj = j;
                data->At = t;
                
                pthread_t threadID;
                pthread_create(&threadID, nullptr, multiBlok, data);
                threads.push_back(threadID);
            }
        }
    }
    
    for (int i = 0; i < (int)threads.size(); i++) {
        pthread_join(threads[i], nullptr);
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
    
    pthread_mutex_destroy(&mtx);
}

// clang++ -std=c++17 PmatrixIOS.cpp -o PmatrixIOS -lpthread && ./PmatrixIOS