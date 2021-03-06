#include<omp.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <chrono>
#include <windows.h>
#include <math.h>
using namespace std;
using namespace chrono;

const float PI = 3.141592653589793f;
bool no_write = true;
float f(float x)
{
	return static_cast<float>(1.0 / (1 + x * x));
}

float Simpson(int i, float h)
{
	return static_cast<float>(2 * f((i - 0.5)*h) + f(i*h));
}

float ConstSimpson(int n, float h)
{
	return static_cast<float>((f(0) + f(1)) / 2 + 2 * f((n - 0.5)*h));
}

float CoeficentSimpson(float h)
{
	return static_cast<float>(h / 3);
}


float Trapece(int i, float h)
{
	return static_cast<float>(f(i*h));
}

float ConstTrapece()
{
	return static_cast<float>((f(0) + f(1)) / 2);
}

float CoeficentTrapece(float h)
{
	return h;
}


float Quadro(int i, float h)
{
	return static_cast<float>(f((i - 0.5)*h));
}

float ConstQuadro(int n, float h)
{
	return Quadro(n, h);
}

float CoeficentQuadro(float h)
{
	return h;
}

float eps(float calcPi)
{
	float delta = (PI - calcPi) / PI;
	if (delta >= 0)
		return delta;

	return static_cast<float>(-delta);
}

float CalculatePiQuadro(int n)
{
	float h = 1.0 / n;
	int i = 0;
	float calcPi = 0;
#pragma omp parallel
	{
#pragma omp for reduction(+:calcPi)
		for (i = 1; i <= n - 1; i++)
		{
			calcPi += Quadro(i, h);
		}
		if (omp_get_thread_num() == 0)
		{
			printf("Number of processes = %d\n", omp_get_num_threads());
			printf("Number of processors = %d\n", omp_get_num_procs());
		}
	}
	return CoeficentQuadro(h)*(calcPi + ConstQuadro(n, h)) * 4;
}

float CalculatePiSimpson(int n)
{
	float h = 1.0 / n;
	int i = 0;
	float calcPi = 0;
#pragma omp parallel
	{
#pragma omp for reduction(+:calcPi)
		for (i = 1; i <= n - 1; i++)
		{
			calcPi += Simpson(i, h);
		}
	}
	return CoeficentSimpson(h)*(calcPi + ConstSimpson(n, h)) * 4;
}

float CalculatePiTrapece(int n)
{
	float h = 1.0 / n;
	float calcPi = 0;
	int i = 0;
#pragma omp parallel
	{
#pragma omp for reduction(+:calcPi)
		for (i = 1; i <= n - 1; i++)
		{
			calcPi += Trapece(i, h);
		}
	}
	return CoeficentTrapece(h)*(calcPi + ConstTrapece()) * 4;
}



struct Result
{
	float Pi;
	float Eps;
	double Time;
};

void PrintTable(int * n, Result(&calcPi)[3][6])
{
	if (!no_write)
	{
		for (int t = 0; t < 3; t++)
		{
			printf("Number threads = %d\n", (int)pow(2, t + 1));
			printf("\n");
			printf("\n");
			for (int k = 0; k < 5; k++)
			{
				printf("n = %d| Pi = %lf| Eps = %lf| Time = %lf micsec\n", n[k], calcPi[t][k].Pi, calcPi[t][k].Eps, calcPi[t][k].Time);
				printf("---------------------------------\n");
			}
		}
		printf("\n");
		printf("\n");
		printf("\n");
	}
	printf("********************************************************************************\n");
	printf("SUMMARY\n");

	for (int k = 0; k < 6; k++)
	{
		double U_1 = 0;
		double U_2 = 0;
		if (calcPi[1][k].Time > 0)
			U_1 = calcPi[0][k].Time / calcPi[1][k].Time;
		if (calcPi[2][k].Time > 0)
			U_2 = calcPi[0][k].Time / calcPi[2][k].Time;

		printf("n = %d |Pi = %lf| Eps = %lf| Time(2) = %lf micsec| Time(4) = %lf micsec| Time(8) = %lf micsec| U_1 = %lf| U_2 = %lf\n", n[k], calcPi[0][k].Pi, calcPi[0][k].Eps,
			calcPi[0][k].Time, calcPi[1][k].Time, calcPi[2][k].Time, U_1, U_2);
		printf("---------------------------------------------------------------------------------\n");
	}
	printf("********************************************************************************\n\n\n");
}


