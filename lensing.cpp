/*
   159735 Parallel Programming Assignment 4

   To compile: make lensing
   To run: ./lensing [nlenses] [lens_scale]
   nlenses is 1 by default, lens_scale is 0.005 by default
   For example:
   "./lensing 2 0.001" will calculate a lensing image with lens example 2, and lens_scale 0.001

 */

#include <iostream>
#include "lenses.h"
#include "arrayff.hxx"
#include <cuda.h>
#include <stdio.h>

using namespace std;

// Output latest cuda error
#define cudaCheckError() {                                                                 \
 cudaError_t e=cudaGetLastError();                                                         \
 if(e!=cudaSuccess) {                                                                      \
   printf("Cuda failure %s:%d: '%s'\n",__FILE__,__LINE__,cudaGetErrorString(e));           \
 }                                                                                         \
}

// Boundaries in physical units on the lens plane
const float WL  = 2.0;
const float XL1 = -WL;
const float XL2 =  WL;
const float YL1 = -WL;
const float YL2 =  WL;

// Define the limitation of len_scale
const float MAX_SCALE =  1.0;
const float MIN_SCALE =  0.00001;


/* Query device about threads limitation */
bool getCudaDeviceInfo(int &maxThreadsPerBlock, int maxDimensionPerGrid[])
{
   cudaDeviceProp deviceProp;
   cudaGetDeviceProperties(&deviceProp, 0); // In our environment, there is only one GPU.
   maxThreadsPerBlock = deviceProp.maxThreadsPerBlock;
   // We'd like to use two-dimensions because it is enough to resolve this assignment
   maxDimensionPerGrid[0] = deviceProp.maxGridSize[0];
   maxDimensionPerGrid[1] = deviceProp.maxGridSize[1];
   return true;
}

// Set up a single lens example
int set_example_1(float** xlens, float** ylens, float** eps)
{
  const int nlenses = 1;
  *xlens = new float[nlenses];
  *ylens = new float[nlenses];
  *eps   = new float[nlenses];
  *xlens[0] = 0.0;
  *ylens[0] = 0.0;
  *eps[0] = 1.0;
  return nlenses;
}

// Simple binary lens
int set_example_2(float** xlens, float** ylens, float** eps)
{
  const int nlenses = 2;
  float* x = new float[nlenses];
  float* y = new float[nlenses];
  float* e = new float[nlenses];

  const float eps1 = 0.2;
  x[0] = -0.4; y[0] = 0.0; e[0] = 1 - eps1;
  x[1] =  0.6; y[1] = 0.2; e[1] = eps1;

  *xlens = x;
  *ylens = y;
  *eps = e;
  return nlenses;
}

// Set up triple lens
int set_example_3(float** xlens, float** ylens, float** eps)
{
  const int nlenses = 3;
  float* x = new float[nlenses];
  float* y = new float[nlenses];
  float* e = new float[nlenses];

  const float eps1 = 0.3;
  const float eps2 = 0.2;

  x[0] = -0.4; y[0] = 0.0; e[0] = 1 - eps1 - eps2;
  x[1] =  0.5; y[1] = 0.0; e[1] = eps1;
  x[2] =  0.0; y[2] = 0.4; e[2] = eps2;

  *xlens = x;
  *ylens = y;
  *eps = e;
  return nlenses;
}

// Pick a random for set_example_n
float pick_random(float x1, float x2)
{
  float f = rand() / static_cast<float>(RAND_MAX);
  return x1 + f * (x2 - x1);
}

// Set up more than 3 lenses
int set_example_n(const int nuse, float** xlens, float** ylens, float** eps)
{
  const int nlenses = nuse;
  float* x = new float[nlenses];
  float* y = new float[nlenses];
  float* e = new float[nlenses];

  float sume = 0;
  const float w = 1.2;
  for (int n =0; n < nlenses; ++n) {
    x[n] = pick_random(-w, w);
    y[n] = pick_random(-w, w);
    e[n] = pick_random(0, 1.0);
    sume += e[n];
  }
  // Normalize the mass fractions
  for (int n =0; n < nlenses; ++n) e[n] /= sume;

  *xlens = x;
  *ylens = y;
  *eps = e;

  return nlenses;
}

// Main fuction running in GPU, calculate lensing image
__global__ void compute(int total, float YL1, float XL1, int length, float lens_scale,
                        float xsrc, float ysrc, float* eps, float* xlens, float* ylens, float ldc,
                        int nlenses, float rsrcSqure, float *lensim)
{
  int row = blockIdx.x * blockDim.x + threadIdx.x;
  int col = blockIdx.y * blockDim.y + threadIdx.y;
  int index = col + row * gridDim.y*blockDim.y;
  if(index >= total)
    return;
  int iy = index / length;
  int ix = index % length;
  float yl = YL1 + lens_scale * iy;
  float xl = XL1 + lens_scale * ix;
  float xs, ys, sepSquare, mu;
  float xd, yd;
  float dx, dy, dr;
  xs = xl;
  ys = yl;
  // shoot
  for (int p = 0; p < nlenses; ++p) {
    dx = xl - xlens[p];
    dy = yl - ylens[p];
    dr = dx * dx + dy * dy;
    xs -= eps[p] * dx / dr;
    ys -= eps[p] * dy / dr;
  }
  xd = xs - xsrc;
  yd = ys - ysrc;
  sepSquare = xd *xd + yd * yd;
  if(sepSquare < rsrcSqure)
  {
    mu = sqrt(1 - sepSquare / rsrcSqure);
    lensim[index] = 1.0 - ldc *(1 - mu);
  } else {
    lensim[index] = 0;
  }
}

