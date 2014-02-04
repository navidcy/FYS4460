#include <iostream>
#include <iomanip>
#include <time.h>
#include <fstream>
#include <lib.h>
#include <vector>
#include <cmath>
using namespace std;

double pi = 4*atan(1);

double b = 5.26;    // Ångstrøm [Å]
double kB = 1.480*pow(10,-23);    // Bolzmann constant
double eps = 0.1*1.0303;          // Energy constant [eV]
double Temp = 100.0;              // Kelvin, initial temperature
double T = (kB/eps)*Temp;            // unitless temperature
//sigma = sqrt(kB*T/m);   // standarddeviation in temp. from Boltzmann distribution

double random_number(){
    /****************************************************************************
     *  Random number generator - Bolzmann distribution
     *                Box-Muller transform
     */
    double U1 = rand()/float(RAND_MAX);
    double U2 = rand()/float(RAND_MAX);
    double val = sqrt(-2*log(U1))*cos(2*pi*U2);
    return val;
}

void initialize(vector < vector < double > > &V, vector < vector < double > > &R, int &N){

    /*   Initialize system:
     * - initial particle positions   - fcc
     * - initial particle velocities  - Boltzmann distibution
     */

    double x,y,z,Nx,Ny,Nz,Lx,Ly,Lz;
    int natom,N_atoms;

    Nx = 3;    // number of origins (from where to place the four atoms) within a box
    Ny = 3;
    Nz = 3;

    Lx = Nx*b;   // length of box along x-axis
    Ly = Ny*b;
    Lz = Nz*b;

    N_atoms = Nx*Ny*Nz;  // number of origins within a box
    N = 4*N_atoms;       // number of atoms in one box. 4 atoms per origin.

    /*****************************************************************************
     *               Initial positions and velocities
     */

    natom = 0;
    for (int ix=0; ix < Nx; ++ix) {
        for (int iy=0; iy < Ny; ++iy){
            for (int iz=0; iz < Nz; ++iz){
                vector < double > r (3); // temp position vector
                vector < double > v (3); // temp velocity vector

                x = ix*b; y=iy*b;z=iz*b;

                r[0] = x; r[1] = y; r[2] = z;
                R.push_back(r); // [x,y,z]
                v[0] = random_number();v[1] = random_number();v[2] = random_number();
                V.push_back(v);

                r[0] = x+0.5; r[1] = y+0.5; r[2] = z;
                R.push_back(r); // [x+0.5,y+0.5,z]
                v[0] = random_number();v[1] = random_number();v[2] = random_number();
                V.push_back(v);

                r[0] = x+0.5; r[1] = y; r[2] = z+0.5;
                R.push_back(r); // [x+0.5,y,z+0.5]
                v[0] = random_number();v[1] = random_number();v[2] = random_number();
                V.push_back(v);

                r[0] = x; r[1] = y+0.5; r[2] = z+0.5;
                R.push_back(r); // [x,y+0.5,z+0.5]
                v[0] = random_number();v[1] = random_number();v[2] = random_number();
                V.push_back(v);


                natom = natom+4;
            }
        }
    }

    /*********************************************************************************************
     *                     Write initial conditions to file
     * string filename = "state000.xyz", open file and write initial postions and velocities;
     */

    double sum_v = 0;
    double quad_sum_v = 0;
    double mean_v;
    double std_dev;

    for (int i=0;i<N;++i){
        cout << "Ar" << " " << R[i][0] << " " << R[i][1] << " " << R[i][2] << " " << V[i][0] << " " << V[i][1] << " " << V[i][2] << " " << endl;
        sum_v = sum_v + (V[i][0] + V[i][1] + V[i][2]);
        quad_sum_v = quad_sum_v + (V[i][0]*V[i][0] + V[i][1]*V[i][1] + V[i][2]*V[i][2]);
    }

    mean_v = sum_v/N;
    std_dev = sqrt(quad_sum_v/N - mean_v*mean_v);
    cout << "----------------------------------------------------------" << endl;
    cout  << "mean velocity = " << mean_v << " with standard deviation = " << std_dev << endl;

    ofstream myfile;
    myfile.open("state000.xyz");
    myfile << N << endl;
    myfile << "initial state fcc lattice of Argon gass" << endl;
    for (int i=0;i<N;++i){
        myfile << "Ar" << " " << R[i][0] << " " << R[i][1] << " " << R[i][2] << " " << V[i][0] << " " << V[i][1] << " " << V[i][2] << " " << endl;
    }
    myfile.close();

}