void PrintTable(int n, Result(&calcPi)[3][1])
{
	printf("********************************************************************************\n");
	printf("SUMMARY\n");
	int k = 0;
	double U_1 = 0;
	double U_2 = 0;
	if (calcPi[1][k].Time > 0)
		U_1 = calcPi[0][k].Time / calcPi[1][k].Time;
	if (calcPi[2][k].Time > 0)
		U_2 = calcPi[0][k].Time / calcPi[2][k].Time;
	printf("n = %d |Pi = %lf| Eps = %lf| Time(2) = %lf micsec| Time(4) = %lf micsec| Time(8) = %lf micsec| U_1 = %lf| U_2 = %lf\n", n, calcPi[0][k].Pi, calcPi[0][k].Eps,
		calcPi[0][k].Time, calcPi[1][k].Time, calcPi[2][k].Time, U_1, U_2);
	printf("---------------------------------------------------------------------------------\n");

	printf("********************************************************************************\n\n\n");
}

int main(int argc, char *argv[])

{
	int valarg = 1;
	if (argc == 2) {
		valarg = atoi(argv[1]);
		valarg = static_cast<int>(valarg);
	}
	system_clock::time_point start, finish;
	long loop;
	double result;
	setlocale(LC_ALL, "");
	Result calculatedPiQuadro[3][6];
	Result calculatedPiTrapece[3][6];
	Result calculatedPiSimpson[3][6];
	Result calcPiQuadroArgv[3][1];
	Result calcPiTrapeceArgv[3][1];
	Result calcPiSimpsonArgv[3][1];
	int n[6];
	n[0] = 10;
	n[1] = 50;
	n[2] = 100;
	n[3] = 500;
	n[4] = 1000;
	n[5] = 50000000;
	//double  start,  finish;
	float pi = 0;

	for (int t = 0; t < 3; t++)
	{
		omp_set_num_threads((int)pow(2, t + 1));
		for (int k = 0; k < 6; k++)
		{
			start = system_clock::now();
			pi = CalculatePiQuadro(n[k]);
			finish = system_clock::now();
			calculatedPiQuadro[t][k].Pi = pi;
			calculatedPiQuadro[t][k].Eps = eps(pi);
			calculatedPiQuadro[t][k].Time = (finish - start).count() / 1000;//finish - start; //?????

			start = system_clock::now();
			pi = CalculatePiTrapece(n[k]);
			finish = system_clock::now();
			calculatedPiTrapece[t][k].Pi = pi;
			calculatedPiTrapece[t][k].Eps = eps(pi);
			calculatedPiTrapece[t][k].Time = (finish - start).count() / 1000;//finish - start; //?????

			start = system_clock::now();
			pi = CalculatePiSimpson(n[k]);
			finish = system_clock::now();
			calculatedPiSimpson[t][k].Pi = pi;
			calculatedPiSimpson[t][k].Eps = eps(pi);
			calculatedPiSimpson[t][k].Time = (finish - start).count() / 1000;//finish - start; //?????
		}

		start = system_clock::now();
		pi = CalculatePiQuadro(valarg);
		finish = system_clock::now();
		calcPiQuadroArgv[t][0].Pi = pi;
		calcPiQuadroArgv[t][0].Eps = eps(pi);
		calcPiQuadroArgv[t][0].Time = (finish - start).count() / 1000;

		start = system_clock::now();
		pi = CalculatePiTrapece(valarg);
		finish = system_clock::now();
		calcPiTrapeceArgv[t][0].Pi = pi;
		calcPiTrapeceArgv[t][0].Eps = eps(pi);
		calcPiTrapeceArgv[t][0].Time = (finish - start).count() / 1000;

		start = system_clock::now();
		pi = CalculatePiSimpson(valarg);
		finish = system_clock::now();
		calcPiSimpsonArgv[t][0].Pi = pi;
		calcPiSimpsonArgv[t][0].Eps = eps(pi);
		calcPiSimpsonArgv[t][0].Time = (finish - start).count() / 1000;

	}
	printf("\nRoot mean square formula:\n");
	PrintTable(valarg, calcPiQuadroArgv);
	printf("\nTrapezium formula:\n");
	PrintTable(valarg, calcPiTrapeceArgv);
	printf("\nSimpson's formula:\n");
	PrintTable(valarg, calcPiSimpsonArgv);

	printf("\nRoot mean square formula:\n");
	PrintTable(n, calculatedPiQuadro);
	printf("\nTrapezium formula:\n");
	PrintTable(n, calculatedPiTrapece);
	printf("\nSimpson's formula:\n");
	PrintTable(n, calculatedPiSimpson);

	system("pause");
	return 0;
}