#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include "buffered_channel.h"

using namespace std;
using namespace std::chrono;

mutex mtx;

// N matrix
// k blok size

void matrixPrint(int N, const vector<vector<int>>& A){
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cout << A[i][j] << " ";
        }
    cout << "\n";
    }
}

struct BlockTask {
    int Ai, Bj, At;
    int k;
    int rowStartA, colStartA;
    int rowStartB, colStartB;
};

void multiBlok(BlockTask task, int N, const vector<vector<int>>& A, const vector<vector<int>>& B, vector<vector<int>>& C) {

    int rowStartC = task.Ai * task.k;
    int colStartC = task.Bj * task.k;

    for (int i = 0; i < task.k && task.rowStartA + i < N; i++) {
        for (int j = 0; j < task.k && task.colStartB + j < N; j++) {
            int sum = 0;
            for (int x = 0; x < task.k && task.colStartA + x < N && task.rowStartB + x < N; x++) {
                sum += A[task.rowStartA + i][task.colStartA + x] * B[task.rowStartB + x][task.colStartB + j];
            }
            unique_lock<std::mutex> lock(mtx);
            C[rowStartC + i][colStartC + j] += sum;
        }
    }
}

void channelWorker(int N, const vector<vector<int>>& A, const vector<vector<int>>& B, vector<vector<int>>& C, BufferedChannel<BlockTask>& channel) {
    while (true) {
        auto [task, valid] = channel.recv();
        if (!valid) break;
        
        multiBlok(task, N, ref(A), ref(B), ref(C));
    }
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
    
    // создание канала
    BufferedChannel<BlockTask> taskChannel(bloksNum * bloksNum * 2); // буфер побольше
    vector<thread> threads;
    
    // запуск потоков для канала 40 для примера
    int numWorkers = 40;
    for (int i = 0; i < numWorkers; i++) {
        threads.emplace_back(channelWorker, N, ref(A), ref(B), ref(C), ref(taskChannel));
    }
    
    for (int i = 0; i < bloksNum; i++) {
        for (int j = 0; j < bloksNum; j++) {
            for (int t = 0; t < bloksNum; t++) {
                BlockTask task;
                task.Ai = i;
                task.Bj = j;
                task.At = t;
                task.k = k;
                task.rowStartA = i * k;
                task.colStartA = t * k;
                task.rowStartB = t * k;
                task.colStartB = j * k;
                
                taskChannel.send(task);
            }
        }
    }
    
    // закрываем канал
    taskChannel.close();
    
    for (auto& th : threads) {
        th.join();
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
}