// Main entry
int main(int argc, char* argv[])
{
   // Set up lensing system configuration - call example_1, _2, _3 or
   // _n according to argv, by default example_1 is used.

   int inputLense = 1;
   float inputScale = 0.005; // By default, use scale 0.005
   if(argc==2)
      inputLense=atoi(argv[1]);
   if(argc==3)
      inputScale=atof(argv[2]);
   // Check and make sure user input is legal
   if(inputLense <= 0)
      inputLense = 1;
   inputScale = min(MAX_SCALE, inputScale);
   inputScale = max(MIN_SCALE, inputScale);

   float* xlens;
   float* ylens;
   float* eps;
   int nlenses;
   // Set up lenses example according to user input
   if(inputLense == 1)
      nlenses = set_example_1(&xlens, &ylens, &eps);
   else if(inputLense == 2)
      nlenses = set_example_2(&xlens, &ylens, &eps);
   else if(inputLense == 3)
      nlenses = set_example_3(&xlens, &ylens, &eps);
   else if(inputLense > 3)
      nlenses = set_example_n(inputLense, &xlens, &ylens, &eps);
   std::cout << "# Simulating " << nlenses << " lens system" << std::endl;
   // Alloc memory for cuda
   size_t size = nlenses * sizeof(float);
   float *d_xlens, *d_ylens, *d_eps;
   cudaMalloc(&d_xlens, size);
   cudaMalloc(&d_ylens, size);
   cudaMalloc(&d_eps, size);
   // Copy memory from host to gpu
   cudaMemcpy(d_xlens, xlens, size, cudaMemcpyHostToDevice);
   cudaMemcpy(d_ylens, ylens, size, cudaMemcpyHostToDevice);
   cudaMemcpy(d_eps, eps, size, cudaMemcpyHostToDevice);
   // For timing
   cudaEvent_t start, stop;
   cudaEventCreate(&start);
   cudaEventCreate(&stop);

   // Source star parameters. You can adjust these if you like - it is
   // interesting to look at the different lens images that result
   const float rsrc = 0.1;      // radius
   const float ldc  = 0.5;      // limb darkening coefficient
   float xsrc = 0.0;      // x and y centre on the map
   float ysrc = 0.0;

   const float lens_scale = inputScale;

   // Size of the lens image
   const int npixx = static_cast<int>(floor((XL2 - XL1) / lens_scale)) + 1;
   const int npixy = static_cast<int>(floor((YL2 - YL1) / lens_scale)) + 1;
   std::cout << "# Building " << npixx << "*" << npixy << " lens image" << std::endl;

   // Put the lens image in  array
   Array<float, 2> lensim(npixy, npixx);
   int totalPixels = npixx * npixy;
   size = totalPixels * sizeof(float);
   float *h_lensim, *d_lensim;
   cudaMalloc(&d_lensim, size);
   h_lensim = new float[totalPixels];

   // Draw the lensing image map here via CUDA
   const float rsrcSqure = rsrc * rsrc;
   // Launch kernel and time it

   int threadsPerBlock;
   int maxDimensionPerGrid[2];
   getCudaDeviceInfo(threadsPerBlock, maxDimensionPerGrid);
   int blocksPerGrid = (totalPixels + threadsPerBlock - 1) / threadsPerBlock;
   int dimensionPerGridX = blocksPerGrid;
   int dimensionPerGridY = 1;
   // If one dimensions is not enough, use two dimensions. In this assignment, two dimensions is enough.
   if(blocksPerGrid > maxDimensionPerGrid[0])
   {
      dimensionPerGridX = (blocksPerGrid +  maxDimensionPerGrid[1] - 1) /  maxDimensionPerGrid[1];
      dimensionPerGridY = maxDimensionPerGrid[1];
   }
   dim3 numBlocks = dim3(dimensionPerGridX, dimensionPerGridY);
   std::cout << "Launching a grid of " << dimensionPerGridX << "*" << dimensionPerGridY << " "
             <<threadsPerBlock  << " threads" << std::endl;
   cudaEventRecord(start, 0);
   compute<<<numBlocks, threadsPerBlock>>>(totalPixels, YL1, XL1, npixy, lens_scale,
                                           xsrc, ysrc, d_eps, d_xlens, d_ylens, ldc,
                                           nlenses, rsrcSqure, d_lensim);
   cudaEventRecord(stop, 0);
   cudaEventSynchronize(stop);
   float time;  // Must be a float
   cudaEventElapsedTime(&time, start, stop);
   cudaEventDestroy(start);
   cudaEventDestroy(stop);
   std::cout << "Kernel took: " << time << " ms" << std::endl;

   cudaMemcpy(h_lensim, d_lensim, size, cudaMemcpyDeviceToHost);
   // Output the result
   for(int i = 0; i < totalPixels; i ++)
   {
      int iy = i / npixx;
      int ix = i % npixx;
      lensim(iy, ix) = h_lensim[i];
   }

   dump_array<float, 2>(lensim, "lens.fit");

   // Release memory
   delete[] xlens;
   delete[] ylens;
   delete[] eps;
   delete[] h_lensim;

   cudaFree(d_xlens);
   cudaFree(d_ylens);
   cudaFree(d_eps);
   cudaFree(d_lensim);
}