void Lennard_Jones(vector < vector < double > > &F, vector < vector < double > > &R, int &N){
    /* The Lenny-Jones potential updates the forces F on particle i in position R.
     */

    double rx,ry,rz,r2,r2i,r6i,r12i;

    int force = 0;
    for (int i = 0; i < N; ++i) {
        rx = R[i][0]; ry = R[i][1]; rz = R[i][2];
        vector < double > f (3);   //  sums up to total force on i
        for (int j = 0; j < N; ++j) {
            if ( j != i) {
                vector < double > r_ij (3);
                vector < double > fij (3);

                r_ij[0] = rx - R[j][0];  // distance between particle i and j.
                r_ij[1] = ry - R[j][1];
                r_ij[2] = rz - R[j][2];

                r2 = r_ij[0]*r_ij[0] + r_ij[1]*r_ij[1] + r_ij[2]*r_ij[2];
                r2i = 1.0/r2;
                r6i = r2i*r2i*r2i;
                r12i = r6i*r6i;

                fij[0] = 24*(2*r12i - r6i)*r2i*r_ij[0];     // force from j on i.
                fij[1] = 24*(2*r12i - r6i)*r2i*r_ij[1];
                fij[2] = 24*(2*r12i - r6i)*r2i*r_ij[2];

                f[0] = f[0] + fij[0];
                f[1] = f[1] + fij[1];
                f[2] = f[2] + fij[2];


                force = force + 1;
            } // end if
        } // end for j

        F[i] = f;  // total force on particle i
    } // end for i


    cout << "------Forces on particle i------------" << endl;
    for (int i = 0; i < N; ++i) {
        cout << i << " " << F[i][0] << " "  << F[i][1] << " " << F[i][2] << endl;
    }

}


void integrator(vector < vector < double > > &R,vector < vector < double > > &V,int N){

    /* *********************************************************************************************
     * Integrator uses the stable Verlet algorithm to calculate the motion of the particles
     *  using the Lenny-Jones potential to find the force between evry particle.
     *  Creates a new state file: stateXXX.xyz for every loop iteration while t < tmax.
     */

    vector < vector < double > > F (N, vector < double > (3,0)); // Vector that holds the forces on particle i.

    Lennard_Jones(F,R,N);

    double dt = 0.11;
    double m = 39.948; // amu
    char* filename;
   // while (t<tmax) {

    filename = new char[20];
    filename = "state001.xyz";
    ofstream myfile;
    myfile.open(filename);
    myfile << N ;
    int time = 1;

    Lennard_Jones(F,R,N); // calculate the force at time (t)
    myfile << "state%03g" << time << endl;

    for (int i = 0; i < N; ++i) {      // update velocity and positions from the forces acting on the particles
        V[i] = V[i] + F[i]*dt/(2*m);   // Calculate V[i] at (t + dt/2)
        R[i] = R[i] + V[i]*dt;         // Calculate R[i] at (t + dt)
    }
    Lennard_Jones(F,R,N);              // calculate the force at time (t+dt) using the new positions.
    for (int i = 0; i < N; ++i) {
        V[i] = V[i] + F[i]*dt/(2*m);   // then find the velocities at time (t+dt)

        // Write to file:
        myfile << "Ar" << " " << R[i][0] << " " << R[i][1] << " " << R[i][2] << " " << V[i][0] << " " << V[i][1] << " " << V[i][2] << " " << endl;
    }
    myfile.close();
}




int main(){

    // create a vector within a vector, using including the <vector> library.
    vector < vector < double > > R; // positions
    vector < vector < double > > V; // velocities
    int N;

    initialize(V,R,N);

    integrator(V,R,N);

    return 0;
}

